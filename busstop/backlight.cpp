#include "backlight.h"

#include <Arduino.h>
#include <math.h>
#include <time.h>

#include "config.h"
#include "time_service.h"

namespace {

constexpr uint32_t kPwmFrequencyHz = 5000;
constexpr uint8_t kPwmBits = 10;
constexpr uint32_t kPwmMax = (1 << kPwmBits) - 1;
constexpr float kGamma = 2.2f;                 // eyes see brightness non-linearly
constexpr unsigned long kScheduleCheckMs = 10000;
constexpr unsigned long kFadeStepMs = 40;
constexpr float kFadeStepPercent = 1.0f;       // about 25 % per second when dimming
constexpr float kFadeUpStepPercent = 4.0f;     // brightening is faster

float currentPercent = config::kBrightnessDay;
float targetPercent = config::kBrightnessDay;
unsigned long lastScheduleCheck = 0;
unsigned long lastFadeStep = 0;
unsigned long wakeUntilMs = 0;  // 0 = not awake
bool wasAwake = false;

float lerp(float from, float to, float t) {
  return from + (to - from) * t;
}

// Brightness wanted right now (percent), from the local time of day.
float scheduledBrightness() {
  struct tm local;
  if (!localTimeNow(local)) return config::kBrightnessDay;  // clock not set yet
  int minute = local.tm_hour * 60 + local.tm_min;

  using namespace config;
  if (minute < kMorningFadeStartMin) return kBrightnessNight;
  if (minute < kDayStartMin) {
    return lerp(kBrightnessNight, kBrightnessDay,
                (float)(minute - kMorningFadeStartMin) / (kDayStartMin - kMorningFadeStartMin));
  }
  if (minute < kEveningFadeStartMin) return kBrightnessDay;
  if (minute < kNightStartMin) {
    return lerp(kBrightnessDay, kBrightnessNight,
                (float)(minute - kEveningFadeStartMin) / (kNightStartMin - kEveningFadeStartMin));
  }
  return kBrightnessNight;
}

void applyBrightness(float percent) {
  float duty = powf(percent / 100.0f, kGamma);
  ledcWrite(config::kBacklightPin, (uint32_t)(duty * kPwmMax + 0.5f));
}

}  // namespace

void backlightInit() {
  ledcAttach(config::kBacklightPin, kPwmFrequencyHz, kPwmBits);
  currentPercent = targetPercent = scheduledBrightness();
  applyBrightness(currentPercent);
}

void backlightUpdate() {
  unsigned long now = millis();

  bool awake = wakeUntilMs != 0 && (long)(wakeUntilMs - now) > 0;
  if (!awake) wakeUntilMs = 0;

  if (now - lastScheduleCheck >= kScheduleCheckMs || awake != wasAwake) {
    targetPercent = scheduledBrightness();
    if (awake && targetPercent < config::kBrightnessDay) targetPercent = config::kBrightnessDay;
    lastScheduleCheck = now;
    wasAwake = awake;
  }

  if (currentPercent == targetPercent || now - lastFadeStep < kFadeStepMs) return;
  lastFadeStep = now;

  float difference = targetPercent - currentPercent;
  float step = difference > 0 ? kFadeUpStepPercent : kFadeStepPercent;
  currentPercent = fabsf(difference) <= step ? targetPercent : currentPercent + (difference > 0 ? step : -step);
  applyBrightness(currentPercent);
}

void backlightWake() {
  wakeUntilMs = millis() + config::kTouchWakeMs;
  if (wakeUntilMs == 0) wakeUntilMs = 1;
}
