#include "utils.h"

String base64Encode(const String &input) {
  static const char *kChars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

  String output;
  int val = 0;    // bit accumulator
  int valb = -6;  // bits left in accumulator, minus 6

  for (size_t i = 0; i < input.length(); i++) {
    val = (val << 8) + (unsigned char)input[i];
    valb += 8;
    while (valb >= 0) {
      output += kChars[(val >> valb) & 0x3F];
      valb -= 6;
    }
  }
  if (valb > -6) output += kChars[((val << 8) >> (valb + 8)) & 0x3F];
  while (output.length() % 4) output += '=';

  return output;
}

String urlEncode(const String &text) {
  static const char *kHex = "0123456789ABCDEF";
  String out;
  for (size_t i = 0; i < text.length(); i++) {
    unsigned char c = text[i];
    if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
      out += (char)c;
    } else {
      out += '%';
      out += kHex[c >> 4];
      out += kHex[c & 0x0F];
    }
  }
  return out;
}

String cutAt(const String &text, char separator) {
  int index = text.indexOf(separator);
  return index > 0 ? text.substring(0, index) : text;
}
