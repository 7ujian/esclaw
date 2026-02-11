#ifndef LOGGER_H
#define LOGGER_H

#include <Arduino.h>

enum LogLevel {
  LOG_LEVEL_NONE = 0,
  LOG_LEVEL_BASIC = 1,
  LOG_LEVEL_VERBOSE = 2
};

class Logger {
public:
  static Logger& getInstance();
  
  void setLevel(LogLevel level);
  LogLevel getLevel() const;
  
  void basic(const String& message);
  void verbose(const String& message);
  
  void println(const String& message);
  void print(const String& message);

private:
  Logger();
  LogLevel level_;
};

#define LOG_BASIC(msg) Logger::getInstance().basic(msg)
#define LOG_VERBOSE(msg) Logger::getInstance().verbose(msg)
#define LOG(msg) Logger::getInstance().println(msg)

#endif
