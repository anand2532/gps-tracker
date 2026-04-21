#pragma once

#include <Arduino.h>

struct GpsData {
  float lat;
  float lon;
  float speed;
  int satellites;
  int signal;
  float battery;
  bool valid;
  String timestamp;
};

class GpsManager {
 public:
  void begin();
  void update();
  GpsData current() const;

 private:
  uint32_t last_update_ms_ = 0;
  uint32_t boot_ms_ = 0;
  GpsData data_{};

  static String formatTimestamp(uint32_t uptime_ms);
};
