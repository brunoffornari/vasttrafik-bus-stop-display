// ============================================================================
// ui_extra.h - Weather, digital clock and analog clock pages
// ============================================================================
#pragma once
#include <Arduino.h>
#include <lvgl.h>

#include "weather.h"

// Builds the three pages inside the given (empty, full-screen) containers.
void uiExtraCreate(lv_obj_t *weatherPage, lv_obj_t *digitalPage, lv_obj_t *analogPage);

// Forgets all widgets (the setup screen deletes them).
void uiExtraReset();

// Shows the forecast. nullptr = no valid data: temperatures show "--".
void uiExtraSetWeather(const Weather *weather);

// Call once a second.
void uiExtraSetTime(int hour, int minute, int second, bool colonVisible, const String &date);
