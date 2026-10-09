// ============================================================================
// wifi_manager.h - WiFi connection and the shared HTTPS client
// ============================================================================
#pragma once
#include <WiFiClientSecure.h>

// One secure client shared by all HTTPS requests.
extern WiFiClientSecure secureClient;

// Sets the network used by ensureWiFiConnected().
void wifiSetCredentials(const String &ssid, const String &password);

// Waits until connected or the timeout ends. Returns true if connected.
bool wifiWaitConnected(unsigned long timeoutMs);

// Returns true if connected (reconnects for up to 10 s when needed).
bool ensureWiFiConnected();

// Non-blocking: true if connected. When not, asks the chip to reconnect (at most
// once per call) and returns right away.
bool wifiIsConnectedOrRetry();
