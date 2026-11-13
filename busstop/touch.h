// ============================================================================
// touch.h - Touch screen: taps and long presses
// ============================================================================
#pragma once

// Starts the touch controller (after displayInit()).
void touchBegin();

// Call from loop(). Reads the touch panel a few times per second, debounced.
void touchPoll();

// True once for each new press.
bool touchWasPressed();

enum class Gesture { None, Tap, SwipeLeft, SwipeRight };

// A finished gesture (reported once, after the finger lifts). A long press is not a gesture.
Gesture touchGesture();

// How long the finger has been down (0 = not touching).
unsigned long touchHeldMs();
