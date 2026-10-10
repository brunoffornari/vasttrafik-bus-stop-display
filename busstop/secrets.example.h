// ============================================================================
// secrets.example.h - OPTIONAL developer defaults
// ============================================================================
// You do NOT need this file. Every display gets its Västtrafik key, WiFi and
// update password from the phone setup page.
//
// For developers: copy this file to secrets.h and the firmware uses these values
// whenever the device has none stored (handy while testing). secrets.h is ignored
// by git. Never publish a firmware file that was built with a secrets.h.
#pragma once

namespace secrets {

static const char *const kClientId = "YOUR_CLIENT_ID";
static const char *const kClientSecret = "YOUR_CLIENT_SECRET";
static const char *const kOtaPassword = "CHOOSE_A_PASSWORD";

}  // namespace secrets
