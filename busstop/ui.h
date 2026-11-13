// ============================================================================
// ui.h - Screen layout (header + departure cards)
// ============================================================================
#pragma once
#include <Arduino.h>

#include "weather.h"

void uiCreate();

// Pages: 0 = departures, 1 = weather, 2 = digital clock, 3 = analog clock
constexpr int kUiPageCount = 4;
void uiShowPage(int page);  // wraps around
int uiPage();

// Weather page and clock pages. nullptr = no valid weather.
void uiSetWeather(const Weather *weather);
void uiSetTimeDetail(int hour, int minute, int second, bool colonVisible, const String &date);

// Setup mode screen: tells the user how to open the setup page
void uiCreateSetup(const char *accessPointName);
void uiSetSetupStatus(const String &message);

// Header
void uiSetStopName(const String &name);
void uiSetClock(const String &time);  // "HH:MM"
void uiSetClockColon(bool visible);   // blink by toggling this
void uiSetDate(const String &date);
void uiSetTemperature(int celsius);
void uiSetTemperatureUnknown();           // shows --
void uiSetNotice(const String &message);  // small orange text next to the date ("" hides it)

// Departure cards. Colors are 0xRRGGBB, or config::kNoColor for the default.
void uiSetRow(int row, const String &line, const String &destination, int minutes,
              uint32_t badgeColor, uint32_t badgeTextColor, bool cancelled);
void uiSetStale(bool stale);               // true = data is old: minutes turn grey
void uiClearRowsFrom(int firstRow);        // hides the cards from this row down
void uiSetStatus(const String &message);   // hides all cards, shows a message
