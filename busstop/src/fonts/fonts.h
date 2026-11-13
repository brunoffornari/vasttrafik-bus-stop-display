// ============================================================================
// fonts.h - Montserrat with Swedish letters (å ä ö Å Ä Ö) and °
// ============================================================================
// Generated from lvgl/scripts/built_in_font/Montserrat-Medium.ttf with lv_font_conv:
//   npx lv_font_conv --font Montserrat-Medium.ttf --size <16|24|32> --bpp 4 --no-compress \
//     --range 0x20-0x7F,0xB0,0xC4,0xC5,0xD6,0xE4,0xE5,0xF6 --format lvgl \
//     --lv-font-name busstop_font_<size> --lv-include lvgl.h -o busstop_font_<size>.c
#pragma once
#include <lvgl.h>

LV_FONT_DECLARE(busstop_font_16)
LV_FONT_DECLARE(busstop_font_24)
LV_FONT_DECLARE(busstop_font_32)
// Big digits only, for the clock and weather pages:
//   --size 128 --range 0x30-0x3A  (busstop_font_clock: 0-9 and :)
//   --size 64  --range 0x2D,0x30-0x39,0xB0,0x43  (busstop_font_temp: - 0-9 ° C)
LV_FONT_DECLARE(busstop_font_clock)
LV_FONT_DECLARE(busstop_font_temp)
