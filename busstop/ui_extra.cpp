#include "ui_extra.h"

#include <math.h>
#include <time.h>

#include "config.h"
#include "src/fonts/fonts.h"

namespace {

constexpr uint32_t kColorCard = 0x13171D;
constexpr uint32_t kColorDivider = 0x232B36;
constexpr uint32_t kColorTextPrimary = 0xF2F5F9;
constexpr uint32_t kColorTextSecondary = 0x8A94A6;
constexpr uint32_t kColorAccent = 0x5EEAD4;
constexpr uint32_t kColorSun = 0xFFC53D;
constexpr uint32_t kColorCloud = 0xB8C2CF;
constexpr uint32_t kColorDarkCloud = 0x8693A5;
constexpr uint32_t kColorRain = 0x5AB0FF;
constexpr uint32_t kColorSecondHand = 0xFF8A3D;

#define FONT_SMALL (&busstop_font_16)
#define FONT_MEDIUM (&busstop_font_24)
#define FONT_LARGE (&busstop_font_32)

enum IconKind { kSun, kPartCloud, kCloud, kFog, kRain, kSnow, kThunder };

IconKind iconFor(int code) {
  if (code == 0) return kSun;
  if (code <= 2) return kPartCloud;
  if (code == 3) return kCloud;
  if (code == 45 || code == 48) return kFog;
  if ((code >= 71 && code <= 77) || code == 85 || code == 86) return kSnow;
  if (code >= 95) return kThunder;
  return kRain;  // drizzle, rain, showers
}

const char *describe(int code) {
  switch (code) {
    case 0: return "Clear";
    case 1: return "Mostly clear";
    case 2: return "Partly cloudy";
    case 3: return "Overcast";
    case 45: case 48: return "Fog";
    case 51: case 53: case 55: case 56: case 57: return "Drizzle";
    case 61: case 63: case 65: case 66: case 67: return "Rain";
    case 71: case 73: case 75: case 77: return "Snow";
    case 80: case 81: case 82: return "Showers";
    case 85: case 86: return "Snow showers";
    case 95: case 96: case 99: return "Thunderstorm";
    default: return "";
  }
}

// --- Drawing helpers (LVGL draw API) ------------------------------------------
void fillCircle(lv_draw_ctx_t *ctx, float cx, float cy, float r, uint32_t color) {
  lv_draw_rect_dsc_t d;
  lv_draw_rect_dsc_init(&d);
  d.bg_color = lv_color_hex(color);
  d.radius = LV_RADIUS_CIRCLE;
  d.border_width = 0;
  lv_area_t a = {(lv_coord_t)lroundf(cx - r), (lv_coord_t)lroundf(cy - r), (lv_coord_t)lroundf(cx + r),
                 (lv_coord_t)lroundf(cy + r)};
  lv_draw_rect(ctx, &d, &a);
}

void fillRound(lv_draw_ctx_t *ctx, float x1, float y1, float x2, float y2, float radius, uint32_t color) {
  lv_draw_rect_dsc_t d;
  lv_draw_rect_dsc_init(&d);
  d.bg_color = lv_color_hex(color);
  d.radius = (lv_coord_t)lroundf(radius);
  d.border_width = 0;
  lv_area_t a = {(lv_coord_t)lroundf(x1), (lv_coord_t)lroundf(y1), (lv_coord_t)lroundf(x2), (lv_coord_t)lroundf(y2)};
  lv_draw_rect(ctx, &d, &a);
}

void strokeLine(lv_draw_ctx_t *ctx, float x1, float y1, float x2, float y2, int width, uint32_t color) {
  lv_draw_line_dsc_t d;
  lv_draw_line_dsc_init(&d);
  d.color = lv_color_hex(color);
  d.width = width;
  d.round_start = 1;
  d.round_end = 1;
  lv_point_t p1 = {(lv_coord_t)lroundf(x1), (lv_coord_t)lroundf(y1)};
  lv_point_t p2 = {(lv_coord_t)lroundf(x2), (lv_coord_t)lroundf(y2)};
  lv_draw_line(ctx, &d, &p1, &p2);
}

// A cloud made of three circles and a base. (ox, oy) is the top-left of a 100x100 unit box scaled by s.
void drawCloud(lv_draw_ctx_t *ctx, float ox, float oy, float s, uint32_t color) {
  fillCircle(ctx, ox + 32 * s, oy + 58 * s, 17 * s, color);
  fillCircle(ctx, ox + 52 * s, oy + 44 * s, 23 * s, color);
  fillCircle(ctx, ox + 73 * s, oy + 57 * s, 17 * s, color);
  fillRound(ctx, ox + 15 * s, oy + 58 * s, ox + 90 * s, oy + 76 * s, 9 * s, color);
}

void drawSun(lv_draw_ctx_t *ctx, float cx, float cy, float r, float s) {
  fillCircle(ctx, cx, cy, r, kColorSun);
  int w = (int)fmaxf(2, 4 * s);
  for (int i = 0; i < 8; i++) {
    float a = i * (float)M_PI / 4;
    strokeLine(ctx, cx + sinf(a) * r * 1.45f, cy - cosf(a) * r * 1.45f, cx + sinf(a) * r * 1.85f,
               cy - cosf(a) * r * 1.85f, w, kColorSun);
  }
}

void drawIcon(lv_event_t *e) {
  lv_obj_t *obj = lv_event_get_target(e);
  lv_draw_ctx_t *ctx = lv_event_get_draw_ctx(e);
  lv_area_t area;
  lv_obj_get_coords(obj, &area);
  float size = (float)lv_area_get_width(&area);
  float s = size / 100.0f;
  float x = area.x1, y = area.y1;
  IconKind kind = (IconKind)(intptr_t)lv_obj_get_user_data(obj);

  switch (kind) {
    case kSun:
      drawSun(ctx, x + 50 * s, y + 50 * s, 20 * s, s);
      break;
    case kPartCloud:
      drawSun(ctx, x + 34 * s, y + 34 * s, 14 * s, s);
      drawCloud(ctx, x + 14 * s, y + 8 * s, 0.85f, kColorCloud);
      break;
    case kCloud:
      drawCloud(ctx, x, y - 6 * s, s, kColorCloud);
      break;
    case kFog:
      drawCloud(ctx, x, y - 14 * s, s, kColorCloud);
      for (int i = 0; i < 3; i++) {
        strokeLine(ctx, x + (20 + i * 6) * s, y + (68 + i * 11) * s, x + (80 - i * 6) * s, y + (68 + i * 11) * s,
                   (int)fmaxf(2, 5 * s), kColorDarkCloud);
      }
      break;
    case kRain:
      drawCloud(ctx, x, y - 14 * s, s, kColorDarkCloud);
      for (int i = 0; i < 3; i++) {
        float dx = (30 + i * 20) * s;
        strokeLine(ctx, x + dx + 4 * s, y + 68 * s, x + dx - 4 * s, y + 88 * s, (int)fmaxf(2, 5 * s), kColorRain);
      }
      break;
    case kSnow:
      drawCloud(ctx, x, y - 14 * s, s, kColorCloud);
      for (int i = 0; i < 3; i++) {
        fillCircle(ctx, x + (30 + i * 20) * s, y + (76 + (i % 2) * 8) * s, 5 * s, 0xFFFFFF);
      }
      break;
    case kThunder:
      drawCloud(ctx, x, y - 14 * s, s, 0x6E7A8C);
      {
        int w = (int)fmaxf(2, 6 * s);
        strokeLine(ctx, x + 56 * s, y + 62 * s, x + 42 * s, y + 80 * s, w, kColorSun);
        strokeLine(ctx, x + 42 * s, y + 80 * s, x + 56 * s, y + 80 * s, w, kColorSun);
        strokeLine(ctx, x + 56 * s, y + 80 * s, x + 44 * s, y + 98 * s, w, kColorSun);
      }
      break;
  }
}

lv_obj_t *makeIcon(lv_obj_t *parent, int size) {
  lv_obj_t *icon = lv_obj_create(parent);
  lv_obj_set_size(icon, size, size);
  lv_obj_set_style_bg_opa(icon, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(icon, 0, 0);
  lv_obj_set_style_pad_all(icon, 0, 0);
  lv_obj_clear_flag(icon, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_add_event_cb(icon, drawIcon, LV_EVENT_DRAW_MAIN_END, nullptr);
  return icon;
}

void setIcon(lv_obj_t *icon, IconKind kind) {
  if ((IconKind)(intptr_t)lv_obj_get_user_data(icon) == kind) return;
  lv_obj_set_user_data(icon, (void *)(intptr_t)kind);
  lv_obj_invalidate(icon);
}

lv_obj_t *makeLabel(lv_obj_t *parent, const lv_font_t *font, uint32_t color, const char *text = "") {
  lv_obj_t *label = lv_label_create(parent);
  lv_obj_set_style_text_font(label, font, 0);
  lv_obj_set_style_text_color(label, lv_color_hex(color), 0);
  lv_label_set_text(label, text);
  return label;
}

void setTextIfChanged(lv_obj_t *label, const char *text) {
  if (strcmp(lv_label_get_text(label), text) != 0) lv_label_set_text(label, text);
}

// Weekday short name from "YYYY-MM-DD" (Sakamoto's method).
const char *weekdayName(const char *date) {
  static const char *const names[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
  static const int offsets[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
  int y = atoi(date), m = atoi(date + 5), d = atoi(date + 8);
  if (m < 1 || m > 12) return "";
  if (m < 3) y--;
  return names[(y + y / 4 - y / 100 + y / 400 + offsets[m - 1] + d) % 7];
}

// --- Widgets -----------------------------------------------------------------
constexpr int kColumnWidth = 90;
constexpr int kColumnsLeft = 15;

struct DayColumn {
  lv_obj_t *name, *icon, *high, *low, *rain;
};

DayColumn columns[config::kForecastDays];
lv_obj_t *nowIcon = nullptr, *nowTemp = nullptr, *nowText = nullptr;
lv_obj_t *digitalHours = nullptr, *digitalColon = nullptr, *digitalMinutes = nullptr, *digitalBox = nullptr;
lv_obj_t *digitalDate = nullptr, *digitalTemp = nullptr;
lv_obj_t *analogFace = nullptr;
int clockHour = 0, clockMinute = 0, clockSecond = 0;

constexpr int kFaceSize = 300;
constexpr float kFaceRadius = kFaceSize / 2.0f;

void hand(lv_draw_ctx_t *ctx, float cx, float cy, float angleDeg, float length, float tail, int width, uint32_t color) {
  float a = angleDeg * (float)M_PI / 180.0f;
  strokeLine(ctx, cx - sinf(a) * tail, cy + cosf(a) * tail, cx + sinf(a) * length, cy - cosf(a) * length, width, color);
}

void drawFace(lv_event_t *e) {
  lv_obj_t *obj = lv_event_get_target(e);
  lv_draw_ctx_t *ctx = lv_event_get_draw_ctx(e);
  lv_area_t area;
  lv_obj_get_coords(obj, &area);
  float cx = area.x1 + kFaceRadius, cy = area.y1 + kFaceRadius;

  for (int i = 0; i < 60; i++) {
    float a = i * (float)M_PI / 30;
    bool hour = i % 5 == 0;
    float outer = kFaceRadius - 8, inner = outer - (hour ? 14 : 5);
    strokeLine(ctx, cx + sinf(a) * inner, cy - cosf(a) * inner, cx + sinf(a) * outer, cy - cosf(a) * outer,
               hour ? 4 : 1, hour ? kColorTextPrimary : kColorTextSecondary);
  }

  float hourAngle = (clockHour % 12 + clockMinute / 60.0f) * 30.0f;
  float minuteAngle = (clockMinute + clockSecond / 60.0f) * 6.0f;
  hand(ctx, cx, cy, hourAngle, kFaceRadius * 0.5f, 10, 7, kColorTextPrimary);
  hand(ctx, cx, cy, minuteAngle, kFaceRadius * 0.78f, 12, 5, kColorTextPrimary);
  hand(ctx, cx, cy, clockSecond * 6.0f, kFaceRadius * 0.86f, 22, 2, kColorSecondHand);
  fillCircle(ctx, cx, cy, 8, kColorAccent);
  fillCircle(ctx, cx, cy, 3, 0x000000);
}

void createWeatherPage(lv_obj_t *page) {
  nowIcon = makeIcon(page, 110);
  lv_obj_set_pos(nowIcon, 26, 14);
  setIcon(nowIcon, kSun);

  nowTemp = makeLabel(page, &busstop_font_temp, kColorTextPrimary, "--\xC2\xB0""C");
  lv_obj_set_pos(nowTemp, 160, 16);

  nowText = makeLabel(page, FONT_MEDIUM, kColorTextSecondary, "");
  lv_obj_set_pos(nowText, 160, 92);

  lv_obj_t *divider = lv_obj_create(page);
  lv_obj_set_size(divider, config::kScreenWidth - 32, 1);
  lv_obj_set_pos(divider, 16, 138);
  lv_obj_set_style_bg_color(divider, lv_color_hex(kColorDivider), 0);
  lv_obj_set_style_border_width(divider, 0, 0);
  lv_obj_set_style_radius(divider, 0, 0);

  for (int i = 0; i < config::kForecastDays; i++) {
    DayColumn &c = columns[i];
    int x = kColumnsLeft + i * kColumnWidth;
    auto centered = [&](lv_obj_t *label, int y) {
      lv_obj_set_width(label, kColumnWidth);
      lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
      lv_obj_set_pos(label, x, y);
    };
    c.name = makeLabel(page, FONT_SMALL, kColorTextSecondary);
    centered(c.name, 150);
    c.icon = makeIcon(page, 56);
    lv_obj_set_pos(c.icon, x + (kColumnWidth - 56) / 2, 174);
    setIcon(c.icon, kCloud);
    lv_obj_add_flag(c.icon, LV_OBJ_FLAG_HIDDEN);
    c.high = makeLabel(page, FONT_MEDIUM, kColorTextPrimary);
    centered(c.high, 238);
    c.low = makeLabel(page, FONT_SMALL, kColorTextSecondary);
    centered(c.low, 270);
    c.rain = makeLabel(page, FONT_SMALL, kColorRain);
    centered(c.rain, 292);
  }
}

void placeDigital() {
  char hours[4], minutes[4];
  snprintf(hours, sizeof(hours), "%02d", clockHour);
  snprintf(minutes, sizeof(minutes), "%02d", clockMinute);
  setTextIfChanged(digitalHours, hours);
  setTextIfChanged(digitalMinutes, minutes);

  lv_point_t h, c, m;
  lv_txt_get_size(&h, hours, &busstop_font_clock, 0, 0, LV_COORD_MAX, LV_TEXT_FLAG_NONE);
  lv_txt_get_size(&c, ":", &busstop_font_clock, 0, 0, LV_COORD_MAX, LV_TEXT_FLAG_NONE);
  lv_txt_get_size(&m, minutes, &busstop_font_clock, 0, 0, LV_COORD_MAX, LV_TEXT_FLAG_NONE);
  int total = h.x + c.x + m.x;
  int x = (lv_obj_get_width(digitalBox) - total) / 2;
  lv_obj_set_width(digitalHours, h.x);
  lv_obj_set_pos(digitalHours, x, 0);
  lv_obj_set_pos(digitalColon, x + h.x, 0);
  lv_obj_set_width(digitalMinutes, m.x);
  lv_obj_set_pos(digitalMinutes, x + h.x + c.x, 0);
}

void createDigitalPage(lv_obj_t *page) {
  lv_point_t size;
  lv_txt_get_size(&size, "00", &busstop_font_clock, 0, 0, LV_COORD_MAX, LV_TEXT_FLAG_NONE);
  digitalBox = lv_obj_create(page);
  lv_obj_set_size(digitalBox, config::kScreenWidth, size.y);
  lv_obj_set_pos(digitalBox, 0, 4);
  lv_obj_set_style_bg_opa(digitalBox, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(digitalBox, 0, 0);
  lv_obj_set_style_pad_all(digitalBox, 0, 0);
  lv_obj_clear_flag(digitalBox, LV_OBJ_FLAG_SCROLLABLE);

  digitalHours = makeLabel(digitalBox, &busstop_font_clock, kColorTextPrimary, "00");
  digitalColon = makeLabel(digitalBox, &busstop_font_clock, kColorTextPrimary, ":");
  digitalMinutes = makeLabel(digitalBox, &busstop_font_clock, kColorTextPrimary, "00");
  placeDigital();

  digitalDate = makeLabel(page, FONT_LARGE, kColorTextSecondary);
  lv_obj_set_width(digitalDate, config::kScreenWidth);
  lv_obj_set_style_text_align(digitalDate, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_set_pos(digitalDate, 0, 214);

  digitalTemp = makeLabel(page, FONT_LARGE, kColorAccent, "--\xC2\xB0""C");
  lv_obj_set_width(digitalTemp, config::kScreenWidth);
  lv_obj_set_style_text_align(digitalTemp, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_set_pos(digitalTemp, 0, 264);
}

void createAnalogPage(lv_obj_t *page) {
  analogFace = lv_obj_create(page);
  lv_obj_set_size(analogFace, kFaceSize, kFaceSize);
  lv_obj_set_pos(analogFace, (config::kScreenWidth - kFaceSize) / 2, (config::kScreenHeight - kFaceSize) / 2);
  lv_obj_set_style_radius(analogFace, LV_RADIUS_CIRCLE, 0);
  lv_obj_set_style_bg_color(analogFace, lv_color_hex(kColorCard), 0);
  lv_obj_set_style_bg_opa(analogFace, LV_OPA_COVER, 0);
  lv_obj_set_style_border_width(analogFace, 3, 0);
  lv_obj_set_style_border_color(analogFace, lv_color_hex(kColorDivider), 0);
  lv_obj_set_style_pad_all(analogFace, 0, 0);
  lv_obj_clear_flag(analogFace, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_add_event_cb(analogFace, drawFace, LV_EVENT_DRAW_MAIN_END, nullptr);

  // Numbers 1-12
  for (int n = 1; n <= 12; n++) {
    float a = n * (float)M_PI / 6;
    char text[4];
    snprintf(text, sizeof(text), "%d", n);
    lv_obj_t *label = makeLabel(analogFace, FONT_MEDIUM, kColorTextPrimary, text);
    lv_point_t size;
    lv_txt_get_size(&size, text, FONT_MEDIUM, 0, 0, LV_COORD_MAX, LV_TEXT_FLAG_NONE);
    float r = kFaceRadius - 44;
    lv_obj_set_pos(label, (int)lroundf(kFaceRadius + sinf(a) * r - size.x / 2.0f) - 3,
                   (int)lroundf(kFaceRadius - cosf(a) * r - size.y / 2.0f) - 3);
  }
}

}  // namespace

void uiExtraCreate(lv_obj_t *weatherPage, lv_obj_t *digitalPage, lv_obj_t *analogPage) {
  createWeatherPage(weatherPage);
  createDigitalPage(digitalPage);
  createAnalogPage(analogPage);
}

void uiExtraReset() {
  nowIcon = nowTemp = nowText = nullptr;
  digitalHours = digitalColon = digitalMinutes = digitalBox = digitalDate = digitalTemp = nullptr;
  analogFace = nullptr;
  for (DayColumn &c : columns) c = DayColumn{};
}

void uiExtraSetWeather(const Weather *weather) {
  if (!nowTemp) return;
  char text[16];
  if (!weather) {
    setTextIfChanged(nowTemp, "--\xC2\xB0""C");
    setTextIfChanged(digitalTemp, "--\xC2\xB0""C");
    return;
  }

  snprintf(text, sizeof(text), "%d\xC2\xB0""C", weather->currentC);
  setTextIfChanged(nowTemp, text);
  setTextIfChanged(digitalTemp, text);
  setTextIfChanged(nowText, describe(weather->currentCode));
  setIcon(nowIcon, iconFor(weather->currentCode));

  for (int i = 0; i < config::kForecastDays; i++) {
    DayColumn &c = columns[i];
    if (i >= weather->dayCount) {
      lv_obj_add_flag(c.icon, LV_OBJ_FLAG_HIDDEN);
      setTextIfChanged(c.name, "");
      setTextIfChanged(c.high, "");
      setTextIfChanged(c.low, "");
      setTextIfChanged(c.rain, "");
      continue;
    }
    const ForecastDay &d = weather->days[i];
    setTextIfChanged(c.name, i == 0 ? "Today" : weekdayName(d.date));
    setIcon(c.icon, iconFor(d.code));
    lv_obj_clear_flag(c.icon, LV_OBJ_FLAG_HIDDEN);
    snprintf(text, sizeof(text), "%d\xC2\xB0", d.maxC);
    setTextIfChanged(c.high, text);
    snprintf(text, sizeof(text), "%d\xC2\xB0", d.minC);
    setTextIfChanged(c.low, text);
    snprintf(text, sizeof(text), "%d%%", d.rainPercent);
    setTextIfChanged(c.rain, text);
  }
}

void uiExtraSetTime(int hour, int minute, int second, bool colonVisible, const String &date) {
  if (!digitalBox) return;
  clockHour = hour;
  clockMinute = minute;
  clockSecond = second;
  placeDigital();
  lv_obj_set_style_text_opa(digitalColon, colonVisible ? LV_OPA_COVER : LV_OPA_TRANSP, 0);
  setTextIfChanged(digitalDate, date.c_str());
  lv_obj_invalidate(analogFace);
}
