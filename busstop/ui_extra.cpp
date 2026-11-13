#include "ui_extra.h"

#include <math.h>
#include <time.h>

#include "config.h"
#include "src/fonts/fonts.h"

namespace {

constexpr uint32_t kColorCard = 0x13171D;
constexpr uint32_t kColorFace = 0x0E1218;
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

enum IconKind { kSun, kPartCloud, kCloud, kFog, kRain, kSnow, kThunder, kMoon, kMoonCloud };

IconKind iconFor(int code, bool day = true) {
  if (code <= 1) return day ? kSun : kMoon;
  if (code == 2) return day ? kPartCloud : kMoonCloud;
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

// A crescent moon: a pale disc with a smaller disc in the card color cut out of it.
void drawMoon(lv_draw_ctx_t *ctx, float cx, float cy, float r) {
  fillCircle(ctx, cx, cy, r, 0xE8E4C9);
  fillCircle(ctx, cx + r * 0.55f, cy - r * 0.35f, r * 0.85f, kColorCard);
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
    case kMoon:
      drawMoon(ctx, x + 50 * s, y + 50 * s, 24 * s);
      fillCircle(ctx, x + 82 * s, y + 24 * s, 3 * s, 0xE8E4C9);
      fillCircle(ctx, x + 74 * s, y + 80 * s, 2 * s, 0xE8E4C9);
      break;
    case kMoonCloud:
      drawMoon(ctx, x + 36 * s, y + 34 * s, 18 * s);
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
constexpr int kCards = config::kForecastDays - 1;  // today is the big card, the rest are small cards
constexpr int kCardWidth = 106;
constexpr int kCardGap = 8;

struct DayCard {
  lv_obj_t *name, *icon, *high, *low, *rain;
};

DayCard cards[kCards];
lv_obj_t *nowIcon = nullptr, *nowTemp = nullptr, *nowText = nullptr;
lv_obj_t *nowHigh = nullptr, *nowLow = nullptr, *nowSun = nullptr;
lv_obj_t *statValue[4] = {};
lv_obj_t *digitalHours = nullptr, *digitalColon = nullptr, *digitalMinutes = nullptr, *digitalBox = nullptr;
lv_obj_t *digitalDate = nullptr;
lv_obj_t *analogFace = nullptr;
int clockHour = 0, clockMinute = 0, clockSecond = 0;

constexpr int kFaceSize = 284;
constexpr float kFaceRadius = kFaceSize / 2.0f;

lv_obj_t *makeCard(lv_obj_t *parent, int x, int y, int w, int h) {
  lv_obj_t *card = lv_obj_create(parent);
  lv_obj_set_size(card, w, h);
  lv_obj_set_pos(card, x, y);
  lv_obj_set_style_bg_color(card, lv_color_hex(kColorCard), 0);
  lv_obj_set_style_bg_opa(card, LV_OPA_COVER, 0);
  lv_obj_set_style_radius(card, 14, 0);
  lv_obj_set_style_border_width(card, 0, 0);
  lv_obj_set_style_pad_all(card, 0, 0);
  lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);
  return card;
}

// Label with a fixed width and alignment, placed at (x, y).
lv_obj_t *placeLabel(lv_obj_t *parent, const lv_font_t *font, uint32_t color, int x, int y, int width,
                     lv_text_align_t align) {
  lv_obj_t *label = makeLabel(parent, font, color);
  lv_obj_set_width(label, width);
  lv_label_set_long_mode(label, LV_LABEL_LONG_CLIP);
  lv_obj_set_style_text_align(label, align, 0);
  lv_obj_set_pos(label, x, y);
  return label;
}

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
    float outer = kFaceRadius - 10, inner = outer - (hour ? 14 : 6);
    strokeLine(ctx, cx + sinf(a) * inner, cy - cosf(a) * inner, cx + sinf(a) * outer, cy - cosf(a) * outer,
               hour ? 4 : 2, hour ? kColorTextPrimary : 0x4B5563);
  }

  float hourAngle = (clockHour % 12 + clockMinute / 60.0f) * 30.0f;
  float minuteAngle = (clockMinute + clockSecond / 60.0f) * 6.0f;
  hand(ctx, cx, cy, hourAngle, kFaceRadius * 0.5f, 12, 8, kColorTextPrimary);
  hand(ctx, cx, cy, minuteAngle, kFaceRadius * 0.76f, 14, 6, kColorTextPrimary);
  hand(ctx, cx, cy, clockSecond * 6.0f, kFaceRadius * 0.84f, 24, 3, kColorSecondHand);
  fillCircle(ctx, cx, cy, 9, kColorSecondHand);
  fillCircle(ctx, cx, cy, 3, 0x000000);
}

void createWeatherPage(lv_obj_t *page) {
  // Big card: now
  lv_obj_t *hero = makeCard(page, 16, 10, 448, 98);
  nowIcon = makeIcon(hero, 76);
  lv_obj_set_pos(nowIcon, 14, 11);
  setIcon(nowIcon, kSun);

  nowTemp = makeLabel(hero, &busstop_font_temp, kColorTextPrimary, "--\xC2\xB0""C");
  lv_obj_set_pos(nowTemp, 106, 8);
  nowText = placeLabel(hero, FONT_MEDIUM, kColorTextSecondary, 106, 58, 190, LV_TEXT_ALIGN_LEFT);

  nowHigh = placeLabel(hero, FONT_MEDIUM, kColorTextPrimary, 290, 12, 142, LV_TEXT_ALIGN_RIGHT);
  nowLow = placeLabel(hero, FONT_MEDIUM, kColorTextSecondary, 290, 42, 142, LV_TEXT_ALIGN_RIGHT);
  nowSun = placeLabel(hero, FONT_SMALL, kColorSun, 270, 72, 162, LV_TEXT_ALIGN_RIGHT);

  // Details strip: feels like, wind, humidity, rain
  lv_obj_t *strip = makeCard(page, 16, 116, 448, 56);
  const char *titles[4] = {"Feels like", "Wind", "Humidity", "Rain"};
  for (int i = 0; i < 4; i++) {
    int x = i * 112;
    placeLabel(strip, FONT_SMALL, kColorTextSecondary, x, 6, 112, LV_TEXT_ALIGN_CENTER);
    lv_label_set_text(lv_obj_get_child(strip, -1), titles[i]);
    statValue[i] = placeLabel(strip, FONT_MEDIUM, kColorTextPrimary, x, 25, 112, LV_TEXT_ALIGN_CENTER);
    lv_label_set_text(statValue[i], "--");
  }

  // Next days
  for (int i = 0; i < kCards; i++) {
    DayCard &c = cards[i];
    int x = 16 + i * (kCardWidth + kCardGap);
    lv_obj_t *card = makeCard(page, x, 180, kCardWidth, 120);
    c.name = placeLabel(card, FONT_SMALL, kColorTextSecondary, 0, 6, kCardWidth, LV_TEXT_ALIGN_CENTER);
    c.icon = makeIcon(card, 42);
    lv_obj_set_pos(c.icon, (kCardWidth - 42) / 2, 26);
    setIcon(c.icon, kCloud);
    c.high = placeLabel(card, FONT_MEDIUM, kColorTextPrimary, 0, 70, kCardWidth, LV_TEXT_ALIGN_CENTER);
    c.low = placeLabel(card, FONT_SMALL, kColorTextSecondary, 4, 97, 48, LV_TEXT_ALIGN_CENTER);
    c.rain = placeLabel(card, FONT_SMALL, kColorRain, 54, 97, 48, LV_TEXT_ALIGN_CENTER);
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
  int x = (lv_obj_get_width(digitalBox) - (h.x + c.x + m.x)) / 2;
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
  lv_obj_set_pos(digitalBox, 0, (config::kScreenHeight - size.y) / 2 - 36);
  lv_obj_set_style_bg_opa(digitalBox, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(digitalBox, 0, 0);
  lv_obj_set_style_pad_all(digitalBox, 0, 0);
  lv_obj_clear_flag(digitalBox, LV_OBJ_FLAG_SCROLLABLE);

  digitalHours = makeLabel(digitalBox, &busstop_font_clock, kColorTextPrimary, "00");
  digitalColon = makeLabel(digitalBox, &busstop_font_clock, kColorAccent, ":");
  digitalMinutes = makeLabel(digitalBox, &busstop_font_clock, kColorTextPrimary, "00");
  placeDigital();

  digitalDate = placeLabel(page, FONT_LARGE, kColorTextSecondary, 0, (config::kScreenHeight + size.y) / 2 - 18,
                           config::kScreenWidth, LV_TEXT_ALIGN_CENTER);
}

void createAnalogPage(lv_obj_t *page) {
  analogFace = lv_obj_create(page);
  lv_obj_set_size(analogFace, kFaceSize, kFaceSize);
  lv_obj_set_pos(analogFace, (config::kScreenWidth - kFaceSize) / 2, 8);
  lv_obj_set_style_radius(analogFace, LV_RADIUS_CIRCLE, 0);
  lv_obj_set_style_bg_color(analogFace, lv_color_hex(kColorFace), 0);
  lv_obj_set_style_bg_opa(analogFace, LV_OPA_COVER, 0);
  lv_obj_set_style_border_width(analogFace, 3, 0);
  lv_obj_set_style_border_color(analogFace, lv_color_hex(0x2A3340), 0);
  lv_obj_set_style_pad_all(analogFace, 0, 0);
  lv_obj_clear_flag(analogFace, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_add_event_cb(analogFace, drawFace, LV_EVENT_DRAW_MAIN_END, nullptr);

  // Numbers 12, 3, 6, 9
  const int numbers[4] = {12, 3, 6, 9};
  for (int i = 0; i < 4; i++) {
    float a = i * (float)M_PI / 2;
    char text[4];
    snprintf(text, sizeof(text), "%d", numbers[i]);
    lv_obj_t *label = makeLabel(analogFace, FONT_MEDIUM, kColorTextPrimary, text);
    lv_point_t size;
    lv_txt_get_size(&size, text, FONT_MEDIUM, 0, 0, LV_COORD_MAX, LV_TEXT_FLAG_NONE);
    float r = kFaceRadius - 46;
    // object coordinates are measured inside the border, so shift by the border width
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
  nowIcon = nowTemp = nowText = nowHigh = nowLow = nowSun = nullptr;
  for (lv_obj_t *&v : statValue) v = nullptr;
  digitalHours = digitalColon = digitalMinutes = digitalBox = digitalDate = nullptr;
  analogFace = nullptr;
  for (DayCard &c : cards) c = DayCard{};
}

void uiExtraSetWeather(const Weather *weather) {
  if (!nowTemp) return;
  char text[32];
  if (!weather) {
    setTextIfChanged(nowTemp, "--\xC2\xB0""C");
    return;
  }

  snprintf(text, sizeof(text), "%d\xC2\xB0""C", weather->currentC);
  setTextIfChanged(nowTemp, text);
  setIcon(nowIcon, iconFor(weather->currentCode, weather->isDay));
  setTextIfChanged(nowText, describe(weather->currentCode));

  if (weather->dayCount > 0) {
    const ForecastDay &today = weather->days[0];
    snprintf(text, sizeof(text), "High %d\xC2\xB0", today.maxC);
    setTextIfChanged(nowHigh, text);
    snprintf(text, sizeof(text), "Low %d\xC2\xB0", today.minC);
    setTextIfChanged(nowLow, text);
    snprintf(text, sizeof(text), "%d%%", today.rainPercent);
    setTextIfChanged(statValue[3], text);
  }
  if (weather->sunrise[0] && weather->sunset[0]) {
    snprintf(text, sizeof(text), "Sun %s - %s", weather->sunrise, weather->sunset);
    setTextIfChanged(nowSun, text);
  }
  snprintf(text, sizeof(text), "%d\xC2\xB0", weather->feelsC);
  setTextIfChanged(statValue[0], text);
  snprintf(text, sizeof(text), "%d m/s", weather->windMs);
  setTextIfChanged(statValue[1], text);
  snprintf(text, sizeof(text), "%d%%", weather->humidity);
  setTextIfChanged(statValue[2], text);

  for (int i = 0; i < kCards; i++) {
    DayCard &c = cards[i];
    int day = i + 1;
    if (day >= weather->dayCount) {
      setTextIfChanged(c.name, "");
      setTextIfChanged(c.high, "");
      setTextIfChanged(c.low, "");
      setTextIfChanged(c.rain, "");
      continue;
    }
    const ForecastDay &d = weather->days[day];
    setTextIfChanged(c.name, weekdayName(d.date));
    setIcon(c.icon, iconFor(d.code));
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
  lv_obj_set_style_text_color(digitalColon, lv_color_hex(kColorAccent), 0);
  lv_obj_set_style_text_opa(digitalColon, colonVisible ? LV_OPA_COVER : LV_OPA_TRANSP, 0);
  setTextIfChanged(digitalDate, date.c_str());
  lv_obj_invalidate(analogFace);
}
