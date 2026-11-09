// ============================================================================
// settings.h - User settings stored on the device (flash)
// ============================================================================
#pragma once
#include <Arduino.h>

struct Settings {
  String ssid;
  String password;
  String stopId;  // Västtrafik stop area gid
  String clientId;      // Västtrafik API key (from developer.vasttrafik.se)
  String clientSecret;
  String otaPassword;   // empty = updates over WiFi are off
  float latitude = 0;   // stop position, for the weather (0 = not saved yet)
  float longitude = 0;
};

// Returns true if a complete set of settings was found.
// A developer's optional secrets.h fills in anything the device has not stored yet.
bool settingsLoad(Settings &settings);
void settingsSave(const Settings &settings);
