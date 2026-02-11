#include "llm_provider.h"
#include <ArduinoHttpClient.h>
#include <WiFiClientSecure.h>

HTTPProvider::HTTPProvider(const String& apiKey, const String& apiBase)
    : apiKey_(apiKey), apiBase_(apiBase) {
}

LLMResponse HTTPProvider::chat(Message* messages, int messageCount,
                               JsonVariantConst* tools, int toolCount,
                               String model,
                               int maxTokens,
                               float temperature) {
  LLMResponse response;
  response.content = "";
  response.toolCallCount = 0;
  response.finishReason = "unknown";
  response.promptTokens = 0;
  response.completionTokens = 0;
  response.totalTokens = 0;

  if (apiBase_.isEmpty()) {
    response.finishReason = "error: no API base configured";
    return response;
  }

  WiFiClientSecure client;
  client.setInsecure();

  String host = apiBase_.substring(8);
  int slashIndex = host.indexOf('/');
  if (slashIndex > 0) {
    host = host.substring(0, slashIndex);
  }

  String path = apiBase_.substring(apiBase_.indexOf('/', 8));
  path += "/chat/completions";

  HttpClient http(client, host, 443);

  bool isZhipuAPI = apiBase_.indexOf("bigmodel.cn") > 0;

  DynamicJsonDocument doc(2048);
  JsonObject root = doc.to<JsonObject>();

  root["model"] = model;
  JsonArray msgs = root.createNestedArray("messages");

  ::Serial.println("Building request for model: " + model);
  ::Serial.println("Is Zhipu API: " + String(isZhipuAPI ? "yes" : "no"));

  for (int i = 0; i < messageCount; i++) {
    if (isZhipuAPI && messages[i].role == "system") {
      ::Serial.println("  Skipping system message for Zhipu API");
      continue;
    }

    JsonObject msgObj = msgs.createNestedObject();
    msgObj["role"] = messages[i].role;
    msgObj["content"] = messages[i].content;
    ::Serial.println("  [" + messages[i].role + "] " + messages[i].content.substring(0, 30) + "...");

    int tcCount = 0;
    for (int j = 0; j < MAX_TOOL_CALLS; j++) {
      if (messages[i].toolCalls[j].name.length() > 0) {
        tcCount++;
      } else {
        break;
      }
    }

    if (tcCount > 0) {
      JsonArray toolCalls = msgObj.createNestedArray("tool_calls");
      for (int j = 0; j < tcCount; j++) {
        const ToolCall& tc = messages[i].toolCalls[j];
        JsonObject tcObj = toolCalls.createNestedObject();
        tcObj["id"] = tc.id;
        tcObj["type"] = tc.type;

        JsonObject function = tcObj.createNestedObject("function");
        function["name"] = tc.name;
        function["arguments"] = tc.arguments;
      }
    } else if (messages[i].toolCallId.length() > 0) {
      msgObj["tool_call_id"] = messages[i].toolCallId;
    }
  }

  if (toolCount > 0 && !isZhipuAPI) {
    JsonArray toolsArr = root.createNestedArray("tools");
    for (int i = 0; i < toolCount; i++) {
      toolsArr.add(tools[i]);
    }
    root["tool_choice"] = "auto";
  } else if (isZhipuAPI && toolCount > 0) {
    ::Serial.println("Zhipu API detected, skipping tools (not supported)");
  }

  if (isZhipuAPI) {
    root["max_tokens"] = (maxTokens > 4096) ? 4096 : maxTokens;
    root["temperature"] = round(temperature * 10) / 10.0;
    ::Serial.println("Zhipu API: limited max_tokens to " + String(root["max_tokens"].as<int>()));
    ::Serial.println("Zhipu API: rounded temperature to " + String(root["temperature"].as<float>()));
  } else {
    root["max_tokens"] = maxTokens;
    root["temperature"] = temperature;
  }

  String requestBody;
  serializeJson(doc, requestBody);

  ::Serial.println("API Base: " + apiBase_);
  ::Serial.println("Host: " + host);
  ::Serial.println("Path: " + path);
  ::Serial.println("API Key: " + (apiKey_.isEmpty() ? "(empty)" : (apiKey_.substring(0, 8) + "..." + apiKey_.substring(apiKey_.length() - 4))));
  ::Serial.println();
  ::Serial.println("Sending request...");
  ::Serial.println("Request body:");
  ::Serial.println(requestBody);
  ::Serial.println();
  ::Serial.println("Request size: " + String(requestBody.length()) + " bytes");

  http.beginRequest();
  http.post(path);
  http.sendHeader("Content-Type", "application/json");
  if (apiKey_.length() > 0) {
    http.sendHeader("Authorization", "Bearer " + apiKey_);
  }
  http.sendHeader("Content-Length", String(requestBody.length()));
  http.beginBody();
  http.print(requestBody);
  http.endRequest();

  int statusCode = http.responseStatusCode();
  ::Serial.println("Status: " + String(statusCode));

  if (statusCode != 200) {
    String errorBody = http.responseBody();
    ::Serial.println("Error: " + errorBody);
    response.finishReason = "error: HTTP " + String(statusCode);
    http.stop();
    return response;
  }

  String responseBody = http.responseBody();
  ::Serial.println("Response: " + String(responseBody.length()) + " bytes");
  http.stop();

  DynamicJsonDocument respDoc(4096);
  DeserializationError error = deserializeJson(respDoc, responseBody);

  if (error) {
    ::Serial.println("JSON parse error: " + String(error.c_str()));
    response.finishReason = "error: JSON parse failed";
    return response;
  }

  if (!respDoc.containsKey("choices")) {
    response.finishReason = "error: no choices";
    return response;
  }

  JsonArray choices = respDoc["choices"];
  if (choices.size() == 0) {
    response.finishReason = "error: empty choices";
    return response;
  }

  JsonObject choice = choices[0];
  if (choice.containsKey("finish_reason")) {
    response.finishReason = choice["finish_reason"].as<String>();
  }

  if (choice.containsKey("message")) {
    JsonObject msg = choice["message"];
    if (msg.containsKey("content")) {
      response.content = msg["content"].as<String>();
    }

    if (msg.containsKey("tool_calls")) {
      JsonArray toolCalls = msg["tool_calls"];
      response.toolCallCount = 0;
      for (JsonVariant tcJson : toolCalls) {
        if (response.toolCallCount >= MAX_TOOL_CALLS) break;

        JsonObject tcObj = tcJson.as<JsonObject>();
        ToolCall& tc = response.toolCalls[response.toolCallCount];

        if (tcObj.containsKey("id")) {
          tc.id = tcObj["id"].as<String>();
        }

        if (tcObj.containsKey("type")) {
          tc.type = tcObj["type"].as<String>();
        } else {
          tc.type = "function";
        }

        if (tcObj.containsKey("function")) {
          JsonObject func = tcObj["function"];
          if (func.containsKey("name")) {
            tc.name = func["name"].as<String>();
          }
          if (func.containsKey("arguments")) {
            tc.arguments = func["arguments"].as<String>();
          }
        }

        response.toolCallCount++;
      }
    }
  }

  if (respDoc.containsKey("usage")) {
    JsonObject usage = respDoc["usage"];
    if (usage.containsKey("prompt_tokens")) {
      response.promptTokens = usage["prompt_tokens"].as<int>();
    }
    if (usage.containsKey("completion_tokens")) {
      response.completionTokens = usage["completion_tokens"].as<int>();
    }
    if (usage.containsKey("total_tokens")) {
      response.totalTokens = usage["total_tokens"].as<int>();
    }
  }

  return response;
}

String HTTPProvider::getDefaultModel() const {
  return "gpt-3.5-turbo";
}

LLMProvider* createProvider(String model, String openaiKey, String openaiBase,
                           String anthropicKey, String anthropicBase,
                           String openrouterKey, String openrouterBase,
                           String zhipuKey, String zhipuBase,
                           String groqKey, String groqBase) {
  String modelLower = model;
  modelLower.toLowerCase();

  String apiKey;
  String apiBase;

  if (modelLower.startsWith("openrouter/") || modelLower.startsWith("anthropic/") ||
      modelLower.startsWith("openai/") || modelLower.startsWith("meta-llama/") ||
      modelLower.startsWith("deepseek/") || modelLower.startsWith("google/")) {
    apiKey = openrouterKey;
    apiBase = (openrouterBase.length() > 0) ? openrouterBase : "https://openrouter.ai/api/v1";
  }
  else if (modelLower.indexOf("claude") >= 0 || modelLower.startsWith("anthropic/")) {
    apiKey = anthropicKey;
    apiBase = (anthropicBase.length() > 0) ? anthropicBase : "https://api.anthropic.com/v1";
  }
  else if (modelLower.indexOf("gpt") >= 0 || modelLower.startsWith("openai/")) {
    apiKey = openaiKey;
    apiBase = (openaiBase.length() > 0) ? openaiBase : "https://api.openai.com/v1";
  }
  else if (modelLower.indexOf("glm") >= 0 || modelLower.indexOf("zhipu") >= 0 ||
           modelLower.indexOf("zai") >= 0) {
    apiKey = zhipuKey;
    apiBase = (zhipuBase.length() > 0) ? zhipuBase : "https://open.bigmodel.cn/api/paas/v4";
  }
  else if (modelLower.indexOf("groq") >= 0 || modelLower.startsWith("groq/")) {
    apiKey = groqKey;
    apiBase = (groqBase.length() > 0) ? groqBase : "https://api.groq.com/openai/v1";
  }
  else {
    apiKey = openrouterKey;
    apiBase = (openrouterBase.length() > 0) ? openrouterBase : "https://openrouter.ai/api/v1";
  }

  if (apiKey.length() == 0) {
    return nullptr;
  }

  return new HTTPProvider(apiKey, apiBase);
}
