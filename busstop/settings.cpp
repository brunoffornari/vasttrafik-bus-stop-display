#include "settings.h"

#include <Preferences.h>

// secrets.h is optional: it only gives developers default keys while testing.
// Everyone else enters their keys on the phone setup page.
#if __has_include("secrets.h")
#include "secrets.h"
#define HAVE_DEV_SECRETS 1
#endif

namespace {
constexpr const char *kNamespace = "busstop";
}

bool settingsLoad(Settings &settings) {
  Preferences prefs;
  if (prefs.begin(kNamespace, true)) {  // read-only; fails if nothing was saved yet
    settings.ssid = prefs.getString("ssid", "");
    settings.password = prefs.getString("password", "");
    settings.stopId = prefs.getString("stopId", "");
    settings.clientId = prefs.getString("clientId", "");
    settings.clientSecret = prefs.getString("clientSecret", "");
    settings.otaPassword = prefs.getString("otaPassword", "");
    prefs.end();
  }

#ifdef HAVE_DEV_SECRETS
  if (settings.clientId.isEmpty()) settings.clientId = secrets::kClientId;
  if (settings.clientSecret.isEmpty()) settings.clientSecret = secrets::kClientSecret;
  if (settings.otaPassword.isEmpty()) settings.otaPassword = secrets::kOtaPassword;
#endif

  return !settings.ssid.isEmpty() && !settings.stopId.isEmpty() && !settings.clientId.isEmpty() &&
         !settings.clientSecret.isEmpty();
}

void settingsSave(const Settings &settings) {
  Preferences prefs;
  prefs.begin(kNamespace, false);
  prefs.putString("ssid", settings.ssid);
  prefs.putString("password", settings.password);
  prefs.putString("stopId", settings.stopId);
  prefs.putString("clientId", settings.clientId);
  prefs.putString("clientSecret", settings.clientSecret);
  prefs.putString("otaPassword", settings.otaPassword);
  prefs.end();
}
