#include "wifi_manager.h"

#include <WiFi.h>

WiFiClientSecure secureClient;

namespace {
String savedSsid;
String savedPassword;
}  // namespace

void wifiSetCredentials(const String &ssid, const String &password) {
  savedSsid = ssid;
  savedPassword = password;
}

bool wifiWaitConnected(unsigned long timeoutMs) {
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < timeoutMs) {
    delay(500);
    Serial.print(".");
  }
  return WiFi.status() == WL_CONNECTED;
}

bool ensureWiFiConnected() {
  if (WiFi.status() == WL_CONNECTED) return true;

  Serial.println("WiFi disconnected, reconnecting...");
  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  WiFi.begin(savedSsid.c_str(), savedPassword.c_str());

  if (wifiWaitConnected(10000)) {
    Serial.println("\nWiFi reconnected");
    return true;
  }
  Serial.println("\nWiFi reconnection failed");
  return false;
}

bool wifiIsConnectedOrRetry() {
  if (WiFi.status() == WL_CONNECTED) return true;
  Serial.println("WiFi down, retrying in the background...");
  WiFi.reconnect();
  return false;
}
