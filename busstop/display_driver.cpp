#include "display_driver.h"

#include <Arduino.h>
#include <TFT_eSPI.h>
#include <lvgl.h>

#include "config.h"

namespace {

constexpr uint32_t kBufferPixels = config::kScreenWidth * config::kDrawBufferLines;

TFT_eSPI tft = TFT_eSPI();

lv_disp_draw_buf_t drawBuf;
lv_color_t *psramBuf = nullptr;
lv_color_t fallbackBuf[kBufferPixels];  // used if PSRAM allocation fails

// Called by LVGL when an area must be sent to the screen.
void flushToDisplay(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *colors) {
  uint32_t w = area->x2 - area->x1 + 1;
  uint32_t h = area->y2 - area->y1 + 1;

  tft.startWrite();
  tft.setAddrWindow(area->x1, area->y1, w, h);
  tft.pushColors((uint16_t *)&colors->full, w * h, true);
  tft.endWrite();

  lv_disp_flush_ready(disp);
}

}  // namespace

void displayInit() {
  tft.begin();
  tft.setRotation(3);  // landscape, upside-down
  tft.fillScreen(TFT_BLACK);

  lv_init();
  psramBuf = (lv_color_t *)ps_malloc(sizeof(lv_color_t) * kBufferPixels);
  if (psramBuf) {
    Serial.println("Using PSRAM for LVGL buffer");
    lv_disp_draw_buf_init(&drawBuf, psramBuf, NULL, kBufferPixels);
  } else {
    Serial.println("PSRAM allocation failed, using fallback");
    lv_disp_draw_buf_init(&drawBuf, fallbackBuf, NULL, kBufferPixels);
  }

  static lv_disp_drv_t driver;
  lv_disp_drv_init(&driver);
  driver.hor_res = config::kScreenWidth;
  driver.ver_res = config::kScreenHeight;
  driver.flush_cb = flushToDisplay;
  driver.draw_buf = &drawBuf;
  lv_disp_drv_register(&driver);
}
