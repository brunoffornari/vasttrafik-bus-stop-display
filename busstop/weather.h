// ============================================================================
// weather.h - Current temperature and 5-day forecast (Open-Meteo)
// ============================================================================
#pragma once
#include <Arduino.h>

#include "config.h"

struct ForecastDay {
  char date[11] = "";   // "2026-11-13"
  int code = 0;         // WMO weather code
  int maxC = 0;
  int minC = 0;
  int rainPercent = 0;  // chance of rain/snow
};

struct Weather {
  int currentC = 0;
  int currentCode = 0;
  bool isDay = true;
  int feelsC = 0;
  int humidity = 0;      // %
  int windMs = 0;        // m/s
  char sunrise[6] = "";  // "07:42"
  char sunset[6] = "";
  ForecastDay days[config::kForecastDays];
  int dayCount = 0;
};

// Gets the weather. Returns false on failure (the old data stays valid then).
bool fetchWeather(Weather &weather);

// Sets where the weather is for. 0/0 keeps the default (Göteborg).
void setWeatherLocation(float latitude, float longitude);
