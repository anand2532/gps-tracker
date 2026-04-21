#include "web_server.h"

#include <ArduinoJson.h>
#include <LittleFS.h>

#include "utils/logger.h"

WebServerManager::WebServerManager(WifiManager& wifi_manager, GpsManager& gps_manager)
    : server_(80), wifi_manager_(wifi_manager), gps_manager_(gps_manager) {}

void WebServerManager::begin() {
  registerApiRoutes();
  registerCaptivePortalRoutes();
  registerStaticRoutes();

  server_.onNotFound([this](AsyncWebServerRequest* request) {
    if (wifi_manager_.isCaptivePortalHost(request->host())) {
      redirectToPortal(request);
      return;
    }
    request->send(404, "text/plain", "Not found");
  });

  server_.begin();
  logger::info("Async web server started on port 80");
}

void WebServerManager::registerApiRoutes() {
  server_.on("/api/gps", HTTP_GET, [this](AsyncWebServerRequest* request) {
    const GpsData data = gps_manager_.current();

    JsonDocument doc;
    doc["lat"] = data.lat;
    doc["lon"] = data.lon;
    doc["speed"] = data.speed;
    doc["satellites"] = data.satellites;
    doc["timestamp"] = data.timestamp;
    doc["signal"] = data.signal;
    doc["battery"] = data.battery;
    doc["valid"] = data.valid;

    String payload;
    serializeJson(doc, payload);

    AsyncWebServerResponse* response =
        request->beginResponse(200, "application/json", payload);
    response->addHeader("Cache-Control", "no-store, no-cache, must-revalidate");
    request->send(response);
  });
}

void WebServerManager::registerCaptivePortalRoutes() {
  const auto portalHandler = [this](AsyncWebServerRequest* request) {
    redirectToPortal(request);
  };

  server_.on("/generate_204", HTTP_GET, portalHandler);
  server_.on("/gen_204", HTTP_GET, portalHandler);
  server_.on("/hotspot-detect.html", HTTP_GET, portalHandler);
  server_.on("/fwlink", HTTP_GET, portalHandler);
  server_.on("/connecttest.txt", HTTP_GET, portalHandler);
  server_.on("/ncsi.txt", HTTP_GET, portalHandler);
}

void WebServerManager::registerStaticRoutes() {
  server_.serveStatic("/", LittleFS, "/").setDefaultFile("index.html");
}

void WebServerManager::redirectToPortal(AsyncWebServerRequest* request) {
  const String location = "http://" + wifi_manager_.apIp().toString() + "/";
  AsyncWebServerResponse* response = request->beginResponse(302, "text/plain", "");
  response->addHeader("Location", location);
  response->addHeader("Cache-Control", "no-store");
  request->send(response);
}
