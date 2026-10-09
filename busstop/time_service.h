// ============================================================================
// time_service.h - NTP sync, timezone and time math
// ============================================================================
#pragma once
#include <Arduino.h>
#include <time.h>

// Sets the timezone, starts NTP and waits until the time is valid.
void timeInit();

// Re-syncs with NTP once a day (call from loop()).
void timeResyncIfDue();

// Fills `out` with the current local time (summer/winter time included).
// Returns false until the clock has been set.
bool localTimeNow(struct tm &out);

// Current Swedish offset from UTC in seconds (3600 = CET, 7200 = CEST).
int timezoneOffsetSeconds();

// Current local time as "HH:MM".
String localTimeString();

// Current local date, e.g. "Fri, Oct 9".
String localDateString();

// Converts an ISO 8601 time ("2025-11-04T13:42:00+01:00") to UTC seconds.
// Returns false if the text cannot be parsed.
bool isoTimeToUtc(const String &isoTime, int64_t &utcSeconds);

// Whole minutes from now until a UTC time (negative = already gone).
int minutesUntilUtc(int64_t utcSeconds);
