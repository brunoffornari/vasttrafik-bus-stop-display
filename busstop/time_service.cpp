#include "time_service.h"

#include <WiFi.h>
#include <time.h>

#include "config.h"
#include "iso_time.h"

namespace {

constexpr time_t kMinValidTime = 24 * 3600;  // anything below this = clock not set yet

unsigned long lastSyncMs = 0;

// Sets the timezone (with automatic summer/winter time) and starts NTP.
void startNtp() {
  configTzTime(config::kTimezone, config::kNtpServer1, config::kNtpServer2);
}

}  // namespace

void timeInit() {
  startNtp();
  Serial.println("Waiting for time sync...");
  time_t now = 0;
  while (now < kMinValidTime && WiFi.status() == WL_CONNECTED) {
    delay(100);
    now = time(nullptr);
  }
  Serial.println(now < kMinValidTime ? "Time sync failed" : "Time synced");
  Serial.printf("Local time: %s (UTC%+d)\n", localTimeString().c_str(), timezoneOffsetSeconds() / 3600);
  lastSyncMs = millis();
}

void timeResyncIfDue() {
  if (millis() - lastSyncMs < config::kTimeResyncIntervalMs) return;
  if (WiFi.status() != WL_CONNECTED) return;  // try again on the next loop

  Serial.println("Daily time re-sync...");
  startNtp();
  lastSyncMs = millis();
}

bool localTimeNow(struct tm &out) {
  time_t now = time(nullptr);
  if (now < kMinValidTime) return false;
  localtime_r(&now, &out);
  return true;
}

int timezoneOffsetSeconds() {
  struct tm local;
  if (!localTimeNow(local)) return 3600;
  return local.tm_isdst > 0 ? 7200 : 3600;  // CEST : CET
}

String localTimeString() {
  struct tm local;
  localTimeNow(local);

  char buffer[6];
  strftime(buffer, sizeof(buffer), "%H:%M", &local);
  return String(buffer);
}

String localDateString() {
  struct tm local;
  localTimeNow(local);

  char buffer[16];
  strftime(buffer, sizeof(buffer), "%a, %b ", &local);
  return String(buffer) + String(local.tm_mday);
}

bool isoTimeToUtc(const String &isoTime, int64_t &utcSeconds) {
  return isoToUtc(isoTime.c_str(), timezoneOffsetSeconds(), utcSeconds);
}

int minutesUntilUtc(int64_t utcSeconds) {
  return (int)((utcSeconds - (int64_t)time(nullptr)) / 60);
}
