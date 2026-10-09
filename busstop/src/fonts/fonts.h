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
