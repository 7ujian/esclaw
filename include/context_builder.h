#ifndef CONTEXT_BUILDER_H
#define CONTEXT_BUILDER_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include "session_manager.h"
#include "tools.h"
#include "agents_md.h"

#define CONTEXT_BUFFER_SIZE 2048

class ContextBuilder {
public:
  static ContextBuilder& getInstance();

  String build(const String& sessionId);
  String buildSystemPrompt();
  String getToolsDescription();

  void setMaxHistory(int maxHistory);

private:
  ContextBuilder();
  ContextBuilder(const ContextBuilder&) = delete;
  ContextBuilder& operator=(const ContextBuilder&) = delete;

  int maxHistory_;
};

#endif
