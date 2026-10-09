// ============================================================================
// config.h - Project settings
// ============================================================================
#pragma once

namespace config {

// --- Display ---------------------------------------------------------------
constexpr int kScreenWidth = 480;
constexpr int kScreenHeight = 320;
constexpr int kDrawBufferLines = 10;  // LVGL draw buffer = width * lines pixels

// Colors: this value means "use the default"
constexpr unsigned long kNoColor = 0xFFFFFFFFUL;

// --- Hardware --------------------------------------------------------------
// LEDs are active LOW (HIGH = off)
constexpr int kLedPins[] = {4, 16, 17};

// --- Backlight (brightness follows the time of day) ---------------------------
constexpr int kBacklightPin = 27;  // same pin as TFT_BL in TFT_eSPI's User_Setup.h
constexpr float kBrightnessDay = 70.0f;    // percent
constexpr float kBrightnessNight = 20.0f;  // percent
constexpr int kMorningFadeStartMin = 6 * 60;   // 06:00 night -> day
constexpr int kDayStartMin = 7 * 60;           // 07:00 full day brightness
constexpr int kEveningFadeStartMin = 20 * 60;  // 20:00 day -> night
constexpr int kNightStartMin = 22 * 60;        // 22:00 night brightness

// --- Touch screen --------------------------------------------------------------
constexpr unsigned long kTouchPollMs = 30;
constexpr int kTouchSdaPin = 33;  // GT911 capacitive touch controller
constexpr int kTouchSclPin = 32;
constexpr int kTouchIntPin = 21;
constexpr int kTouchResetPin = 25;
constexpr unsigned long kTouchWakeMs = 30UL * 1000UL;   // tap = full brightness for this long
constexpr unsigned long kSetupHintHoldMs = 1500;        // hold this long to see the hint
constexpr unsigned long kSetupHoldMs = 4000;            // hold this long to open setup

// --- Connection health -----------------------------------------------------------
constexpr unsigned long kStaleAfterMs = 90UL * 1000UL;          // no fresh data -> "No connection"
constexpr unsigned long kWeatherMaxAgeMs = 60UL * 60UL * 1000UL;  // older temperature -> "--"
constexpr unsigned long kWifiSetupAfterMs = 15UL * 60UL * 1000UL; // WiFi down this long -> setup page

// --- Updates over WiFi -----------------------------------------------------------
static const char *const kOtaHostname = "busstop";

// --- Setup page ------------------------------------------------------------
static const char *const kSetupApName = "BusStop-Setup";
constexpr int kBootButtonPin = 0;                          // press after power-on to open setup
constexpr unsigned long kSetupButtonWindowMs = 2000;       // how long to listen for the button
constexpr unsigned long kSetupTimeoutMs = 5UL * 60UL * 1000UL;  // restart if nobody uses the page

// --- Västtrafik API --------------------------------------------------------
static const char *const kTokenUrl = "https://ext-api.vasttrafik.se/token";
static const char *const kDepartureUrlBase = "https://ext-api.vasttrafik.se/pr/v4/stop-areas/";
static const char *const kLocationSearchUrl = "https://ext-api.vasttrafik.se/pr/v4/locations/by-text?q=";
constexpr int kMaxStopResults = 8;
constexpr int kMaxDepartures = 5;
constexpr int kMaxRetries = 3;
constexpr unsigned long kDepartureIntervalMs = 15UL * 1000UL;

// --- Weather (Open-Meteo, no API key needed) -------------------------------
static const char *const kWeatherUrl =
    "https://api.open-meteo.com/v1/forecast?latitude=57.7089&longitude=11.9746&current=temperature_2m";
constexpr unsigned long kWeatherIntervalMs = 10UL * 60UL * 1000UL;  // 10 minutes
constexpr unsigned long kWeatherRetryMs = 30UL * 1000UL;            // after a failure

// --- Time ------------------------------------------------------------------
static const char *const kTimezone = "CET-1CEST-2,M3.5.0/2:00:00,M10.5.0/3:00:00";  // Sweden
static const char *const kNtpServer1 = "pool.ntp.org";
static const char *const kNtpServer2 = "time.nist.gov";
constexpr unsigned long kTimeResyncIntervalMs = 24UL * 60UL * 60UL * 1000UL;  // once a day

}  // namespace config
