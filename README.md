# round-screen

Firmware for an **ESP32-C3 Super Mini** and a **1.28″ round GC9A01 display** (240×240). Uses **LovyanGFX**, **WiFiManager**, and a circular **radar/sonar** UI.

## Flow

1. **Wi‑Fi setup** — yellow status screens + captive portal (`RoundScreen-Setup` → http://192.168.4.1)
2. **Radar UI** — black circular sonar grid with green crosshairs, rings, and labels (step 2)

Hold **BOOT** (GPIO **9**) for **3 seconds** to clear saved Wi‑Fi.

## Project layout

```
include/
  config.h                 — pins, Wi‑Fi, display constants
  hardware/
    lgfx_config.hpp        — GC9A01 + SPI
    display.h              — panel init
    display_font.h         — VLW smooth font
  ui/
    radar_theme.h          — radar colors & geometry
    radar_display.h        — sonar grid component
    status_screens.h       — Wi‑Fi setup / error screens
  services/
    wifi_setup.h           — WiFiManager + BOOT reset
data/
  ui_font.vlw              — embedded UI font
src/
  main.cpp
  hardware/
  ui/
  services/
```

## Wiring

| Display | ESP32-C3 Super Mini |
|---------|---------------------|
| VCC | 3V3 |
| GND | GND |
| RST | GPIO **0** |
| CS | GPIO **1** |
| DC | GPIO **10** |
| SDA | GPIO **3** |
| SCL | GPIO **4** |

Edit **`include/config.h`** for pins (`GPIO_NUM_*`) and AP name. If colors look inverted, toggle `kDisplayInvert` / `kDisplayRgbOrder`.

## Build

```bash
pio run -t upload
pio device monitor
```

Serial: **115200** baud.

## Radar UI

Static grid per spec:

- Black background, dark green 2 px antialiased grid
- White **N / S / E / W** at the round edge; **10km** on ring 3 (east)
- White center dot (physical round bezel hides unused square corners)

Tweak layout in `include/ui/radar_theme.h`.
