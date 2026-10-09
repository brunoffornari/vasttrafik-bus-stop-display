// ============================================================================
// backlight.h - Screen brightness by time of day
// ============================================================================
#pragma once

// Starts PWM on the backlight pin. Call right after displayInit().
void backlightInit();

// Call from loop(): follows the day/night schedule with smooth fades.
void backlightUpdate();

// Full brightness for a while (call when the screen is touched).
void backlightWake();
