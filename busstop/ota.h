// ============================================================================
// ota.h - Update the firmware over WiFi
// ============================================================================
#pragma once

// Starts listening for updates (call once WiFi is connected).
void otaBegin();

// Call from loop().
void otaHandle();
