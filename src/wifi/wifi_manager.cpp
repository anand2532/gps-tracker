#include "wifi_manager.h"

#include <WiFi.h>

#include "utils/logger.h"

namespace {
constexpr byte kDnsPort = 53;
}

void WifiManager::begin(const char* ssid, const char* password) {
  WiFi.mode(WIFI_AP);
  WiFi.setSleep(false);

  const bool started = WiFi.softAP(ssid, password);
  ap_ip_ = WiFi.softAPIP();

  if (!started) {
    logger::error("Failed to start SoftAP");
    return;
  }

  dns_server_.start(kDnsPort, "*", ap_ip_);
  logger::infof("SoftAP started: %s", ssid);
  logger::infof("AP IP: %s", ap_ip_.toString().c_str());
}

void WifiManager::loop() { dns_server_.processNextRequest(); }

IPAddress WifiManager::apIp() const { return ap_ip_; }

bool WifiManager::isCaptivePortalHost(const String& host) const {
  if (host.isEmpty()) {
    return false;
  }

  const String ip = ap_ip_.toString();
  if (host == ip) {
    return false;
  }

  return host.indexOf(ip) < 0;
}
