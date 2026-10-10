# 3D-printed case

A case for the 3.5" ESP32 display board described in [docs/hardware.md](../docs/hardware.md), made to sit on a shelf or hallway table.

![The printed case with the display, on a shelf](../docs/images/display-on-shelf.jpg)

## Files

| File | What it is |
| --- | --- |
| [`stl/busstop_front.stl`](stl/busstop_front.stl) | The front part: holds the screen board. About 110 x 64 x 50 mm. |
| [`stl/busstop_back.stl`](stl/busstop_back.stl) | The back plate. About 108 x 57 x 5 mm. |
| [`source/busstop.f3d`](source/busstop.f3d) | The editable design, for [Autodesk Fusion](https://www.autodesk.com/products/fusion-360) (free for personal use). Change it and export new STL files. |

Print the two `.stl` files. If you want to change the design, open the `.f3d` file in Fusion. Designed by
Bruno Fornari.

<!-- TODO (maintainer): add a STEP export of the design (Fusion: File > Export > STEP) so people with other CAD
programs can edit it too. -->

## Print settings

| Setting | Value |
| --- | --- |
| Printer | Bambu Lab A1 mini |
| Material | PLA |
| Layer height | 0.2 mm |
| Other settings | The normal (default) profile, nothing special |
| Supports | Not needed |

Print the front and the back as two separate parts.

<!-- TODO (optional, maintainer): add print time, infill and the orientation of each part, if you want. -->

## What you need

- The two printed parts (front and back)
- The display board ([docs/hardware.md](../docs/hardware.md))
- **4 x M2.5 x 6 mm screws**

## Assembly

1. Tip: upload the firmware and run the setup first, so you know the board works before you close the case.
2. Put the display board in the front part.
3. Put the back plate on and screw it to the front part with the **4 x M2.5 x 6 mm screws**. The screws go in
   **from the back**, into the front part.

<!-- TODO (optional, maintainer): add a photo of the open case, and say how the board is held in place. -->

## Power

**The easy way (recommended): use the board's USB connector** with a normal USB cable and a 5 V phone charger.
No tools or soldering needed.

*Optional, for advanced users:* the author soldered a regular 5 V power cable to the board's **VCC** and **GND**
pads instead of using USB. That needs soldering skills. Use **5 V only**, take care of the polarity (plus to VCC,
minus to GND), and to be safe do not connect USB and the soldered cable at the same time. You do the soldering
at your own risk.

## License

This case is released under the **Creative Commons Attribution-ShareAlike 4.0 International** license
([CC BY-SA 4.0](LICENSE)). You may share and change it, also commercially, if you give credit and share your
changes under the same license.
