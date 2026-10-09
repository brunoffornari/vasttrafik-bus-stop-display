// ============================================================================
// utils.h - Small helpers
// ============================================================================
#pragma once
#include <Arduino.h>

// Base64-encodes a string (used for the OAuth2 Basic auth header).
String base64Encode(const String &input);

// Percent-encodes text for use in a URL query.
String urlEncode(const String &text);

// Returns the text before the first occurrence of `separator` (or all of it).
String cutAt(const String &text, char separator);
