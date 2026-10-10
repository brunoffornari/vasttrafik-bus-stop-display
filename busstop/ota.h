// ============================================================================
// ota.h - Update the firmware over WiFi
// ============================================================================
#pragma once
#include <Arduino.h>

// Starts listening for updates (call once WiFi is connected).
// Without a password, updates over WiFi stay off.
void otaBegin(const String &password);

// Call from loop().
void otaHandle();
