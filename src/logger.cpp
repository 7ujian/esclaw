#include "logger.h"

Logger::Logger() : level_(LOG_LEVEL_BASIC) {
}

Logger& Logger::getInstance() {
  static Logger instance;
  return instance;
}

void Logger::setLevel(LogLevel level) {
  level_ = level;
}

LogLevel Logger::getLevel() const {
  return level_;
}

void Logger::basic(const String& message) {
  if (level_ >= LOG_LEVEL_BASIC) {
    Serial.println(message);
  }
}

void Logger::verbose(const String& message) {
  if (level_ >= LOG_LEVEL_VERBOSE) {
    Serial.println(message);
  }
}

void Logger::println(const String& message) {
  basic(message);
}

void Logger::print(const String& message) {
  if (level_ >= LOG_LEVEL_BASIC) {
    Serial.print(message);
  }
}
