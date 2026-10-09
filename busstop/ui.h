// ============================================================================
// ui.h - Screen layout (header + departure cards)
// ============================================================================
#pragma once
#include <Arduino.h>

void uiCreate();

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
