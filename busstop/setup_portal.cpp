#include "setup_portal.h"

#include <ArduinoJson.h>
#include <DNSServer.h>
#include <WebServer.h>
#include <WiFi.h>
#include <lvgl.h>

#include "config.h"
#include "settings.h"
#include "setup_page.h"
#include "ui.h"
#include "vasttrafik.h"

namespace {

enum class State { Idle, Connecting, Connected, Failed };

constexpr unsigned long kConnectTimeoutMs = 20000;
constexpr unsigned long kRestartDelayMs = 1500;

WebServer server(80);
DNSServer dns;

State state = State::Idle;
String errorMessage;
String pendingSsid;
String pendingPassword;
String pendingClientId;
String pendingClientSecret;
String pendingOtaPassword;
unsigned long connectStartedMs = 0;
unsigned long lastActivityMs = 0;
unsigned long restartAtMs = 0;  // 0 = not scheduled

void sendJson(const JsonDocument &doc, int code = 200) {
  String out;
  serializeJson(doc, out);
  server.send(code, "application/json; charset=utf-8", out);
}

void sendError(int code, const char *message) {
  DynamicJsonDocument doc(128);
  doc["error"] = message;
  sendJson(doc, code);
}

void markActivity() {
  lastActivityMs = millis();
}

// --- Routes ----------------------------------------------------------------

void handleRoot() {
  markActivity();
  server.send_P(200, "text/html; charset=utf-8", kSetupPage);
}

void handleScan() {
  markActivity();
  int found = WiFi.scanNetworks();

  DynamicJsonDocument doc(3072);
  JsonArray names = doc.to<JsonArray>();
  for (int i = 0; i < found && names.size() < 15; i++) {
    String ssid = WiFi.SSID(i);
    if (ssid.isEmpty()) continue;
    bool duplicate = false;
    for (JsonVariant existing : names) {
      if (ssid == existing.as<String>()) duplicate = true;
    }
    if (!duplicate) names.add(ssid);
  }
  WiFi.scanDelete();
  sendJson(doc);
}

void handleConnect() {
  markActivity();
  pendingSsid = server.arg("ssid");
  pendingPassword = server.arg("password");
  pendingClientId = server.arg("clientId");
  pendingClientSecret = server.arg("clientSecret");
  pendingOtaPassword = server.arg("otaPassword");
  pendingClientId.trim();
  pendingClientSecret.trim();
  if (pendingSsid.isEmpty()) {
    sendError(400, "Missing network name");
    return;
  }
  if (pendingClientId.isEmpty() || pendingClientSecret.isEmpty()) {
    sendError(400, "Missing Västtrafik key");
    return;
  }
  setCredentials(pendingClientId, pendingClientSecret);

  Serial.printf("Setup: connecting to %s\n", pendingSsid.c_str());
  uiSetSetupStatus("Connecting to " + pendingSsid + "...");
  WiFi.disconnect(false);
  WiFi.begin(pendingSsid.c_str(), pendingPassword.c_str());
  state = State::Connecting;
  connectStartedMs = millis();
  errorMessage = "";

  DynamicJsonDocument doc(64);
  doc["ok"] = true;
  sendJson(doc);
}

void handleStatus() {
  markActivity();
  DynamicJsonDocument doc(256);
  switch (state) {
    case State::Idle:       doc["state"] = "idle"; break;
    case State::Connecting: doc["state"] = "connecting"; break;
    case State::Connected:  doc["state"] = "connected"; break;
    case State::Failed:     doc["state"] = "failed"; break;
  }
  if (state == State::Failed) doc["error"] = errorMessage;
  sendJson(doc);
}

void handleStops() {
  markActivity();
  if (state != State::Connected) {
    sendError(409, "Connect to WiFi first");
    return;
  }

  StopMatch matches[config::kMaxStopResults];
  int count = searchStops(server.arg("q"), matches, config::kMaxStopResults);
  if (count < 0) {
    sendError(502, "Stop search failed");
    return;
  }

  DynamicJsonDocument doc(2048);
  JsonArray list = doc.to<JsonArray>();
  for (int i = 0; i < count; i++) {
    JsonObject item = list.createNestedObject();
    item["id"] = matches[i].id;
    item["name"] = matches[i].name;
  }
  sendJson(doc);
}

void handleSave() {
  markActivity();
  String stopId = server.arg("stopId");
  if (state != State::Connected || stopId.isEmpty()) {
    sendError(400, "Nothing to save");
    return;
  }

  Settings settings;
  settings.ssid = pendingSsid;
  settings.password = pendingPassword;
  settings.stopId = stopId;
  settings.clientId = pendingClientId;
  settings.clientSecret = pendingClientSecret;
  settings.otaPassword = pendingOtaPassword;
  settingsSave(settings);
  Serial.println("Setup: settings saved, restarting");
  uiSetSetupStatus("Saved! Restarting...");

  DynamicJsonDocument doc(64);
  doc["ok"] = true;
  sendJson(doc);
  restartAtMs = millis() + kRestartDelayMs;
}

// Any other address (captive portal checks) goes to the setup page.
void handleNotFound() {
  server.sendHeader("Location", "http://" + WiFi.softAPIP().toString() + "/");
  server.send(302, "text/plain", "");
}

// --- Background work -------------------------------------------------------

void updateConnection() {
  if (state != State::Connecting) return;

  if (WiFi.status() == WL_CONNECTED) {
    if (requestToken()) {
      state = State::Connected;
      uiSetSetupStatus("WiFi OK. Pick your stop.");
    } else {
      state = State::Failed;
      errorMessage = "Connected to WiFi, but Västtrafik did not accept the key. Check the Client ID and Secret.";
      uiSetSetupStatus("Cannot reach Vasttrafik");
    }
  } else if (millis() - connectStartedMs > kConnectTimeoutMs) {
    state = State::Failed;
    errorMessage = "Could not connect. Check the password.";
    uiSetSetupStatus("WiFi failed");
  }
}

}  // namespace

void runSetupPortal() {
  Serial.println("Starting setup portal");

  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(config::kSetupApName);
  delay(100);
  dns.start(53, "*", WiFi.softAPIP());

  server.on("/", HTTP_GET, handleRoot);
  server.on("/scan", HTTP_GET, handleScan);
  server.on("/connect", HTTP_POST, handleConnect);
  server.on("/status", HTTP_GET, handleStatus);
  server.on("/stops", HTTP_GET, handleStops);
  server.on("/save", HTTP_POST, handleSave);
  server.onNotFound(handleNotFound);
  server.begin();

  uiCreateSetup(config::kSetupApName);
  markActivity();

  while (true) {
    lv_timer_handler();
    dns.processNextRequest();
    server.handleClient();
    updateConnection();

    if (restartAtMs != 0 && millis() >= restartAtMs) ESP.restart();
    if (millis() - lastActivityMs > config::kSetupTimeoutMs) {
      Serial.println("Setup timed out, restarting");
      ESP.restart();
    }
    delay(5);
  }
}
