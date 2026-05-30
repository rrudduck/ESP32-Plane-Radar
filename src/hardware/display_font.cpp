#include "hardware/display_font.h"

#include "hardware/display.h"

extern "C" {
extern const uint8_t _binary_data_ui_font_vlw_start[] asm(
    "_binary_data_ui_font_vlw_start");
extern const uint8_t _binary_data_ui_font_vlw_end[] asm("_binary_data_ui_font_vlw_end");
}

namespace {

bool s_vlw_loaded = false;

}  // namespace

bool displayFontInit() {
  const size_t font_len = static_cast<size_t>(_binary_data_ui_font_vlw_end -
                                              _binary_data_ui_font_vlw_start);
  s_vlw_loaded =
      tft.loadFont(_binary_data_ui_font_vlw_start, lgfx::IFont::ft_vlw) &&
      font_len > 0;
  if (!s_vlw_loaded) {
    Serial.println("Smooth font load failed — using bitmap fallback");
  }
  return s_vlw_loaded;
}

bool displayFontIsSmooth() { return s_vlw_loaded; }
