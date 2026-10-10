// ============================================================================
// Bus Stop Display (ESP32)
// ============================================================================
// Shows the clock, the temperature and the next departures from a Västtrafik
// stop on a TFT display (LVGL).
//
//   busstop.ino      - setup() and loop()
//   config.h         - settings (intervals, timezone, brightness, touch ...)
//   secrets.h        - optional developer defaults (not needed: use the setup page)
//   display_driver   - TFT + LVGL setup
//   gt911            - touch controller (I2C)
//   backlight        - brightness by time of day (smooth fades, tap to wake)
//   touch            - taps and long presses
//   ui               - screen layout
//   wifi_manager     - WiFi connection, shared HTTPS client
//   time_service     - NTP (daily re-sync), summer/winter time, time math
//   vasttrafik       - OAuth2 token and departures
//   weather          - temperature from Open-Meteo
//   settings         - WiFi + stop saved on the device
//   setup_portal     - phone setup page (hotspot "BusStop-Setup")
//   ota              - firmware updates over WiFi
//   utils, iso_time  - small helpers
// ============================================================================

#include <Arduino.h>
#include <WiFi.h>
#include <esp_system.h>
#include <lvgl.h>

#include "backlight.h"
#include "config.h"
#include "display_driver.h"
#include "ota.h"
#include "settings.h"
#include "setup_portal.h"
#include "time_service.h"
#include "touch.h"
#include "ui.h"
#include "vasttrafik.h"
#include "weather.h"
#include "wifi_manager.h"

namespace {

// --- State -------------------------------------------------------------------
unsigned long lastDepartureUpdate = 0;
unsigned long lastWeatherUpdate = 0;
unsigned long lastClockUpdate = 0;

DepartureBoard shownBoard;       // last good departures (kept while offline)
bool boardValid = false;
unsigned long lastGoodFetchMs = 0;

bool weatherOk = false;          // false = retry sooner
unsigned long lastWeatherSuccessMs = 0;

unsigned long wifiDownSinceMs = 0;  // 0 = WiFi is fine

// --- Touch ---------------------------------------------------------------------

// True if the BOOT button is pressed within a short window after power-on.
bool setupButtonPressed() {
  pinMode(config::kBootButtonPin, INPUT_PULLUP);
  unsigned long start = millis();
  while (millis() - start < config::kSetupButtonWindowMs) {
    if (digitalRead(config::kBootButtonPin) == LOW) return true;
    delay(10);
  }
  return false;
}

// Small orange text next to the date: touch hint or connection warning.
void updateNotice(unsigned long now) {
  unsigned long held = touchHeldMs();
  if (held >= config::kSetupHintHoldMs) {
    unsigned long left = (config::kSetupHoldMs - held) / 1000 + 1;
    uiSetNotice(held >= config::kSetupHoldMs ? "Opening setup..." : "Hold for setup " + String(left) + "s");
  } else if (boardValid && now - lastGoodFetchMs > config::kStaleAfterMs) {
    uiSetNotice("No connection");
  } else {
    uiSetNotice("");
  }
}

void handleTouch(unsigned long now) {
  touchPoll();
  if (touchWasPressed()) backlightWake();

  updateNotice(now);
  if (touchHeldMs() >= config::kSetupHoldMs) {
    Serial.println("Long press: opening setup");
    runSetupPortal();
  }
}

// --- Departures ----------------------------------------------------------------

// Draws the last good departures, counting minutes down from the clock.
void renderBoard(unsigned long now) {
  if (!boardValid) return;

  int shown = 0;
  for (int i = 0; i < shownBoard.count && shown < config::kMaxDepartures; i++) {
    const Departure &d = shownBoard.items[i];
    int minutes = minutesUntilUtc(d.departUtc);
    if (minutes < 0) continue;  // already left
    uiSetRow(shown++, d.line, d.direction, minutes, d.color, d.textColor, d.cancelled);
  }

  if (shown == 0) uiSetStatus("No departures found");
  else uiClearRowsFrom(shown);

  uiSetStale(now - lastGoodFetchMs > config::kStaleAfterMs);
}

// Handles the result of a departures request.
void showBoard(FetchResult result, const DepartureBoard &board, unsigned long now) {
  switch (result) {
    case FetchResult::Ok:
    case FetchResult::NoDepartures:
    case FetchResult::NoValidDepartures:
      lastGoodFetchMs = now;
      uiSetStopName(board.stopName.isEmpty() ? "Loading Stop" : board.stopName);
      break;
    default:
      break;
  }

  switch (result) {
    case FetchResult::Ok:
      shownBoard = board;
      boardValid = true;
      for (int i = 0; i < board.count; i++) {
        const Departure &d = board.items[i];
        Serial.printf("Added departure: %s - %s  %d min%s\n", d.line.c_str(), d.direction.c_str(),
                      minutesUntilUtc(d.departUtc), d.cancelled ? "  (cancelled)" : "");
      }
      renderBoard(now);
      break;
    case FetchResult::NoDepartures:
      boardValid = false;
      uiSetStatus("No departures found");
      break;
    case FetchResult::NoValidDepartures:
      boardValid = false;
      uiSetStatus("No valid departures");
      break;
    // Errors: keep showing the last good data (it turns grey after a while)
    case FetchResult::ParseError:
      if (!boardValid) uiSetStatus("Error loading departures");
      break;
    case FetchResult::HttpError:
      if (!boardValid) uiSetStatus("HTTP error");
      break;
    case FetchResult::ConnectionError:
      if (!boardValid) uiSetStatus("Connection error");
      break;
    case FetchResult::RetriesExhausted:
      Serial.printf("Failed after %d retries\n", config::kMaxRetries);
      break;
  }
}

void updateDepartures(unsigned long now) {
  if (!wifiIsConnectedOrRetry()) {
    if (!boardValid) uiSetStatus("WiFi error");
    if (wifiDownSinceMs == 0) wifiDownSinceMs = now;
    if (now - wifiDownSinceMs > config::kWifiSetupAfterMs) {
      Serial.println("WiFi down for too long, opening setup");
      runSetupPortal();
    }
    return;
  }
  wifiDownSinceMs = 0;

  if (!hasToken()) {
    Serial.println("Getting new token...");
    requestToken();
    return;
  }

  Serial.println("Getting departures...");
  DepartureBoard board;
  showBoard(fetchDepartures(board), board, now);
}

// --- Weather -------------------------------------------------------------------

void updateWeather(unsigned long now) {
  int celsius;
  weatherOk = fetchTemperature(celsius);
  if (weatherOk) {
    uiSetTemperature(celsius);
    lastWeatherSuccessMs = now;
  }
}

}  // namespace

void setup() {
  Serial.begin(115200);
  Serial.printf("Reboot reason: %d\n", esp_reset_reason());
  Serial.printf("Free heap at setup start: %u bytes\n", ESP.getFreeHeap());

  for (int pin : config::kLedPins) {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, HIGH);  // off
  }

  secureClient.setInsecure();
  displayInit();
  backlightInit();
  touchBegin();

  Settings settings;
  bool configured = settingsLoad(settings);
  if (!configured || setupButtonPressed()) runSetupPortal();

  wifiSetCredentials(settings.ssid, settings.password);
  setCredentials(settings.clientId, settings.clientSecret);
  setStopId(settings.stopId);
  if (!ensureWiFiConnected()) {
    Serial.println("WiFi connection failed, opening setup");
    runSetupPortal();
  }

  timeInit();
  otaBegin(settings.otaPassword);
  Serial.println("Getting token...");
  requestToken();

  uiCreate();
  uiSetClock(localTimeString());
  uiSetDate(localDateString());
  updateWeather(millis());
  lastWeatherUpdate = millis();

  Serial.printf("Free heap at setup end: %u bytes\n", ESP.getFreeHeap());
}

void loop() {
  unsigned long now = millis();
  lv_timer_handler();
  handleTouch(now);

  if (now - lastClockUpdate >= 1000) {
    static bool colonVisible = true;
    colonVisible = !colonVisible;
    uiSetClockColon(colonVisible);
    uiSetClock(localTimeString());
    uiSetDate(localDateString());
    renderBoard(now);
    if (lastWeatherSuccessMs != 0 && now - lastWeatherSuccessMs > config::kWeatherMaxAgeMs) uiSetTemperatureUnknown();
    lastClockUpdate = now;
  }

  if (now - lastDepartureUpdate > config::kDepartureIntervalMs) {
    Serial.printf("Free heap: %u bytes\n", ESP.getFreeHeap());
    updateDepartures(now);
    lastDepartureUpdate = now;
  }

  unsigned long weatherInterval = weatherOk ? config::kWeatherIntervalMs : config::kWeatherRetryMs;
  if (now - lastWeatherUpdate > weatherInterval) {
    if (WiFi.status() == WL_CONNECTED) updateWeather(now);
    lastWeatherUpdate = now;
  }

  timeResyncIfDue();
  backlightUpdate();
  otaHandle();

  delay(5);
}
