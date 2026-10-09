# Bus Stop Display

A small ESP32 screen for the hallway wall. It shows the next departures from a
Västtrafik stop, the clock, the date and the outside temperature.

- Live departures with line colors, delays included, cancelled buses marked
- Clock with blinking colon, correct summer/winter time (EU rule), re-synced daily
- Temperature from [Open-Meteo](https://open-meteo.com) (no key needed)
- Brightness follows the time of day with smooth fades. A tap wakes the screen.
- Setup from your phone: pick WiFi and bus stop on a web page, nothing is hard-coded
- If the internet goes away it keeps counting down and shows "No connection"
- Updates over WiFi, with a small Mac app or one command

## Hardware

- ESP32-3248S035C: 3.5" 480x320 screen (ST7796) with GT911 capacitive touch
- Board in Arduino IDE: **ESP32 Dev Module**, Partition Scheme **Minimal SPIFFS (1.9MB APP with OTA)**

## First time

1. Install Arduino IDE with the ESP32 board package, and the libraries
   **lvgl 8.3.x**, **TFT_eSPI** and **ArduinoJson 7**.
2. Copy the files in [`busstop/docs/library-config`](busstop/docs/library-config) into your Arduino
   libraries: `TFT_eSPI_User_Setup.h` replaces `TFT_eSPI/User_Setup.h`, and `lv_conf.h` goes next to the
   `lvgl` folder (`Documents/Arduino/libraries/lv_conf.h`). They hold the pin layout of this board.
3. Copy `busstop/secrets.example.h` to `busstop/secrets.h` and fill in your
   [Västtrafik API](https://developer.vasttrafik.se) credentials and an update password.
4. Open `busstop/busstop.ino`, select the board and port, and upload by cable.
   (The sketch folder must keep the name `busstop`.)
5. On first start the screen says **Setup**. Join the WiFi `BusStop-Setup` with your phone, pick your
   WiFi and your bus stop, and save.

## Using it

| Do this | To get this |
| --- | --- |
| Tap the screen | Full brightness for 30 seconds |
| Hold the screen for 4 seconds | Open the setup page again |
| Press BOOT within 2 seconds after power-on | Open the setup page |

If the WiFi is gone for 15 minutes, the setup page opens by itself.

## Updating over WiFi

- **Mac app:** build it once with `busstop/tools/BusStopUpdater/build.sh`, then open
  *Bus Stop Updater*, choose the `.ino`, enter the display's IP address (or press *Find*) and press
  *Update display*. It reads the update password from `secrets.h`.
- **Command line:** `busstop/update.sh` (WiFi) or `busstop/update.sh cable` (USB).

Both need Arduino IDE installed (they use its compiler).

## Code map

| File | What it does |
| --- | --- |
| `busstop.ino` | `setup()` and `loop()` |
| `config.h` | All settings: intervals, brightness, touch, timezone |
| `secrets.h` | API and update passwords (not in git) |
| `ui.*`, `src/fonts` | Screen layout and the Montserrat fonts with å ä ö |
| `vasttrafik.*`, `weather.*` | The two web APIs |
| `time_service.*`, `iso_time.h` | NTP, summer/winter time, time math |
| `backlight.*`, `gt911.*`, `touch.*` | Brightness, touch chip, taps and long presses |
| `setup_portal.*`, `setup_page.h`, `settings.*` | Phone setup page and saved settings |
| `wifi_manager.*`, `ota.*` | WiFi and updates over WiFi |
| `tools/BusStopUpdater` | The Mac update app (SwiftUI) |
