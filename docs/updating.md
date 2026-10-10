# Compiling and updating

Pick the way that suits you. **A** always works. **B-D** update over WiFi, without the USB cable (you must have
chosen an **update password** on the setup page; without one, WiFi updates are off).

The author tested these on **macOS**. The Windows and Linux ways use the standard Arduino tools, but have not
been tested by the author. If one fails for you, open an issue with the error text.

Where Arduino keeps its files ("Arduino15"):

| System | Folder |
| --- | --- |
| Windows | `%LOCALAPPDATA%\Arduino15` |
| macOS | `~/Library/Arduino15` |
| Linux | `~/.arduino15` |

## A. USB cable (Windows, macOS, Linux)

Arduino IDE: connect the board, choose the board settings from [getting started](getting-started.md), press
**Upload**.

## B. Over WiFi from Arduino IDE (Windows, macOS, Linux)

1. Make the computer and the display use the same WiFi.
2. **Tools > Port** and look under **Network ports** for **busstop**.
3. Press **Upload** and type your update password when asked.

If *busstop* does not appear (Windows without Bonjour, some routers), use C or D.

## C. Command line with arduino-cli (Windows, macOS, Linux)

Install [arduino-cli](https://arduino.github.io/arduino-cli/) (or use the one inside Arduino IDE).

```bash
# compile (run in the folder that contains the "busstop" folder)
arduino-cli compile --fqbn esp32:esp32:esp32:PartitionScheme=min_spiffs --output-dir build busstop

# send to the display (replace the IP address and the password)
python3 "<Arduino15>/packages/esp32/hardware/esp32/<version>/tools/espota.py" \
  -i 192.168.1.89 -p 3232 -a YOUR_PASSWORD -f build/busstop.ino.bin -r
```

On Windows use `python` instead of `python3`, and write the path with `\`. `espota.py` comes with the ESP32
board package (see the table above for the folder).

## D. macOS: the one-click app or script

- **Bus Stop Updater** (Apple Silicon Macs): unzip
  [`Bus-Stop-Updater.zip`](../busstop/tools/BusStopUpdater/Bus-Stop-Updater.zip), drag the app to
  Applications (first time: right-click > Open, because the app is not notarized). Choose the `.ino` file, type
  the display's IP address (or press **Find**), open **Advanced** and enter your update password, press
  **Update display**. You can build the app yourself with
  [`busstop/tools/BusStopUpdater/build.sh`](../busstop/tools/BusStopUpdater/build.sh).
- **Script:** `export BUSSTOP_OTA_PASSWORD=YOUR_PASSWORD` then `busstop/update.sh`
  (`busstop/update.sh cable` for USB).

Both need Arduino IDE installed, because they use its compiler.

## Finding the display's IP address

Your router's list of connected devices (look for **busstop**), or run `ping busstop.local` in a terminal and
read the address it prints.
