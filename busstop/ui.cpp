#include "ui.h"

#include <lvgl.h>

#include "config.h"
#include "src/fonts/fonts.h"

namespace {

// --- Layout (480 x 320) ------------------------------------------------------
constexpr int kMargin = 16;
constexpr int kCardWidth = config::kScreenWidth - 2 * kMargin;
constexpr int kDividerY = 74;
constexpr int kRowsTop = 82;
constexpr int kRowPitch = 47;
constexpr int kRowHeight = 43;
constexpr int kBadgeWidth = 60;
constexpr int kBadgeHeight = 30;
constexpr int kDestinationX = 78;
constexpr int kDestinationWidth = 224;
constexpr int kNumberRightInset = 56;
constexpr int kNumberWidth = 84;
constexpr int kStopLabelWidth = 276;

// --- Colors ----------------------------------------------------------------
constexpr uint32_t kColorBackground = 0x000000;
constexpr uint32_t kColorCard = 0x13171D;
constexpr uint32_t kColorDivider = 0x232B36;
constexpr uint32_t kColorTextPrimary = 0xF2F5F9;
constexpr uint32_t kColorTextSecondary = 0x8A94A6;
constexpr uint32_t kColorAccent = 0x5EEAD4;
constexpr uint32_t kColorBadgeDefault = 0x2F6FED;
constexpr uint32_t kColorSoon = 0xFF8A3D;    // 0-2 min
constexpr uint32_t kColorMedium = 0xFFC53D;  // 3-9 min
constexpr uint32_t kColorLater = 0x4ADE80;   // 10+ min
constexpr uint32_t kColorStale = 0x6B7280;   // data is old
constexpr uint32_t kColorCancelled = 0xFF5C5C;

#define FONT_SMALL (&busstop_font_16)
#define FONT_MEDIUM (&busstop_font_24)
#define FONT_LARGE (&busstop_font_32)

// --- Widgets ---------------------------------------------------------------
struct Row {
  lv_obj_t *card;
  lv_obj_t *badge;
  lv_obj_t *badgeText;
  lv_obj_t *destination;
  lv_obj_t *number;
  lv_obj_t *unit;
};

Row rows[config::kMaxDepartures];
int rowMinutes[config::kMaxDepartures] = {};
bool stale = false;
lv_obj_t *stopLabel = nullptr;
lv_obj_t *dateLabel = nullptr;
lv_obj_t *noticeLabel = nullptr;
lv_obj_t *clockBox = nullptr;
lv_obj_t *clockHours = nullptr;
lv_obj_t *clockColon = nullptr;
lv_obj_t *clockMinutes = nullptr;
lv_obj_t *tempLabel = nullptr;
lv_obj_t *statusLabel = nullptr;
lv_obj_t *setupStatusLabel = nullptr;

// --- Helpers ---------------------------------------------------------------
lv_obj_t *makeLabel(lv_obj_t *parent, const lv_font_t *font, uint32_t color, const char *text = "") {
  lv_obj_t *label = lv_label_create(parent);
  lv_obj_set_style_text_font(label, font, 0);
  lv_obj_set_style_text_color(label, lv_color_hex(color), 0);
  lv_label_set_text(label, text);
  return label;
}

// A plain rounded box: no border, no padding, no scrolling.
lv_obj_t *makeBox(lv_obj_t *parent, int width, int height, uint32_t color, int radius) {
  lv_obj_t *box = lv_obj_create(parent);
  lv_obj_set_size(box, width, height);
  lv_obj_set_style_bg_color(box, lv_color_hex(color), 0);
  lv_obj_set_style_bg_opa(box, LV_OPA_COVER, 0);
  lv_obj_set_style_radius(box, radius, 0);
  lv_obj_set_style_border_width(box, 0, 0);
  lv_obj_set_style_pad_all(box, 0, 0);
  lv_obj_clear_flag(box, LV_OBJ_FLAG_SCROLLABLE);
  return box;
}

// Only touches the label when the text changed (no redraw, no scroll restart).
void setTextIfChanged(lv_obj_t *label, const char *text) {
  if (strcmp(lv_label_get_text(label), text) != 0) lv_label_set_text(label, text);
}

void setScrollingLabel(lv_obj_t *label, int width) {
  lv_obj_set_width(label, width);
  lv_label_set_long_mode(label, LV_LABEL_LONG_SCROLL_CIRCULAR);
}

// True for light colors (they need dark text).
bool isBright(uint32_t rgb) {
  int r = (rgb >> 16) & 0xFF, g = (rgb >> 8) & 0xFF, b = rgb & 0xFF;
  return (r * 299 + g * 587 + b * 114) / 1000 > 150;
}

uint32_t minutesColor(int minutes) {
  if (minutes <= 2) return kColorSoon;
  if (minutes <= 9) return kColorMedium;
  return kColorLater;
}

void createRow(int index) {
  Row &r = rows[index];

  r.card = makeBox(lv_scr_act(), kCardWidth, kRowHeight, kColorCard, 10);
  lv_obj_set_pos(r.card, kMargin, kRowsTop + index * kRowPitch);
  lv_obj_add_flag(r.card, LV_OBJ_FLAG_HIDDEN);

  r.badge = makeBox(r.card, kBadgeWidth, kBadgeHeight, kColorBadgeDefault, 8);
  lv_obj_align(r.badge, LV_ALIGN_LEFT_MID, 8, 0);
  r.badgeText = makeLabel(r.badge, FONT_MEDIUM, kColorTextPrimary);

  r.destination = makeLabel(r.card, FONT_MEDIUM, kColorTextPrimary);
  setScrollingLabel(r.destination, kDestinationWidth);
  lv_obj_align(r.destination, LV_ALIGN_LEFT_MID, kDestinationX, 0);

  r.number = makeLabel(r.card, FONT_LARGE, kColorLater);
  lv_obj_set_width(r.number, kNumberWidth);
  lv_label_set_long_mode(r.number, LV_LABEL_LONG_CLIP);
  lv_obj_set_style_text_align(r.number, LV_TEXT_ALIGN_RIGHT, 0);
  lv_obj_align(r.number, LV_ALIGN_RIGHT_MID, -kNumberRightInset, 0);

  r.unit = makeLabel(r.card, FONT_SMALL, kColorTextSecondary, "min");
  lv_obj_align(r.unit, LV_ALIGN_RIGHT_MID, -14, 4);
}

void applyNumberColor(int index) {
  uint32_t color = stale ? kColorStale : minutesColor(rowMinutes[index]);
  lv_obj_set_style_text_color(rows[index].number, lv_color_hex(color), 0);
}

void showStatusLabel(bool visible) {
  if (visible) lv_obj_clear_flag(statusLabel, LV_OBJ_FLAG_HIDDEN);
  else lv_obj_add_flag(statusLabel, LV_OBJ_FLAG_HIDDEN);
}

void styleScreen() {
  lv_obj_t *screen = lv_scr_act();
  lv_obj_set_style_bg_color(screen, lv_color_hex(kColorBackground), 0);
  lv_obj_set_scrollbar_mode(screen, LV_SCROLLBAR_MODE_OFF);
}

}  // namespace

// ============================================================================
// Main screen
// ============================================================================

void uiCreate() {
  styleScreen();
  lv_obj_t *screen = lv_scr_act();

  // Header: stop name + date (left), clock + temperature (right)
  stopLabel = makeLabel(screen, FONT_MEDIUM, kColorTextPrimary, "Loading Stop");
  setScrollingLabel(stopLabel, kStopLabelWidth);
  lv_obj_set_pos(stopLabel, kMargin, 8);

  dateLabel = makeLabel(screen, FONT_SMALL, kColorTextSecondary);
  lv_obj_set_pos(dateLabel, kMargin, 46);

  noticeLabel = makeLabel(screen, FONT_SMALL, kColorSoon);
  lv_obj_set_width(noticeLabel, 190);
  lv_label_set_long_mode(noticeLabel, LV_LABEL_LONG_CLIP);
  lv_obj_set_pos(noticeLabel, 130, 46);

  // Clock: hours, colon, minutes placed side by side so only the colon can blink
  lv_point_t digitsSize, colonSize;
  lv_txt_get_size(&digitsSize, "00", FONT_LARGE, 0, 0, LV_COORD_MAX, LV_TEXT_FLAG_NONE);
  lv_txt_get_size(&colonSize, ":", FONT_LARGE, 0, 0, LV_COORD_MAX, LV_TEXT_FLAG_NONE);

  clockBox = lv_obj_create(screen);
  lv_obj_set_size(clockBox, digitsSize.x * 2 + colonSize.x, digitsSize.y);
  lv_obj_set_style_bg_opa(clockBox, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(clockBox, 0, 0);
  lv_obj_set_style_pad_all(clockBox, 0, 0);
  lv_obj_clear_flag(clockBox, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_align(clockBox, LV_ALIGN_TOP_RIGHT, -kMargin, 6);

  clockHours = makeLabel(clockBox, FONT_LARGE, kColorTextPrimary, "00");
  clockColon = makeLabel(clockBox, FONT_LARGE, kColorTextPrimary, ":");
  clockMinutes = makeLabel(clockBox, FONT_LARGE, kColorTextPrimary, "00");
  uiSetClock("00:00");

  tempLabel = makeLabel(screen, FONT_MEDIUM, kColorAccent, "--\xC2\xB0""C");
  lv_obj_align(tempLabel, LV_ALIGN_TOP_RIGHT, -kMargin, 42);

  lv_obj_t *divider = makeBox(screen, kCardWidth, 1, kColorDivider, 0);
  lv_obj_set_pos(divider, kMargin, kDividerY);

  // Departure cards (hidden until there is data)
  for (int i = 0; i < config::kMaxDepartures; i++) createRow(i);

  statusLabel = makeLabel(screen, FONT_MEDIUM, kColorTextSecondary, "Loading...");
  lv_obj_set_width(statusLabel, kCardWidth);
  lv_obj_set_style_text_align(statusLabel, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_align(statusLabel, LV_ALIGN_CENTER, 0, 36);
}

void uiSetStopName(const String &name) {
  if (stopLabel) setTextIfChanged(stopLabel, name.c_str());
}

void uiSetClock(const String &time) {  // "HH:MM"
  if (!clockHours) return;
  String hours = time.substring(0, 2);
  String minutes = time.substring(3, 5);
  setTextIfChanged(clockHours, hours.c_str());
  setTextIfChanged(clockMinutes, minutes.c_str());

  // Place the parts by their real widths so the right edge stays aligned
  lv_point_t h, c, m;
  lv_txt_get_size(&h, hours.c_str(), FONT_LARGE, 0, 0, LV_COORD_MAX, LV_TEXT_FLAG_NONE);
  lv_txt_get_size(&c, ":", FONT_LARGE, 0, 0, LV_COORD_MAX, LV_TEXT_FLAG_NONE);
  lv_txt_get_size(&m, minutes.c_str(), FONT_LARGE, 0, 0, LV_COORD_MAX, LV_TEXT_FLAG_NONE);
  int x = lv_obj_get_width(clockBox) - (h.x + c.x + m.x);
  lv_obj_set_width(clockHours, h.x);
  lv_obj_set_pos(clockHours, x, 0);
  lv_obj_set_pos(clockColon, x + h.x, 0);
  lv_obj_set_width(clockMinutes, m.x);
  lv_obj_set_pos(clockMinutes, x + h.x + c.x, 0);
}

void uiSetClockColon(bool visible) {
  if (clockColon) lv_obj_set_style_text_opa(clockColon, visible ? LV_OPA_COVER : LV_OPA_TRANSP, 0);
}

void uiSetDate(const String &date) {
  if (dateLabel) setTextIfChanged(dateLabel, date.c_str());
}

void uiSetTemperature(int celsius) {
  if (!tempLabel) return;
  char text[16];
  snprintf(text, sizeof(text), "%d\xC2\xB0""C", celsius);
  setTextIfChanged(tempLabel, text);
}

void uiSetTemperatureUnknown() {
  if (tempLabel) setTextIfChanged(tempLabel, "--\xC2\xB0""C");
}

void uiSetNotice(const String &message) {
  if (noticeLabel) setTextIfChanged(noticeLabel, message.c_str());
}

void uiSetRow(int index, const String &line, const String &destination, int minutes,
              uint32_t badgeColor, uint32_t badgeTextColor, bool cancelled) {
  if (index < 0 || index >= config::kMaxDepartures) return;
  Row &r = rows[index];

  if (badgeColor == config::kNoColor) badgeColor = kColorBadgeDefault;
  if (badgeTextColor == config::kNoColor) badgeTextColor = isBright(badgeColor) ? 0x101010 : kColorTextPrimary;
  lv_obj_set_style_bg_color(r.badge, lv_color_hex(badgeColor), 0);
  lv_obj_set_style_text_color(r.badgeText, lv_color_hex(badgeTextColor), 0);
  setTextIfChanged(r.badgeText, line.c_str());
  lv_obj_center(r.badgeText);

  setTextIfChanged(r.destination, destination.c_str());
  lv_obj_set_style_text_color(r.destination, lv_color_hex(cancelled ? kColorStale : kColorTextPrimary), 0);
  lv_obj_set_style_text_decor(r.destination, cancelled ? LV_TEXT_DECOR_STRIKETHROUGH : LV_TEXT_DECOR_NONE, 0);

  rowMinutes[index] = minutes;
  if (cancelled) {
    setTextIfChanged(r.number, "");
    setTextIfChanged(r.unit, "Cancelled");
    lv_obj_set_style_text_color(r.unit, lv_color_hex(kColorCancelled), 0);
    lv_obj_clear_flag(r.unit, LV_OBJ_FLAG_HIDDEN);
  } else {
    setTextIfChanged(r.unit, "min");
    lv_obj_set_style_text_color(r.unit, lv_color_hex(kColorTextSecondary), 0);
    if (minutes <= 0) {
      setTextIfChanged(r.number, "Now");
      lv_obj_align(r.number, LV_ALIGN_RIGHT_MID, -14, 0);
      lv_obj_add_flag(r.unit, LV_OBJ_FLAG_HIDDEN);
    } else {
      char text[8];
      snprintf(text, sizeof(text), "%d", minutes);
      setTextIfChanged(r.number, text);
      lv_obj_align(r.number, LV_ALIGN_RIGHT_MID, -kNumberRightInset, 0);
      lv_obj_clear_flag(r.unit, LV_OBJ_FLAG_HIDDEN);
    }
    applyNumberColor(index);

  }

  lv_obj_clear_flag(r.card, LV_OBJ_FLAG_HIDDEN);
  showStatusLabel(false);
}

void uiSetStale(bool isStale) {
  if (stale == isStale) return;
  stale = isStale;
  for (int i = 0; i < config::kMaxDepartures; i++) {
    if (rows[i].number) applyNumberColor(i);
  }
}

void uiClearRowsFrom(int firstRow) {
  for (int i = firstRow < 0 ? 0 : firstRow; i < config::kMaxDepartures; i++) {
    lv_obj_add_flag(rows[i].card, LV_OBJ_FLAG_HIDDEN);
  }
}

void uiSetStatus(const String &message) {
  uiClearRowsFrom(0);
  setTextIfChanged(statusLabel, message.c_str());
  showStatusLabel(true);
}

// ============================================================================
// Setup screen
// ============================================================================

void uiCreateSetup(const char *accessPointName) {
  lv_obj_t *screen = lv_scr_act();
  lv_obj_clean(screen);  // remove the main screen (when opened from a long press)
  styleScreen();
  stopLabel = dateLabel = noticeLabel = clockHours = clockColon = clockMinutes = tempLabel = statusLabel = nullptr;

  lv_obj_t *title = makeLabel(screen, FONT_LARGE, kColorAccent, "Setup");
  lv_obj_set_pos(title, 20, 15);

  lv_obj_t *steps = makeLabel(screen, FONT_MEDIUM, kColorTextPrimary);
  lv_label_set_long_mode(steps, LV_LABEL_LONG_WRAP);
  lv_obj_set_width(steps, 440);
  lv_label_set_text_fmt(steps,
                        "1. On your phone, join the WiFi:\n   %s\n"
                        "2. The setup page opens.\n   If not, go to 192.168.4.1\n"
                        "3. Pick your WiFi and bus stop.",
                        accessPointName);
  lv_obj_set_pos(steps, 20, 70);

  setupStatusLabel = makeLabel(screen, FONT_MEDIUM, kColorSoon, "Waiting for phone...");
  lv_obj_set_pos(setupStatusLabel, 20, 275);
}

void uiSetSetupStatus(const String &message) {
  if (setupStatusLabel) lv_label_set_text(setupStatusLabel, message.c_str());
}
