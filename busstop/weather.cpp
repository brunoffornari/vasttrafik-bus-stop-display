#include "weather.h"

#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <math.h>

#include "config.h"
#include "wifi_manager.h"

namespace {
float weatherLat = config::kDefaultLatitude;
float weatherLon = config::kDefaultLongitude;
}  // namespace

void setWeatherLocation(float latitude, float longitude) {
  if (latitude == 0 && longitude == 0) return;  // keep the default
  weatherLat = latitude;
  weatherLon = longitude;
}

bool fetchTemperature(int &celsius) {
  String url = String(config::kWeatherUrlBase) + "&latitude=" + String(weatherLat, 4) + "&longitude=" + String(weatherLon, 4);
  HTTPClient https;
  if (!https.begin(secureClient, url)) {
    Serial.println("Weather HTTPS begin failed");
    return false;
  }

  int httpCode = https.GET();
  if (httpCode != 200) {
    Serial.printf("Weather request failed: HTTP %d\n", httpCode);
    https.end();
    return false;
  }

  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, https.getString());
  https.end();

  if (error || doc["current"]["temperature_2m"].isNull()) {
    Serial.println("Weather parse error");
    return false;
  }

  celsius = (int)lroundf(doc["current"]["temperature_2m"].as<float>());
  Serial.printf("Temperature: %d C\n", celsius);
  return true;
}
