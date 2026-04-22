#pragma once

#include <ESPAsyncWebServer.h>

#include "gps/gps_manager.h"
#include "wifi/wifi_manager.h"

class WebServerManager {
 public:
  WebServerManager(WifiManager& wifi_manager, GpsManager& gps_manager);
  void begin();

 private:
  AsyncWebServer server_;
  WifiManager& wifi_manager_;
  GpsManager& gps_manager_;

  void registerCaptivePortalRoutes();
  void registerApiRoutes();
  void registerStaticRoutes();
  void redirectToPortal(AsyncWebServerRequest* request);
  void sendFileOrFallback(AsyncWebServerRequest* request, const char* path,
                          const char* content_type);
};
