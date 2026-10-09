#include "settings.h"

#include <Preferences.h>

namespace {
constexpr const char *kNamespace = "busstop";
}

bool settingsLoad(Settings &settings) {
  Preferences prefs;
  if (!prefs.begin(kNamespace, true)) return false;  // nothing saved yet
  settings.ssid = prefs.getString("ssid", "");
  settings.password = prefs.getString("password", "");
  settings.stopId = prefs.getString("stopId", "");
  prefs.end();
  return !settings.ssid.isEmpty() && !settings.stopId.isEmpty();
}

void settingsSave(const Settings &settings) {
  Preferences prefs;
  prefs.begin(kNamespace, false);
  prefs.putString("ssid", settings.ssid);
  prefs.putString("password", settings.password);
  prefs.putString("stopId", settings.stopId);
  prefs.end();
}
