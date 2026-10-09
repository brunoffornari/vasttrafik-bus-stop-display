// ============================================================================
// iso_time.h - Parse ISO 8601 timestamps to UTC (no Arduino/timezone needed)
// ============================================================================
#pragma once
#include <stdint.h>
#include <stdio.h>
#include <string.h>

// Days since 1970-01-01 for a calendar date (proleptic Gregorian).
inline int64_t daysFromCivil(int year, unsigned month, unsigned day) {
  year -= month <= 2;
  const int64_t era = (year >= 0 ? year : year - 399) / 400;
  const unsigned yoe = (unsigned)(year - era * 400);
  const unsigned doy = (153 * (month + (month > 2 ? -3 : 9)) + 2) / 5 + day - 1;
  const unsigned doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
  return era * 146097 + (int64_t)doe - 719468;
}

// Converts "2026-10-09T21:35:00+02:00" (also "Z" or fractional seconds) to UTC
// seconds. The offset in the text is used; if there is none, fallbackOffsetSeconds.
inline bool isoToUtc(const char *iso, int fallbackOffsetSeconds, int64_t &utc) {
  int year, month, day, hour, minute, second;
  if (sscanf(iso, "%d-%d-%dT%d:%d:%d", &year, &month, &day, &hour, &minute, &second) != 6) return false;

  const char *p = iso + 19;  // first character after "YYYY-MM-DDTHH:MM:SS"
  if (strlen(iso) < 19) return false;
  if (*p == '.') {
    p++;
    while (*p >= '0' && *p <= '9') p++;
  }

  int offset = fallbackOffsetSeconds;
  if (*p == 'Z') {
    offset = 0;
  } else if (*p == '+' || *p == '-') {
    int offHour = 0, offMinute = 0;
    if (sscanf(p + 1, "%2d:%2d", &offHour, &offMinute) >= 1) {
      offset = (offHour * 3600 + offMinute * 60) * (*p == '-' ? -1 : 1);
    }
  }

  utc = daysFromCivil(year, (unsigned)month, (unsigned)day) * 86400 + hour * 3600 + minute * 60 + second - offset;
  return true;
}
