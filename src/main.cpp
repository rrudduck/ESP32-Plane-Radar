/**
 * Plane Radar — WiFi setup, then radar UI on the round GC9A01 display.
 */

#include <Arduino.h>
#include <WiFi.h>

#include "config.h"
#include "hardware/display.h"
#include "services/adsb_client.h"
#include "services/ota_update.h"
#include "services/radar_location.h"
#include "services/wifi_setup.h"
#include "ui/radar_display.h"
#include "ui/radar_range.h"
#include "ui/status_screens.h"

namespace {

bool g_radar_visible = false;
unsigned long g_wifi_down_since = 0;
unsigned long g_last_reconnect_ms = 0;
unsigned long g_last_adsb_fetch_ms = 0;
unsigned long g_adsb_interval_ms = config::kAdsbFetchIntervalMs;
unsigned long g_last_adsb_success_ms = 0;
unsigned long g_last_wifi_kick_ms = 0;
uint8_t g_adsb_failures = 0;

constexpr unsigned long kAdsbBackoffMaxMs = 30000;
constexpr unsigned long kAdsbStaleKickMs = 90000;
constexpr unsigned long kWifiKickIntervalMs = 30000;
constexpr uint8_t kAdsbFailureKickThreshold = 4;

void showRadarIfConnected() {
  if (WiFi.status() != WL_CONNECTED) {
    g_radar_visible = false;
    return;
  }
  ui::radarDisplayDraw();
  g_radar_visible = true;
}

void onRangeTap() {
  ui::radar::rangeNext();
  char range_label[12];
  ui::radar::formatCurrentRing3Label(range_label, sizeof(range_label));
  Serial.printf("Range: %s (outer ~%.0f km)\n", range_label,
                ui::radar::rangeCurrent().outer_km);

  if (g_radar_visible && WiFi.status() == WL_CONNECTED) {
    ui::radarDisplayDraw();
  }
}

void handleBootButton() {
  bootButtonPollLongPress();
  if (bootButtonConsumeTap()) {
    onRangeTap();
  }
}

void fetchAndDrawAircraft() {
  const float fetch_km = ui::radar::fetchRadiusKm();
  if (!services::adsb::fetchUpdate(services::location::lat(),
                                   services::location::lon(), fetch_km)) {
    ++g_adsb_failures;
    g_adsb_interval_ms =
        g_adsb_interval_ms < kAdsbBackoffMaxMs / 2 ? g_adsb_interval_ms * 2
                                                   : kAdsbBackoffMaxMs;
    Serial.printf("adsb: fetch failed, backoff=%lu ms\n", g_adsb_interval_ms);

    const bool kick_allowed = millis() - g_last_wifi_kick_ms >= kWifiKickIntervalMs;
    if (WiFi.status() == WL_CONNECTED &&
        g_adsb_failures >= kAdsbFailureKickThreshold && kick_allowed) {
      g_last_wifi_kick_ms = millis();
      Serial.println("adsb: repeated failures, requesting WiFi reconnect");
      WiFi.reconnect();
    }
    handleBootButton();
    return;
  }

  g_adsb_failures = 0;
  g_last_adsb_success_ms = millis();
  if (g_adsb_interval_ms != config::kAdsbFetchIntervalMs) {
    Serial.println("adsb: fetch recovered");
  }
  g_adsb_interval_ms = config::kAdsbFetchIntervalMs;
  ui::radarDisplayRefreshAircraft();
  handleBootButton();
}

}  // namespace

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println();
  Serial.println("Plane Radar");

  bootButtonInit();
  displayInit();
  if (wifiShowsSetupScreenOnBoot()) {
    statusScreenPortal();
  }
  services::location::init();
  ui::radar::rangeInit();
  services::adsb::setPollFn(wifiLoop);

  if (wifiSetupConnect()) {
    showRadarIfConnected();
  }
}

void loop() {
  handleBootButton();
  wifiLoop();
  services::ota::loop();

  if (WiFi.status() != WL_CONNECTED) {
    if (g_radar_visible) {
      Serial.println("WiFi lost — will reconnect");
      g_radar_visible = false;
    }

    if (g_wifi_down_since == 0) {
      g_wifi_down_since = millis();
    }

    const unsigned long down_ms = millis() - g_wifi_down_since;
    if (down_ms >= config::kWifiDownGraceMs &&
        millis() - g_last_reconnect_ms >= config::kWifiReconnectIntervalMs) {
      g_last_reconnect_ms = millis();
      if (wifiReconnect()) {
        g_wifi_down_since = 0;
        showRadarIfConnected();
      }
    }
  } else {
    g_wifi_down_since = 0;

    if (g_last_adsb_success_ms != 0 &&
        millis() - g_last_adsb_success_ms >= kAdsbStaleKickMs &&
        millis() - g_last_wifi_kick_ms >= kWifiKickIntervalMs) {
      g_last_wifi_kick_ms = millis();
      Serial.println("adsb: stale data window exceeded, requesting WiFi reconnect");
      WiFi.reconnect();
    }

    if (!g_radar_visible) {
      showRadarIfConnected();
    } else if (millis() - g_last_adsb_fetch_ms >= g_adsb_interval_ms) {
      g_last_adsb_fetch_ms = millis();
      fetchAndDrawAircraft();
    }
  }

  delay(10);
}
