// ============================================================================
// weather.h - Current temperature (Open-Meteo)
// ============================================================================
#pragma once

// Gets the current temperature in °C (rounded). Returns false on failure.
// latitude/longitude 0 = use the default location (Göteborg).
bool fetchTemperature(int &celsius);
void setWeatherLocation(float latitude, float longitude);
