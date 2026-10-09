// ============================================================================
// weather.h - Current temperature (Open-Meteo)
// ============================================================================
#pragma once

// Gets the current temperature in °C (rounded). Returns false on failure.
bool fetchTemperature(int &celsius);
