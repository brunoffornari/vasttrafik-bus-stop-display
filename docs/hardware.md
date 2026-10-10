# Hardware

## What you need

| Part | Notes |
| --- | --- |
| **3.5" ESP32 display board with a 480x320 ST7796 screen and GT911 capacitive touch** | Commonly sold as **ESP32-3248S035C**. The **C** is the *capacitive* touch version, which this project supports. The **R** version has *resistive* touch (XPT2046) and is **not** supported. If you are unsure, check that the listing says GT911 or "capacitive". |
| USB cable that carries **data** | Charge-only cables will not show up as a serial port. |
| 5 V USB power supply | Any phone charger works. |
| 3D-printed case | Files and print notes in [`case/`](../case). Optional. |

## What the board has (as used by this project)

| Function | Pins |
| --- | --- |
| Screen (SPI, ST7796) | MISO 12, MOSI 13, SCLK 14, CS 15, DC 2, backlight **27** |
| Touch (GT911, I²C, address 0x5D) | SDA 33, SCL 32, INT 21, RST 25 |
| RGB LED (active low, kept off) | 4, 16, 17 |
| BOOT button | 0 |

The screen pins live in [`library-config/TFT_eSPI_User_Setup.h`](library-config/TFT_eSPI_User_Setup.h) and the
touch and backlight pins in [`busstop/config.h`](../busstop/config.h). If your board uses different pins,
those are the two places to change.

## Notes

- The board needs the **ESP32 Dev Module** setting in Arduino IDE.
- Windows may need a USB driver for the board's USB-serial chip (CH340 or CP210x, depending on the board)
  before a port appears. See [getting started](getting-started.md).
