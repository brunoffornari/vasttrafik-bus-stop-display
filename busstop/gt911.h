// ============================================================================
// gt911.h - GT911 capacitive touch controller (I2C)
// ============================================================================
#pragma once

// Resets the chip and starts I2C. Call once, after displayInit().
void gt911Begin();

// True while at least one finger is on the screen.
bool gt911Touched();

// Position of the last touch point (raw panel units). False if there has been none yet.
bool gt911Position(int &x, int &y);
