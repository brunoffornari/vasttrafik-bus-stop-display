#include "gt911.h"

#include <Arduino.h>
#include <Wire.h>

#include "config.h"

namespace {

constexpr uint8_t kAddress = 0x5D;
constexpr uint16_t kStatusRegister = 0x814E;  // bit 7 = new data, bits 0-3 = number of fingers

bool touched = false;
int lastX = 0, lastY = 0;
bool haveFix = false;
constexpr uint16_t kPointRegister = 0x8150;  // first touch point: x lo, x hi, y lo, y hi

bool writeStatus(uint8_t value) {
  Wire.beginTransmission(kAddress);
  Wire.write(kStatusRegister >> 8);
  Wire.write(kStatusRegister & 0xFF);
  Wire.write(value);
  return Wire.endTransmission() == 0;
}

bool readStatus(uint8_t &status) {
  Wire.beginTransmission(kAddress);
  Wire.write(kStatusRegister >> 8);
  Wire.write(kStatusRegister & 0xFF);
  if (Wire.endTransmission(false) != 0) return false;
  if (Wire.requestFrom((int)kAddress, 1) != 1) return false;
  status = Wire.read();
  return true;
}

void readPoint() {
  Wire.beginTransmission(kAddress);
  Wire.write(kPointRegister >> 8);
  Wire.write(kPointRegister & 0xFF);
  if (Wire.endTransmission(false) != 0) return;
  if (Wire.requestFrom((int)kAddress, 4) != 4) return;
  uint8_t b[4];
  for (int i = 0; i < 4; i++) b[i] = Wire.read();
  lastX = b[0] | (b[1] << 8);
  lastY = b[2] | (b[3] << 8);
  haveFix = true;
}

}  // namespace

void gt911Begin() {
  // Reset with INT low so the chip picks I2C address 0x5D
  pinMode(config::kTouchResetPin, OUTPUT);
  pinMode(config::kTouchIntPin, OUTPUT);
  digitalWrite(config::kTouchIntPin, LOW);
  digitalWrite(config::kTouchResetPin, LOW);
  delay(10);
  digitalWrite(config::kTouchResetPin, HIGH);
  delay(10);
  pinMode(config::kTouchIntPin, INPUT);
  delay(60);

  Wire.begin(config::kTouchSdaPin, config::kTouchSclPin);
  Wire.setClock(400000);
}

bool gt911Touched() {
  uint8_t status;
  if (!readStatus(status)) return touched;  // keep the last known state on I2C errors
  if (status & 0x80) {                      // new data: update, then ask for the next frame
    touched = (status & 0x0F) > 0;
    if (touched) readPoint();
    writeStatus(0);
  }
  return touched;
}

bool gt911Position(int &x, int &y) {
  x = lastX;
  y = lastY;
  return haveFix;
}
