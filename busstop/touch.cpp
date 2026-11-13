#include "touch.h"

#include <Arduino.h>

#include "config.h"
#include <stdlib.h>

#include "gt911.h"

namespace {

bool pressed = false;
bool pressEvent = false;
int disagreeing = 0;  // consecutive readings that differ from `pressed`
unsigned long pressStartMs = 0;
unsigned long lastPollMs = 0;
int startX = 0, startY = 0, curX = 0, curY = 0;
Gesture pendingGesture = Gesture::None;

}  // namespace

void touchBegin() {
  gt911Begin();
}

void touchPoll() {
  unsigned long now = millis();
  if (now - lastPollMs < config::kTouchPollMs) return;
  lastPollMs = now;

  bool reading = gt911Touched();
  if (reading && pressed) gt911Position(curX, curY);
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
    gt911Position(startX, startY);
    curX = startX;
    curY = startY;
    Serial.println("Touch: pressed");
  } else {
    unsigned long heldMs = now - pressStartMs;
    // The screen's long side is the panel's raw Y axis; swap if your board differs (config.h).
    int along = config::kTouchSwipeUsesY ? curY - startY : curX - startX;
    int across = config::kTouchSwipeUsesY ? curX - startX : curY - startY;
    if (config::kTouchSwipeInvert) along = -along;
    Serial.printf("Touch: released after %lu ms, moved along %d across %d\n", heldMs, along, across);
    pendingGesture = Gesture::None;
    if (heldMs < config::kTapMaxMs) {
      if (abs(along) >= config::kSwipeMinPx && abs(along) > 2 * abs(across)) {
        pendingGesture = along < 0 ? Gesture::SwipeLeft : Gesture::SwipeRight;
      } else if (abs(along) < config::kTapSlopPx && abs(across) < config::kTapSlopPx) {
        pendingGesture = Gesture::Tap;
      }
    }
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

Gesture touchGesture() {
  Gesture g = pendingGesture;
  pendingGesture = Gesture::None;
  return g;
}
