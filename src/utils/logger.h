#pragma once

#include <Arduino.h>

namespace logger {

void begin(unsigned long baud_rate);
void info(const char* message);
void warn(const char* message);
void error(const char* message);
void infof(const char* format, ...);

}  // namespace logger
