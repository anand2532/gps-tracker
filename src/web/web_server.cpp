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
  server_.on("/", HTTP_GET, [this](AsyncWebServerRequest* request) {
    sendFileOrFallback(request, "/index.html", "text/html; charset=utf-8");
  });

  server_.on("/index.html", HTTP_GET, [this](AsyncWebServerRequest* request) {
    sendFileOrFallback(request, "/index.html", "text/html; charset=utf-8");
  });

  server_.on("/style.css", HTTP_GET, [this](AsyncWebServerRequest* request) {
    sendFileOrFallback(request, "/style.css", "text/css; charset=utf-8");
  });

  server_.on("/app.js", HTTP_GET, [this](AsyncWebServerRequest* request) {
    sendFileOrFallback(request, "/app.js", "application/javascript; charset=utf-8");
  });

  server_.serveStatic("/vendor/", LittleFS, "/vendor/")
      .setCacheControl("public, max-age=604800, immutable");
  server_.serveStatic("/tiles/", LittleFS, "/tiles/")
      .setCacheControl("public, max-age=604800, immutable");

  // Reduce noisy "favicon missing" file lookups in logs.
  server_.on("/favicon.ico", HTTP_GET,
             [](AsyncWebServerRequest* request) { request->send(204); });
}

void WebServerManager::redirectToPortal(AsyncWebServerRequest* request) {
  const String location = "http://" + wifi_manager_.apIp().toString() + "/";
  AsyncWebServerResponse* response = request->beginResponse(302, "text/plain", "");
  response->addHeader("Location", location);
  response->addHeader("Cache-Control", "no-store");
  request->send(response);
}

void WebServerManager::sendFileOrFallback(AsyncWebServerRequest* request,
                                          const char* path,
                                          const char* content_type) {
  if (LittleFS.exists(path)) {
    AsyncWebServerResponse* response = request->beginResponse(LittleFS, path, content_type);
    response->addHeader("Cache-Control", "no-store, max-age=0");
    request->send(response);
    return;
  }

  logger::error("LittleFS asset missing; upload filesystem with pio run -t uploadfs");
  const char* html =
      "<!doctype html><html><head><meta charset='utf-8'><meta name='viewport' "
      "content='width=device-width,initial-scale=1'><title>Asset Missing</title></head>"
      "<body style='font-family:Arial;padding:16px;background:#111;color:#eee'>"
      "<h2>Web assets missing on device</h2>"
      "<p>Run:<br><code>pio run -t uploadfs</code></p>"
      "<p>Then reload <code>http://192.168.4.1/</code>.</p>"
      "</body></html>";
  request->send(500, "text/html; charset=utf-8", html);
}
