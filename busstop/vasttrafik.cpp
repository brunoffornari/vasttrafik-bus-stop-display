#include "vasttrafik.h"

#include <ArduinoJson.h>
#include <HTTPClient.h>

#include "secrets.h"
#include "time_service.h"
#include "utils.h"
#include "wifi_manager.h"

namespace {

String accessToken;  // empty = needs refresh
String stopId;

// "#RRGGBB" -> number, or kNoColor if missing/invalid.
uint32_t parseHexColor(const char *text) {
  if (!text || text[0] != '#' || strlen(text) != 7) return config::kNoColor;
  return (uint32_t)strtoul(text + 1, nullptr, 16);
}

// Turns the API JSON into a DepartureBoard.
FetchResult parseDepartures(const String &payload, DepartureBoard &board) {
  // Keep only the fields we use, so the response fits in memory.
  StaticJsonDocument<512> filter;
  JsonObject item = filter["results"].createNestedObject();
  item["stopPoint"]["name"] = true;
  item["estimatedOtherwisePlannedTime"] = true;
  item["isCancelled"] = true;
  item["serviceJourney"]["direction"] = true;
  item["serviceJourney"]["line"]["name"] = true;
  item["serviceJourney"]["line"]["backgroundColor"] = true;
  item["serviceJourney"]["line"]["foregroundColor"] = true;

  DynamicJsonDocument doc(4096);
  DeserializationError error = deserializeJson(doc, payload, DeserializationOption::Filter(filter));
  if (error) {
    Serial.printf("JSON parse error: %s\n", error.c_str());
    return FetchResult::ParseError;
  }

  JsonArray results = doc["results"].as<JsonArray>();
  if (results.isNull()) {
    Serial.println("Results array is null");
    return FetchResult::ParseError;
  }
  Serial.printf("Found %d departures\n", results.size());

  board.count = 0;
  board.stopName = "";

  // Stop name, e.g. "Gräddgatan, Göteborg" -> "Gräddgatan"
  if (results.size() > 0 && !results[0]["stopPoint"]["name"].isNull()) {
    String name = cutAt(results[0]["stopPoint"]["name"].as<String>(), ',');
    board.stopName = name;
  } else {
    Serial.println("Stop name not found in response");
  }

  if (results.size() == 0) return FetchResult::NoDepartures;

  for (JsonObject dep : results) {
    if (board.count >= config::kMaxDepartures) break;

    String line = dep["serviceJourney"]["line"]["name"] | "";
    if (line.startsWith("Buss ") || line.startsWith("Tram ")) line = line.substring(5);

    String direction = dep["serviceJourney"]["direction"] | "";
    direction = cutAt(direction, ',');  // remove city suffix

    String departureTime = dep["estimatedOtherwisePlannedTime"] | "";

    if (line.isEmpty() || direction.isEmpty() || departureTime.isEmpty()) {
      Serial.println("Missing departure data, skipping...");
      continue;
    }

    int64_t departUtc;
    if (!isoTimeToUtc(departureTime, departUtc)) {
      Serial.println("Failed to parse time: " + departureTime);
      continue;
    }
    if (minutesUntilUtc(departUtc) < 0) continue;  // already left

    Departure &d = board.items[board.count++];
    d.line = line;
    d.direction = direction;
    d.departUtc = departUtc;
    d.cancelled = dep["isCancelled"] | false;
    d.color = parseHexColor(dep["serviceJourney"]["line"]["backgroundColor"] | "");
    d.textColor = parseHexColor(dep["serviceJourney"]["line"]["foregroundColor"] | "");
  }

  return board.count > 0 ? FetchResult::Ok : FetchResult::NoValidDepartures;
}

}  // namespace

void setStopId(const String &id) {
  stopId = id;
}

int searchStops(const String &query, StopMatch *results, int maxResults) {
  String url = String(config::kLocationSearchUrl) + urlEncode(query) + "&types=stoparea&limit=" + String(maxResults);

  HTTPClient https;
  int httpCode = 0;
  for (int attempt = 0; attempt < 2; attempt++) {  // 2nd attempt after a token refresh
    if (!https.begin(secureClient, url)) return -1;
    https.addHeader("Authorization", "Bearer " + accessToken);
    httpCode = https.GET();
    if (httpCode != 401) break;
    https.end();
    if (!requestToken()) return -1;
  }
  if (httpCode != 200) {
    Serial.printf("Stop search failed: HTTP %d\n", httpCode);
    https.end();
    return -1;
  }

  StaticJsonDocument<128> filter;
  filter["results"][0]["gid"] = true;
  filter["results"][0]["name"] = true;

  DynamicJsonDocument doc(4096);
  DeserializationError error = deserializeJson(doc, https.getString(), DeserializationOption::Filter(filter));
  https.end();
  if (error) {
    Serial.printf("Stop search parse error: %s\n", error.c_str());
    return -1;
  }

  int count = 0;
  for (JsonObject item : doc["results"].as<JsonArray>()) {
    if (count >= maxResults) break;
    results[count].id = item["gid"].as<String>();
    results[count].name = item["name"].as<String>();
    count++;
  }
  return count;
}

bool hasToken() {
  return accessToken.length() > 0;
}

bool requestToken() {
  Serial.printf("Free heap before requestToken: %u bytes\n", ESP.getFreeHeap());
  accessToken = "";

  HTTPClient https;
  if (!https.begin(secureClient, config::kTokenUrl)) {
    Serial.println("Token HTTPS begin failed");
    return false;
  }

  https.addHeader("Content-Type", "application/x-www-form-urlencoded");
  String credentials = String(secrets::kClientId) + ":" + String(secrets::kClientSecret);
  https.addHeader("Authorization", "Basic " + base64Encode(credentials));

  int httpCode = https.POST("grant_type=client_credentials&scope=default");
  if (httpCode == 200) {
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, https.getString());
    if (!error && doc.containsKey("access_token")) {
      accessToken = doc["access_token"].as<String>();
      Serial.println("Token obtained");
    } else {
      Serial.printf("Token parse error: %s\n", error.c_str());
    }
  } else {
    Serial.printf("Token request failed: HTTP %d\n", httpCode);
  }
  https.end();

  Serial.printf("Free heap after requestToken: %u bytes\n", ESP.getFreeHeap());
  return hasToken();
}

FetchResult fetchDepartures(DepartureBoard &board) {
  String url = String(config::kDepartureUrlBase) + stopId +
               "/departures?timeSpanInMinutes=60&limit=" + String(config::kMaxDepartures);
  Serial.printf("Fetching departures from: %s\n", url.c_str());
  Serial.printf("Free heap before fetchDepartures: %u bytes\n", ESP.getFreeHeap());

  FetchResult result = FetchResult::RetriesExhausted;
  HTTPClient https;

  for (int attempt = 1; attempt <= config::kMaxRetries; attempt++) {
    if (!https.begin(secureClient, url)) {
      Serial.println("HTTPS begin failed");
      result = FetchResult::ConnectionError;
      continue;
    }

    https.addHeader("Authorization", "Bearer " + accessToken);
    int httpCode = https.GET();
    Serial.printf("Attempt %d - HTTP code: %d\n", attempt, httpCode);

    if (httpCode == 200) {
      String payload = https.getString();
      https.end();
      Serial.printf("Payload size: %d bytes\n", payload.length());

      result = parseDepartures(payload, board);
      if (result != FetchResult::ParseError) break;  // done (even if empty)
    } else if (httpCode == 429 || httpCode == 503) {
      Serial.printf("Retryable error: %d\n", httpCode);
      https.end();
      delay(1000);
    } else if (httpCode == 401) {
      Serial.println("Token expired, getting new token...");
      https.end();
      requestToken();
    } else {
      Serial.printf("HTTP error: %d\n", httpCode);
      https.end();
      result = FetchResult::HttpError;
    }
  }

  Serial.printf("Free heap after fetchDepartures: %u bytes\n", ESP.getFreeHeap());
  return result;
}
