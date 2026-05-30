#include "ui/radar_display.h"

#include <lgfx/v1/lgfx_fonts.hpp>

#include "hardware/display.h"
#include "hardware/display_font.h"
#include "ui/radar_theme.h"

namespace fonts = lgfx::v1::fonts;

namespace ui {
namespace radar {

uint16_t kColorBackground = 0x0000;
uint16_t kColorGrid = 0x0320;
uint16_t kColorLabel = 0xFFFF;
uint16_t kColorCenter = 0xFFFF;

}  // namespace radar

namespace {

constexpr auto& kLabelGfxFont = fonts::FreeSans12pt7b;

void initPalette() {
  radar::kColorBackground = tft.color565(0, 0, 0);
  radar::kColorGrid = tft.color565(0, 120, 0);
  radar::kColorLabel = tft.color565(255, 255, 255);
  radar::kColorCenter = tft.color565(255, 255, 255);
}

float labelScalePlusPx(float base_scale, int extra_px) {
  if (displayFontIsSmooth()) {
    tft.setTextSize(base_scale);
  } else {
    tft.setFont(&kLabelGfxFont);
    base_scale = 1.0f;
    tft.setTextSize(base_scale);
  }
  const int h = tft.fontHeight();
  if (h <= 0) {
    return base_scale;
  }
  return base_scale * static_cast<float>(h + extra_px) / static_cast<float>(h);
}

void applyLabelStyle(float size) {
  if (displayFontIsSmooth()) {
    tft.setTextSize(size);
  } else {
    tft.setFont(&kLabelGfxFont);
    tft.setTextSize(size);
  }
}

void drawLabel(const char* text, int x, int y, textdatum_t datum, float size) {
  tft.setTextDatum(datum);
  tft.setTextColor(radar::kColorLabel, radar::kColorBackground);
  applyLabelStyle(size);
  tft.drawString(text, x, y);
}

void drawLabelWithBackground(const char* text, int x, int y, textdatum_t datum,
                             float size) {
  applyLabelStyle(size);
  tft.setTextDatum(datum);

  const int tw = tft.textWidth(text);
  const int th = tft.fontHeight();
  constexpr int kPadX = 3;
  constexpr int kPadY = 2;

  int left = x;
  int top = y - th / 2;
  if (datum == textdatum_t::middle_left) {
    left = x - kPadX;
    top = y - th / 2 - kPadY;
  }

  tft.fillRect(left, top, tw + kPadX * 2, th + kPadY * 2,
               radar::kColorBackground);
  tft.setTextColor(radar::kColorLabel, radar::kColorBackground);
  tft.drawString(text, x, y);
}

void drawCircleThick(int cx, int cy, int r, uint16_t color) {
  tft.drawCircle(cx, cy, r, color);
  if (r > 0) {
    tft.drawCircle(cx, cy, r - 1, color);
  }
}

void drawRings(int cx, int cy, int outer_radius) {
  for (int i = 1; i <= radar::kRingCount; ++i) {
    const int r = (outer_radius * i) / radar::kRingCount;
    drawCircleThick(cx, cy, r, radar::kColorGrid);
  }
}

void drawCrosshairs(int cx, int cy, int radius, uint16_t color) {
  tft.drawLine(cx - 1, cy - radius, cx - 1, cy + radius, color);
  tft.drawLine(cx, cy - radius, cx, cy + radius, color);
  tft.drawLine(cx - radius, cy - 1, cx + radius, cy - 1, color);
  tft.drawLine(cx - radius, cy, cx + radius, cy, color);
}

void drawCenterDot(int cx, int cy) {
  tft.fillCircle(cx, cy, radar::kCenterDotRadius, radar::kColorCenter);
}

void drawCardinalLabels() {
  const int cx = radar::kCenterX;
  const int cy = radar::kCenterY;
  const int pad = radar::kEdgeInset;
  const float size =
      labelScalePlusPx(radar::kCardinalLabelSize, radar::kLabelExtraPx);

  drawLabel("N", cx, pad, textdatum_t::top_center, size);
  drawLabel("S", cx, radar::kSize - pad + radar::kCardinalSouthYOffset,
            textdatum_t::bottom_center, size);
  drawLabel("W", radar::kCardinalSideInset, cy, textdatum_t::middle_left, size);
  drawLabel("E", radar::kSize - radar::kCardinalSideInset, cy,
            textdatum_t::middle_right, size);
}

void drawScaleLabel(int cx, int cy, int outer_radius) {
  constexpr int kRingIndex = 3;
  const int r = (outer_radius * kRingIndex) / radar::kRingCount;
  const float size =
      labelScalePlusPx(radar::kScaleLabelSize, radar::kLabelExtraPx);
  const int x = cx + r + radar::kScaleLabelOffsetX;
  drawLabelWithBackground("10km", x, cy, textdatum_t::middle_left, size);
}

}  // namespace

void radarDisplayDraw() {
  initPalette();

  const int cx = radar::kCenterX;
  const int cy = radar::kCenterY;
  const int grid_r = radar::kGridOuterRadius;

  tft.fillScreen(radar::kColorBackground);

  drawRings(cx, cy, grid_r);
  drawCrosshairs(cx, cy, grid_r, radar::kColorGrid);
  drawCenterDot(cx, cy);
  drawCardinalLabels();
  drawScaleLabel(cx, cy, grid_r);

  tft.setTextDatum(textdatum_t::top_left);
}

}  // namespace ui
