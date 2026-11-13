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

bool fetchWeather(Weather &weather) {
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

  DynamicJsonDocument doc(6144);
  DeserializationError error = deserializeJson(doc, https.getString());
  https.end();

  JsonArray dates = doc["daily"]["time"];
  if (error || doc["current"]["temperature_2m"].isNull() || dates.isNull() || dates.size() == 0) {
    Serial.println("Weather parse error");
    return false;
  }

  Weather result;
  result.currentC = (int)lroundf(doc["current"]["temperature_2m"].as<float>());
  result.currentCode = doc["current"]["weather_code"] | 0;
  result.isDay = (doc["current"]["is_day"] | 1) != 0;
  result.feelsC = (int)lroundf(doc["current"]["apparent_temperature"] | doc["current"]["temperature_2m"].as<float>());
  result.humidity = (int)lroundf(doc["current"]["relative_humidity_2m"] | 0.0f);
  result.windMs = (int)lroundf(doc["current"]["wind_speed_10m"] | 0.0f);
  const char *rise = doc["daily"]["sunrise"][0] | "";
  const char *set = doc["daily"]["sunset"][0] | "";
  if (strlen(rise) >= 16) strlcpy(result.sunrise, rise + 11, sizeof(result.sunrise));
  if (strlen(set) >= 16) strlcpy(result.sunset, set + 11, sizeof(result.sunset));
  for (size_t i = 0; i < dates.size() && result.dayCount < config::kForecastDays; i++) {
    ForecastDay &d = result.days[result.dayCount++];
    strlcpy(d.date, dates[i] | "", sizeof(d.date));
    d.code = doc["daily"]["weather_code"][i] | 0;
    d.maxC = (int)lroundf(doc["daily"]["temperature_2m_max"][i].as<float>());
    d.minC = (int)lroundf(doc["daily"]["temperature_2m_min"][i].as<float>());
    d.rainPercent = doc["daily"]["precipitation_probability_max"][i] | 0;
  }
  weather = result;
  Serial.printf("Temperature: %d C, %d forecast days\n", weather.currentC, weather.dayCount);
  return true;
}
