#include "weather.h"

#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <math.h>

#include "config.h"
#include "wifi_manager.h"

bool fetchTemperature(int &celsius) {
  HTTPClient https;
  if (!https.begin(secureClient, config::kWeatherUrl)) {
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
