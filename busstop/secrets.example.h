// ============================================================================
// secrets.example.h - Copy this file to secrets.h and fill in your values
// ============================================================================
// secrets.h is ignored by git. Never commit your real keys.
#pragma once

namespace secrets {

// Västtrafik OAuth2 client credentials (create an application at
// https://developer.vasttrafik.se to get them)
static const char *const kClientId = "YOUR_CLIENT_ID";
static const char *const kClientSecret = "YOUR_CLIENT_SECRET";

// Password for updating the device over WiFi (choose any password)
static const char *const kOtaPassword = "CHOOSE_A_PASSWORD";

}  // namespace secrets
