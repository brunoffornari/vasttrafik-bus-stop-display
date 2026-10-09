#!/bin/bash
# Builds the sketch and uploads it to the bus stop display.
#   ./update.sh          over WiFi (device must be on the same network)
#   ./update.sh cable    over USB (needed the very first time or if WiFi updates fail)
set -e
cd "$(dirname "$0")"

CLI="/Applications/Arduino IDE.app/Contents/Resources/app/lib/backend/resources/arduino-cli"
FQBN="esp32:esp32:esp32:PartitionScheme=min_spiffs,UploadSpeed=230400"
USB_PORT="/dev/cu.usbserial-110"
ESPOTA="$HOME/Library/Arduino15/packages/esp32/hardware/esp32/3.3.2/tools/espota.py"
BUILD_DIR="$PWD/build"

"$CLI" compile --fqbn "$FQBN" --output-dir "$BUILD_DIR" .

if [ "$1" = "cable" ]; then
  "$CLI" upload -p "$USB_PORT" --fqbn "$FQBN" .
else
  IP=$(ping -c1 -t3 busstop.local | sed -n 's/^PING [^(]*(\([0-9.]*\)).*/\1/p')
  if [ -z "$IP" ]; then echo "Cannot find busstop.local on the network"; exit 1; fi
  PASSWORD=$(grep -o 'kOtaPassword = "[^"]*"' secrets.h | cut -d'"' -f2)
  echo "Uploading to $IP over WiFi..."
  python3 "$ESPOTA" -i "$IP" -p 3232 -a "$PASSWORD" -f "$BUILD_DIR/busstop.ino.bin" -r 2>&1 | tr '\r' '\n' | grep -vE "^\s*$" | tail -4
fi
