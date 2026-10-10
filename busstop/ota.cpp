#include "ota.h"

#include <ArduinoOTA.h>
#include <lvgl.h>

#include "backlight.h"
#include "config.h"
#include "ui.h"

namespace {
bool started = false;
}

void otaBegin(const String &password) {
  if (started) return;
  if (password.isEmpty()) {
    Serial.println("OTA off: no update password set");
    return;
  }

  ArduinoOTA.setHostname(config::kOtaHostname);
  ArduinoOTA.setPassword(password.c_str());

  ArduinoOTA.onStart([]() {
    Serial.println("OTA update started");
    backlightWake();
    uiSetStatus("Updating... 0%");
    lv_timer_handler();
  });
  ArduinoOTA.onProgress([](unsigned int done, unsigned int total) {
    static int lastPercent = -1;
    int percent = total ? (int)(done * 100ULL / total) : 0;
    if (percent == lastPercent) return;
    lastPercent = percent;
    uiSetStatus("Updating... " + String(percent) + "%");
    lv_timer_handler();
  });
  ArduinoOTA.onEnd([]() {
    uiSetStatus("Update done, restarting");
    lv_timer_handler();
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("OTA error %u\n", error);
    uiSetStatus("Update failed");
    lv_timer_handler();
  });

  ArduinoOTA.begin();
  started = true;
  Serial.printf("OTA ready: %s.local\n", config::kOtaHostname);
}

void otaHandle() {
  if (started) ArduinoOTA.handle();
}
