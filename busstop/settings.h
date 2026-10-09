// ============================================================================
// settings.h - User settings stored on the device (flash)
// ============================================================================
#pragma once
#include <Arduino.h>

struct Settings {
  String ssid;
  String password;
  String stopId;  // Västtrafik stop area gid
};

// Returns true if a complete set of settings was found.
bool settingsLoad(Settings &settings);
void settingsSave(const Settings &settings);
