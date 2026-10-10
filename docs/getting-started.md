# Getting started: from a new board to departures on the screen

Takes about 30-40 minutes the first time. Works on **Windows, macOS and Linux**. The author developed and
tested it on macOS; the Windows and Linux notes follow the standard Arduino tools, so tell us (open an
issue) if anything is different on your computer.

You need: the display board and a USB data cable ([hardware](hardware.md)), a computer, a phone, and your
free Västtrafik key ([how to get it](vasttrafik-api-key.md)). **Get the key first.**

## 1. Get this project

Click **Code > Download ZIP** on GitHub and unzip it, or run `git clone`. The Arduino sketch is the folder
**`busstop`**. Its name must stay `busstop` (Arduino requires the folder name to match `busstop.ino`).

## 2. Install Arduino IDE

Download **Arduino IDE 2.x** from [arduino.cc/en/software](https://www.arduino.cc/en/software) and install it.

- **Windows:** if the board never shows up as a port, install the USB driver for its chip (**CH340** or
  **CP210x**, depending on the board) and try another cable.
- **Linux:** to use the USB port you must be in the `dialout` group:
  `sudo usermod -a -G dialout $USER`, then log out and in again.
- **macOS:** nothing extra is usually needed.

## 3. Add ESP32 support

1. Open **File > Preferences** (macOS: **Arduino IDE > Settings**).
2. In *Additional boards manager URLs* add:
   `https://espressif.github.io/arduino-esp32/package_esp32_index.json`
3. Open **Tools > Board > Boards Manager**, search **esp32**, and install **esp32 by Espressif Systems**.
   Tested with **3.3.2**.

## 4. Install the libraries

Open **Tools > Manage Libraries** and install these (pick the version in the drop-down):

| Library | Version tested | Careful |
| --- | --- | --- |
| **lvgl** (by kisvegabor) | **8.3.11** | Must be **8.x**. Version 9 does not work. |
| **TFT_eSPI** (by Bodmer) | 2.5.43 | |
| **ArduinoJson** (by Benoit Blanchon) | 7.4.2 | |

## 5. Copy the two library settings files

These tell the libraries about this board's screen. Find your Arduino **libraries** folder:

| System | Usual location |
| --- | --- |
| Windows | `Documents\Arduino\libraries` |
| macOS | `~/Documents/Arduino/libraries` |
| Linux | `~/Arduino/libraries` |

(Check **File > Preferences > Sketchbook location**; `libraries` is inside it.)

1. Copy [`docs/library-config/TFT_eSPI_User_Setup.h`](library-config/TFT_eSPI_User_Setup.h) into
   `libraries/TFT_eSPI/` and name it **`User_Setup.h`** (replace the existing file).
2. Copy [`docs/library-config/lv_conf.h`](library-config/lv_conf.h) into `libraries/` itself, **next to** the
   `lvgl` folder (not inside it).

Updating TFT_eSPI later can overwrite `User_Setup.h`. Copy it again if the screen goes blank after an update.

## 6. Open the sketch and upload

1. Open `busstop/busstop.ino` in Arduino IDE.
2. Connect the board with the USB cable.
3. **Tools** menu:
   - **Board:** *esp32 > ESP32 Dev Module*
   - **Partition Scheme:** *Minimal SPIFFS (1.9MB APP with OTA/190KB SPIFFS)*
   - **Upload Speed:** *230400* (use this if uploads fail at the default speed)
   - **Port:** the one that appeared when you connected the board
4. Click **Upload** (the arrow). The first build takes a few minutes.

You do **not** need a `secrets.h` file. (It is an optional shortcut for developers, see
[`busstop/secrets.example.h`](../busstop/secrets.example.h).)

## 7. Set it up with your phone

The screen shows **Setup**.

1. On your phone join the WiFi **`BusStop-Setup`** (no password). The setup page opens by itself. If not, open
   a browser and go to **192.168.4.1**.
2. Choose your **home WiFi** and type its password.
3. Enter your **Västtrafik Client ID and Client secret**.
4. Optional: choose an **update password**. You need it to update the display over WiFi later. Without one,
   WiFi updates stay off.
5. Press **Connect**. Wait until it says it is connected.
6. **Search for your stop** by name and tap it, then press **Save and start**.

The display restarts and shows your departures.

## Next

- Update the code over WiFi: [updating.md](updating.md)
- Something wrong? [troubleshooting.md](troubleshooting.md)
