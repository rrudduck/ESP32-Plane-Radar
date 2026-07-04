#include "services/ota_update.h"

#include <ArduinoOTA.h>
#include <WiFi.h>

#include "config.h"

namespace services::ota {

namespace {

bool s_initialized = false;
int s_last_progress_pct = -1;

void onOtaStart() {
  const char* kind = ArduinoOTA.getCommand() == U_FLASH ? "sketch" : "filesystem";
  Serial.printf("ota: start (%s)\n", kind);
}

void onOtaEnd() { Serial.println("ota: done"); }

void onOtaProgress(unsigned int progress, unsigned int total) {
  if (total == 0) {
    return;
  }
  const int pct = static_cast<int>((progress * 100U) / total);
  if (pct == s_last_progress_pct || (pct % 10 != 0 && pct != 99)) {
    return;
  }
  s_last_progress_pct = pct;
  Serial.printf("ota: %d%%\n", pct);
}

void onOtaError(ota_error_t error) {
  Serial.printf("ota: error[%u]\n", static_cast<unsigned>(error));
}

void ensureInitialized() {
  if (!config::kOtaEnabled || s_initialized || WiFi.status() != WL_CONNECTED) {
    return;
  }

  ArduinoOTA.setPort(config::kOtaPort);
  ArduinoOTA.setHostname(config::kOtaHostname);
  if (config::kOtaPassword[0] != '\0') {
    ArduinoOTA.setPassword(config::kOtaPassword);
  }

  ArduinoOTA.onStart(onOtaStart);
  ArduinoOTA.onEnd(onOtaEnd);
  ArduinoOTA.onProgress(onOtaProgress);
  ArduinoOTA.onError(onOtaError);

  ArduinoOTA.begin();
  s_initialized = true;
  Serial.printf("ota: ready at %s:%u\n", WiFi.localIP().toString().c_str(),
                static_cast<unsigned>(config::kOtaPort));
}

}  // namespace

void loop() {
  if (!config::kOtaEnabled) {
    return;
  }

  ensureInitialized();
  if (s_initialized && WiFi.status() == WL_CONNECTED) {
    ArduinoOTA.handle();
  }
}

}  // namespace services::ota
