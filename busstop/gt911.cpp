#include "gt911.h"

#include <Arduino.h>
#include <Wire.h>

#include "config.h"

namespace {

constexpr uint8_t kAddress = 0x5D;
constexpr uint16_t kStatusRegister = 0x814E;  // bit 7 = new data, bits 0-3 = number of fingers

bool touched = false;

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
    writeStatus(0);
  }
  return touched;
}
