#include <Arduino.h>
#include <LittleFS.h>

#include "gps/gps_manager.h"
#include "utils/logger.h"
#include "web/web_server.h"
#include "wifi/wifi_manager.h"

namespace {
constexpr const char* kApSsid = "GPS-Tracker-ESP32";
constexpr const char* kApPassword = "gpstracker123";

WifiManager wifiManager;
GpsManager gpsManager;
WebServerManager webServerManager(wifiManager, gpsManager);
}  // namespace

void setup() {
  logger::begin(115200);
  logger::info("Booting GPS tracker firmware...");

  if (!LittleFS.begin(true)) {
    logger::error("LittleFS mount failed");
  }

  gpsManager.begin();
  wifiManager.begin(kApSsid, kApPassword);
  webServerManager.begin();
}

void loop() {
  gpsManager.update();
  wifiManager.loop();
}
