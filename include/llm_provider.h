#ifndef LLM_PROVIDER_H
#define LLM_PROVIDER_H

#include <Arduino.h>
#include <ArduinoJson.h>

#define MAX_TOOL_CALLS 5
#define MAX_MESSAGES 10

struct ToolCall {
  String id;
  String type;
  String name;
  String arguments;
};

struct Message {
  String role;
  String content;
  ToolCall toolCalls[MAX_TOOL_CALLS];
  String toolCallId;
};

struct LLMResponse {
  String content;
  ToolCall toolCalls[MAX_TOOL_CALLS];
  int toolCallCount;
  String finishReason;
  int promptTokens;
  int completionTokens;
  int totalTokens;
};

class LLMProvider {
public:
  virtual ~LLMProvider() {}

  virtual LLMResponse chat(Message* messages, int messageCount,
                         JsonVariantConst* tools, int toolCount,
                         String model,
                         int maxTokens = 2048,
                         float temperature = 0.7) = 0;

  virtual String getDefaultModel() const = 0;
};

class HTTPProvider : public LLMProvider {
public:
  HTTPProvider(const String& apiKey, const String& apiBase);

  LLMResponse chat(Message* messages, int messageCount,
                 JsonVariantConst* tools, int toolCount,
                 String model,
                 int maxTokens = 2048,
                 float temperature = 0.7) override;

  String getDefaultModel() const override;

private:
  String apiKey_;
  String apiBase_;
};

LLMProvider* createProvider(String model, String openaiKey, String openaiBase,
                           String anthropicKey, String anthropicBase,
                           String openrouterKey, String openrouterBase,
                           String zhipuKey, String zhipuBase,
                           String groqKey, String groqBase);

#endif
