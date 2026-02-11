#ifndef AGENT_LOOP_H
#define AGENT_LOOP_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include "llm_provider.h"
#include "context_builder.h"
#include "session_manager.h"
#include "tools.h"
#include "message_bus.h"
#include "config.h"

#define MAX_ITERATIONS 5

class AgentLoop {
public:
  static AgentLoop& getInstance();

  void setLLMProvider(LLMProvider* provider);
  void process();

  bool processMessage(const InboundMessage& msg);

private:
  AgentLoop();
  AgentLoop(const AgentLoop&) = delete;
  AgentLoop& operator=(const AgentLoop&) = delete;

  void executeToolCall(const ToolCall& call, Session* session);
  String buildLLMMessages(const String& sessionId, const String& userMessage);

  LLMProvider* llmProvider_;
};

#endif
