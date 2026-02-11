#include "agent_loop.h"
#include "tools.h"
#include "message_bus.h"
#include "logger.h"

AgentLoop::AgentLoop()
  : llmProvider_(nullptr) {
}

AgentLoop& AgentLoop::getInstance() {
  static AgentLoop instance;
  return instance;
}

void AgentLoop::setLLMProvider(LLMProvider* provider) {
  llmProvider_ = provider;
}

void AgentLoop::process() {
  MessageBus& bus = MessageBus::getInstance();

  while (bus.hasInbound()) {
    InboundMessage msg = bus.consumeInbound();
    processMessage(msg);
  }
}

bool AgentLoop::processMessage(const InboundMessage& msg) {
  if (!llmProvider_) {
    OutboundMessage outMsg;
    outMsg.channel = msg.channel;
    outMsg.content = "Error: No LLM provider configured";
    outMsg.timestamp = millis();
    MessageBus::getInstance().publishOutbound(outMsg);
    return false;
  }

  Session* session = SessionManager::getInstance().getOrCreateSession(msg.channel, msg.senderId);

  Config& cfg = Config::getInstance();
  int maxIterations = cfg.getAgentConfig().maxToolIterations;

  Message llmMessages[2];
  llmMessages[0].role = "system";
  llmMessages[0].content = ContextBuilder::getInstance().buildSystemPrompt();
  llmMessages[1].role = "user";
  llmMessages[1].content = msg.content;

  for (int iteration = 0; iteration < maxIterations; iteration++) {
    LOG_VERBOSE("=== Starting LLM call (iteration " + String(iteration) + ") ===");
    
    LLMResponse response = llmProvider_->chat(
      llmMessages, 2,
      nullptr, 0,
      cfg.getAgentConfig().model,
      cfg.getAgentConfig().maxTokens,
      cfg.getAgentConfig().temperature
    );

    LOG_VERBOSE("=== LLM call returned ===");
    LOG_VERBOSE("Finish reason: " + response.finishReason);

    if (response.finishReason.startsWith("error")) {
      OutboundMessage outMsg;
      outMsg.channel = msg.channel;
      outMsg.content = "Error: " + response.finishReason;
      outMsg.timestamp = millis();
      MessageBus::getInstance().publishOutbound(outMsg);
      return false;
    }

    if (response.toolCallCount == 0) {
      LOG_VERBOSE("No tool calls, saving response...");
      
      SessionManager::getInstance().addMessage(session->sessionId, "user", msg.content);
      SessionManager::getInstance().addMessage(session->sessionId, "assistant", response.content);
      
      OutboundMessage outMsg;
      outMsg.channel = msg.channel;
      outMsg.content = response.content;
      outMsg.timestamp = millis();
      MessageBus::getInstance().publishOutbound(outMsg);
      
      Serial.println();
      Serial.println(response.content);
      Serial.println();
      
      SessionManager::getInstance().saveSession(session->sessionId);
      
      return true;
    }

    llmMessages[0].role = "assistant";
    llmMessages[0].content = response.content;
  }

  OutboundMessage outMsg;
  outMsg.channel = msg.channel;
  outMsg.content = "Error: Max tool iterations reached";
  outMsg.timestamp = millis();
  MessageBus::getInstance().publishOutbound(outMsg);
  return false;
}
