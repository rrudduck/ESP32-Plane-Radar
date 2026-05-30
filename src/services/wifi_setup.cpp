#include "services/wifi_setup.h"

#include <WiFi.h>
#include <WiFiManager.h>

#include <esp_system.h>

#include "config.h"
#include "ui/status_screens.h"

namespace {

void initBootButton() {
  pinMode(config::kBootPin, INPUT_PULLUP);
}

bool bootHeldFor(unsigned long hold_ms) {
  if (digitalRead(config::kBootPin) != LOW) {
    return false;
  }
  const unsigned long start = millis();
  while (digitalRead(config::kBootPin) == LOW) {
    if (millis() - start >= hold_ms) {
      return true;
    }
    delay(10);
  }
  return false;
}

void resetWifiCredentials() {
  WiFiManager wm;
  wm.resetSettings();
  WiFi.disconnect(true, true);
  Serial.println("WiFi credentials cleared");
}

void configureWifiManager(WiFiManager& wm) {
  wm.setConfigPortalTimeout(config::kWifiPortalTimeoutSec);
  wm.setAPStaticIPConfig(IPAddress(192, 168, 4, 1), IPAddress(192, 168, 4, 1),
                         IPAddress(255, 255, 255, 0));
  wm.setAPCallback([](WiFiManager*) { statusScreenPortal(); });
}

bool wifiLinkUp() {
  return WiFi.status() == WL_CONNECTED &&
         WiFi.localIP() != IPAddress(0, 0, 0, 0);
}

void prepareSta() {
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(WIFI_PS_NONE);
  WiFi.setAutoReconnect(true);
}

void startStaConnect(const String& ssid, const String& pass) {
  prepareSta();
  if (ssid.length() > 0) {
    WiFi.begin(ssid.c_str(), pass.c_str());
  } else {
    WiFi.begin();
  }
}

bool waitForLinkWithUi(const char* ssid_for_ui, unsigned long attempt_ms) {
  const unsigned long deadline = millis() + attempt_ms;
  while (millis() < deadline) {
    if (wifiLinkUp()) {
      return true;
    }
    statusScreenConnectingTick();
    delay(config::kWifiConnectingFrameMs);
  }
  return wifiLinkUp();
}

bool tryConnectWithUi(const String& ssid, const String& pass, bool show_ui) {
  if (wifiLinkUp()) {
    return true;
  }

  const char* ui_ssid = ssid.length() > 0 ? ssid.c_str() : "network";
  if (show_ui) {
    statusScreenConnectingBegin(ui_ssid);
  }

  for (uint8_t attempt = 1; attempt <= config::kWifiConnectAttempts; ++attempt) {
    if (attempt > 1) {
      Serial.printf("WiFi connect retry %u/%u\n", attempt,
                    config::kWifiConnectAttempts);
      WiFi.disconnect(true);
      WiFi.mode(WIFI_OFF);
      delay(400);
    }

    startStaConnect(ssid, pass);

    if (waitForLinkWithUi(ui_ssid, config::kWifiConnectAttemptMs)) {
      return true;
    }
  }

  return false;
}

bool connectSavedNetwork(bool show_ui) {
  WiFiManager wm;
  if (!wm.getWiFiIsSaved()) {
    return false;
  }

  const String ssid = wm.getWiFiSSID();
  const String pass = wm.getWiFiPass();
  return tryConnectWithUi(ssid, pass, show_ui);
}

}  // namespace

bool wifiBootButtonPressed() {
  return digitalRead(config::kBootPin) == LOW;
}

bool wifiClearCredentialsIfBootHeld() {
  initBootButton();
  if (!bootHeldFor(config::kBootResetHoldMs)) {
    return false;
  }

  resetWifiCredentials();
  statusScreenWifiReset();
  delay(800);
  return true;
}

void wifiResetCredentialsAndReboot() {
  resetWifiCredentials();
  statusScreenPortal();
  delay(500);
  esp_restart();
}

bool wifiReconnect() {
  initBootButton();
  Serial.println("WiFi reconnecting...");
  return connectSavedNetwork(true);
}

bool wifiSetupConnect() {
  initBootButton();

  WiFiManager wm;
  configureWifiManager(wm);

  Serial.println("Connecting to WiFi (portal opens if needed)...");

  if (wifiLinkUp()) {
    Serial.printf("Connected: %s  IP %s\n", WiFi.SSID().c_str(),
                  WiFi.localIP().toString().c_str());
    return true;
  }

  if (connectSavedNetwork(true)) {
    Serial.printf("Connected: %s  IP %s\n", WiFi.SSID().c_str(),
                  WiFi.localIP().toString().c_str());
    return true;
  }

  if (wm.getWiFiIsSaved()) {
    Serial.println("Saved WiFi could not connect — opening setup portal");
  } else {
    Serial.println("No saved WiFi — opening setup portal");
  }

  const bool connected = wm.startConfigPortal(config::kPortalApName);

  if (connected && wifiLinkUp()) {
    Serial.printf("Connected: %s  IP %s\n", WiFi.SSID().c_str(),
                  WiFi.localIP().toString().c_str());
    return true;
  }

  Serial.println("WiFi connection failed");
  statusScreenConnectFailed();
  return false;
}
