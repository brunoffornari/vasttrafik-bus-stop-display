# Hardware

## What you need

| Part | Notes |
| --- | --- |
| **3.5" ESP32 display board: 480x320 ST7796 screen, GT911 capacitive touch** | See "The board this was built on" below. Boards of this type are often called **ESP32-3248S035C** (the **C** means *capacitive* touch). The **R** version has *resistive* touch (XPT2046) and is **not** supported. |
| USB cable that carries **data** | Charge-only cables will not show up as a serial port. |
| 5 V USB power supply | Any phone charger works. Power the board through its USB connector. Soldering a power cable to VCC and GND is possible but not needed, see [`case/`](../case#power). |
| 3D-printed case and **4 x M2.5 x 6 mm screws** | Files, print settings and parts list in [`case/`](../case). Optional. |

## The board this was built on

Built and tested on a 3.5-inch board from AliExpress: the **3.5 inch** option of the listing
["2.4/2.8/3.5 inch LCD TFT Module ESP32 Arduino LVGL"](https://www.aliexpress.com/item/1005009795213070.html)
by the seller *AITEWIN ROBOT* (bought in 2026, about 250 kr). Be careful:

- The listing **does not state the exact model or the touch chip**. The board that arrived has an ST7796 screen
  and a **GT911** touch chip (I²C address `0x5D`), with the pins in the table below.
- The same listing sells 2.4" and 2.8" boards, which do **not** work with this project.
- One reviewer of that listing writes that almost no firmware works on the 3.5-inch version. Sellers also
  change parts without notice, so **compare your board with the pin table** before blaming the code.

**Buying in Sweden:** as of October 2026 no Swedish shop that could be found sold this board. Opencircuit sells a
different 3.5-inch board (LILYGO LILY Pi) with another touch chip (FT6x36), which would need code changes. If
you find a Swedish shop for this board, please open an issue or a pull request and add it here.

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
