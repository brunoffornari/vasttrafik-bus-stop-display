// ============================================================================
// vasttrafik.h - Västtrafik API (OAuth2 token + departures)
// ============================================================================
#pragma once
#include <Arduino.h>

#include "config.h"

struct Departure {
  String line;       // e.g. "18"
  String direction;  // e.g. "Kalleback"
  int64_t departUtc = 0;   // real-time departure (UTC seconds)
  bool cancelled = false;
  uint32_t color = config::kNoColor;      // line badge background
  uint32_t textColor = config::kNoColor;  // line badge text
};

struct DepartureBoard {
  String stopName;  // empty if the API did not return one
  Departure items[config::kMaxDepartures];
  int count = 0;
};

enum class FetchResult {
  Ok,
  NoDepartures,       // API returned an empty list
  NoValidDepartures,  // list had entries, none usable
  ParseError,
  HttpError,
  ConnectionError,
  RetriesExhausted,   // only 429/503/401 answers
};

struct StopMatch {
  String id;    // stop area gid
  String name;  // e.g. "Gräddgatan, Göteborg"
};

// Chooses which stop fetchDepartures() reads.
void setStopId(const String &stopId);

// Searches stops by name. Returns the number found, or -1 on failure.
int searchStops(const String &query, StopMatch *results, int maxResults);

bool hasToken();

// Gets a new access token. Returns true on success.
bool requestToken();

// Fetches the next departures. Gets a new token on HTTP 401.
FetchResult fetchDepartures(DepartureBoard &board);
