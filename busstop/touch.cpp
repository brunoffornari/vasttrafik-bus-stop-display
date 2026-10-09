#include "touch.h"

#include <Arduino.h>

#include "config.h"
#include "gt911.h"

namespace {

bool pressed = false;
bool pressEvent = false;
int disagreeing = 0;  // consecutive readings that differ from `pressed`
unsigned long pressStartMs = 0;
unsigned long lastPollMs = 0;

}  // namespace

void touchBegin() {
  gt911Begin();
}

void touchPoll() {
  unsigned long now = millis();
  if (now - lastPollMs < config::kTouchPollMs) return;
  lastPollMs = now;

  bool reading = gt911Touched();
  if (reading == pressed) {
    disagreeing = 0;
    return;
  }

  // Need 2 equal readings in a row to change state (filters electrical noise)
  if (++disagreeing < 2) return;
  disagreeing = 0;
  pressed = reading;
  if (pressed) {
    pressStartMs = now;
    pressEvent = true;
    Serial.println("Touch: pressed");
  } else {
    Serial.printf("Touch: released after %lu ms\n", now - pressStartMs);
  }
}

bool touchWasPressed() {
  bool event = pressEvent;
  pressEvent = false;
  return event;
}

unsigned long touchHeldMs() {
  return pressed ? millis() - pressStartMs : 0;
}
