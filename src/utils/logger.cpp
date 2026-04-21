#include "logger.h"

#include <cstdarg>
#include <cstdio>

namespace logger {
namespace {
void logWithLevel(const char* level, const char* message) {
  Serial.print("[");
  Serial.print(level);
  Serial.print("] ");
  Serial.println(message);
}
}  // namespace

void begin(unsigned long baud_rate) {
  Serial.begin(baud_rate);
  delay(50);
}

void info(const char* message) { logWithLevel("INFO", message); }

void warn(const char* message) { logWithLevel("WARN", message); }

void error(const char* message) { logWithLevel("ERROR", message); }

void infof(const char* format, ...) {
  char buffer[192];
  va_list args;
  va_start(args, format);
  vsnprintf(buffer, sizeof(buffer), format, args);
  va_end(args);
  info(buffer);
}

}  // namespace logger
