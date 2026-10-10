# Third-party notices

This project is **not affiliated with or endorsed by Västtrafik**. "Västtrafik" belongs to its owner and
is only used here to say which service the display works with.

The project's own code is under the [MIT License](LICENSE); the 3D case is under
[CC BY-SA 4.0](case/LICENSE). It also uses the following, which stay under their own licenses:

| What | Used for | License |
| --- | --- | --- |
| [Montserrat](https://github.com/JulietaUla/Montserrat) font | The screen fonts in `busstop/src/fonts`, generated from the font file that ships with LVGL, with å ä ö added | SIL Open Font License 1.1, see [`busstop/src/fonts/OFL.txt`](busstop/src/fonts/OFL.txt) |
| [LVGL](https://lvgl.io) 8.3 | Graphics on the screen. `docs/library-config/lv_conf.h` is the project's configuration for it | MIT |
| [TFT_eSPI](https://github.com/Bodmer/TFT_eSPI) | Screen driver. `docs/library-config/TFT_eSPI_User_Setup.h` is a settings file for it | BSD / MIT, see the library's `license.txt` |
| [ArduinoJson](https://arduinojson.org) 7 | Reading web answers | MIT |
| [Arduino-ESP32](https://github.com/espressif/arduino-esp32) | ESP32 support (WiFi, web server, updates) | LGPL 2.1 |
| [Open-Meteo](https://open-meteo.com) | Temperature. Free for non-commercial use, and they ask for credit: weather data by Open-Meteo.com (CC BY 4.0) | see their terms |
| [Västtrafik Planera Resa API](https://developer.vasttrafik.se) | Departures and stop search. Each user needs their own free key and must follow Västtrafik's terms | Västtrafik's terms |

The libraries are **not** included in this repository. You install them yourself (see
[docs/getting-started.md](docs/getting-started.md)).
