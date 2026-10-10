# Troubleshooting

## Setup page

- **The page does not open:** open a browser and go to `192.168.4.1`. Turn off mobile data on your phone, and
  make sure it is still joined to `BusStop-Setup`.
- **"Västtrafik did not accept the key":** see [the key guide](vasttrafik-api-key.md). Most often the app is not
  subscribed to **Planera Resa v4**, or a value was copied with a space.
- **"Could not connect" to WiFi:** check the password. The display only uses 2.4 GHz WiFi, not 5 GHz.
- **The setup page closes by itself:** it restarts after 5 minutes without use. Start again.

## Opening the setup page again

Hold the screen for **4 seconds**, or press the **BOOT** button within 2 seconds after power-on. It also opens
by itself if WiFi has been gone for 15 minutes.

## The screen

- **Blank or white screen after uploading:** the library settings files are missing or were overwritten. Redo
  step 5 of [getting started](getting-started.md).
- **Colors look wrong:** same cause, check `User_Setup.h`.
- **Shows "Loading..." for a long time:** wait a minute. If it stays, open the Serial Monitor (115200 baud) and
  look for errors. Check the key and the stop.
- **"No connection" in orange:** the display lost internet. It keeps counting down the last departures, and the
  minutes turn grey until data comes back.
- **Temperature shows `--`:** the weather service did not answer. It retries by itself.

## Compiling and uploading

- **`lvgl.h` or other errors:** check the library versions in [getting started](getting-started.md). lvgl must be
  **8.x** and ArduinoJson **7.x**.
- **"Sketch too big":** choose the **Minimal SPIFFS** partition scheme.
- **No port / port busy:** use a **data** USB cable, close any open Serial Monitor, install the USB driver
  (Windows) or join the `dialout` group (Linux).
- **Upload stops or "chip stopped responding":** set **Upload Speed** to 230400 or lower.
- **WiFi update says "Authentication Failed":** wrong update password.

## Time and language

- The clock follows Swedish time, including summer/winter time. It re-syncs with the internet every day.
- Swedish letters (å ä ö) work. Other languages' letters are not included in the font.
