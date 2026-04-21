#pragma once

#include <DNSServer.h>
#include <IPAddress.h>

class WifiManager {
 public:
  void begin(const char* ssid, const char* password);
  void loop();

  IPAddress apIp() const;
  bool isCaptivePortalHost(const String& host) const;

 private:
  DNSServer dns_server_;
  IPAddress ap_ip_;
};
