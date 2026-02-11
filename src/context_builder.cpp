#include "context_builder.h"
#include "session_manager.h"
#include "tools.h"
#include "agents_md.h"

ContextBuilder::ContextBuilder()
  : maxHistory_(10) {
}

ContextBuilder& ContextBuilder::getInstance() {
  static ContextBuilder instance;
  return instance;
}

void ContextBuilder::setMaxHistory(int maxHistory) {
  maxHistory_ = maxHistory;
}

String ContextBuilder::buildSystemPrompt() {
  String prompt = "";
  
  const char* ptr = AGENTS_MD;
  int len = strlen_P(AGENTS_MD);
  for (int i = 0; i < len && prompt.length() < 1000; i++) {
    prompt += pgm_read_byte_near(ptr + i);
  }

  return prompt;
}

String ContextBuilder::getToolsDescription() {
  String desc = "\n\n## Available Tools\n\n";

  ToolRegistry& registry = ToolRegistry::getInstance();
  int toolCount = registry.getToolCount();

  StaticJsonDocument<1024> doc;
  JsonArray toolsArray = doc.to<JsonArray>();
  registry.getToolDefinitions(toolsArray);

  for (JsonVariant tool : toolsArray) {
    String name = tool["name"] | "";
    String description = tool["description"] | "";
    desc += "### " + name + "\n";
    desc += description + "\n\n";
  }

  return desc;
}

String ContextBuilder::build(const String& sessionId) {
  String context = buildSystemPrompt();
  context += getToolsDescription();

  Session* session = SessionManager::getInstance().getSession(sessionId);
  if (session && session->messageCount > 0) {
    context += "\n\n## Conversation History\n\n";
    
    int startIdx = 0;
    if (session->messageCount > maxHistory_) {
      startIdx = session->messageCount - maxHistory_;
    }

    for (int i = startIdx; i < session->messageCount; i++) {
      context += session->messages[i].role + ": " + session->messages[i].content + "\n\n";
    }
  }

  return context;
}
