#include "agent_loop.h"
#include "tools.h"
#include "message_bus.h"

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
    ::Serial.println("=== Starting LLM call (iteration " + String(iteration) + ") ===");
    
    LLMResponse response = llmProvider_->chat(
      llmMessages, 2,
      nullptr, 0,
      cfg.getAgentConfig().model,
      cfg.getAgentConfig().maxTokens,
      cfg.getAgentConfig().temperature
    );

    ::Serial.println("=== LLM call returned ===");
    ::Serial.println("Finish reason: " + response.finishReason);
    ::Serial.println("Content length: " + String(response.content.length()));
    ::Serial.println("Tool calls: " + String(response.toolCallCount));

    if (response.finishReason.startsWith("error")) {
      ::Serial.println("Error from LLM, publishing error message...");
      OutboundMessage outMsg;
      outMsg.channel = msg.channel;
      outMsg.content = "Error: " + response.finishReason;
      outMsg.timestamp = millis();
      MessageBus::getInstance().publishOutbound(outMsg);
      ::Serial.println("Error message published, returning false");
      return false;
    }

    if (response.toolCallCount == 0) {
      ::Serial.println("No tool calls, saving response...");
      
      ::Serial.println("Adding user message to session...");
      SessionManager::getInstance().addMessage(session->sessionId, "user", msg.content);
      ::Serial.println("User message added");
      
      ::Serial.println("Adding assistant message to session...");
      SessionManager::getInstance().addMessage(session->sessionId, "assistant", response.content);
      ::Serial.println("Assistant message added");
      
      ::Serial.println("Creating outbound message...");
      OutboundMessage outMsg;
      outMsg.channel = msg.channel;
      outMsg.content = response.content;
      outMsg.timestamp = millis();
      ::Serial.println("Outbound message created, publishing...");
      MessageBus::getInstance().publishOutbound(outMsg);
      ::Serial.println("Outbound message published");
      
      ::Serial.println("Saving session...");
      SessionManager::getInstance().saveSession(session->sessionId);
      ::Serial.println("Session saved");
      
      ::Serial.println("=== Response processed successfully ===");
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
