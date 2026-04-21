#include "gps_manager.h"

#include <math.h>

#include "utils/logger.h"

namespace {
constexpr uint32_t kUpdateIntervalMs = 1000;
// Central Delhi (Connaught Place area) — dummy track orbits this point slightly.
constexpr float kOriginLat = 28.6139f;
constexpr float kOriginLon = 77.2090f;
}  // namespace

void GpsManager::begin() {
  boot_ms_ = millis();
  // Seed full state so /api/gps returns usable JSON before the first loop() tick.
  data_.lat = kOriginLat;
  data_.lon = kOriginLon;
  data_.speed = 18.5f;
  data_.satellites = 9;
  data_.signal = 78;
  data_.battery = 4.05f;
  data_.valid = true;
  data_.timestamp = formatTimestamp(0);
  logger::info("GPS manager initialized (simulated Delhi dummy data)");
}

void GpsManager::update() {
  const uint32_t now = millis();
  if (now - last_update_ms_ < kUpdateIntervalMs) {
    return;
  }
  last_update_ms_ = now;

  const float t = static_cast<float>(now - boot_ms_) / 1000.0f;

  // Always-on dummy fix: small movement around Delhi so the UI always receives lat/lon.
  data_.valid = true;
  data_.timestamp = formatTimestamp(now - boot_ms_);

  data_.lat = kOriginLat + 0.0020f * sinf(t * 0.12f);
  data_.lon = kOriginLon + 0.0025f * cosf(t * 0.09f);
  data_.speed = 20.0f + 8.0f * sinf(t * 0.18f);
  data_.satellites = 8 + static_cast<int>(3.0f + 2.0f * sinf(t * 0.25f));
  if (data_.satellites > 14) {
    data_.satellites = 14;
  }
  data_.signal = 55 + static_cast<int>(30.0f * sinf(t * 0.15f));
  if (data_.signal > 99) {
    data_.signal = 99;
  }
  if (data_.signal < 40) {
    data_.signal = 40;
  }

  const float drain = static_cast<float>(now - boot_ms_) * 0.00000004f;
  data_.battery = max(3.6f, 4.08f - drain);
}

GpsData GpsManager::current() const { return data_; }

String GpsManager::formatTimestamp(uint32_t uptime_ms) {
  const uint32_t total_seconds = uptime_ms / 1000;
  const uint32_t hours = total_seconds / 3600;
  const uint32_t minutes = (total_seconds % 3600) / 60;
  const uint32_t seconds = total_seconds % 60;

  char buffer[24];
  snprintf(buffer, sizeof(buffer), "%02lu:%02lu:%02lu", hours, minutes, seconds);
  return String(buffer);
}
