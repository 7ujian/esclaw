#include <Arduino.h>
#include <LittleFS.h>
#include "config.h"
#include "wifi_manager.h"
#include "message_bus.h"
#include "serial_console.h"
#include "llm_provider.h"
#include "agent_loop.h"
#include "session_manager.h"
#include "tools.h"

void updateWiFiConfig(const String& ssid, const String& password) {
  Config::getInstance().getWiFiConfig().ssid = ssid;
  Config::getInstance().getWiFiConfig().password = password;
  Config::getInstance().save();
  WiFiManager::getInstance().configure(ssid, password);
  WiFiManager::getInstance().connect();
}

extern void registerAllTools();

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("🦞 ESPClaw - ESP32 AI Assistant");
  Serial.println("Initializing...");

  if (!LittleFS.begin()) {
    Serial.println("Failed to mount LittleFS, formatting...");
    LittleFS.format();
    LittleFS.begin();
  }

  Config::getInstance().load();

  Serial.println("WiFi SSID: " + Config::getInstance().getWiFiConfig().ssid);
  if (Config::getInstance().getWiFiConfig().password.isEmpty()) {
    Serial.println("WiFi Password: (empty)");
  } else {
    Serial.println("WiFi Password: ***");
  }
  Serial.println("WiFi Timeout: " + String(Config::getInstance().getWiFiConfig().timeout) + "s");

  WiFiManager::getInstance().configure(
    Config::getInstance().getWiFiConfig().ssid,
    Config::getInstance().getWiFiConfig().password
  );

  Serial.println("Attempting WiFi connection...");
  if (WiFiManager::getInstance().connect()) {
    Serial.println("WiFi connected: " + WiFiManager::getInstance().getIP());
  } else {
    Serial.println("WiFi connection failed. Status: " + String(WiFi.status()));
    Serial.println("Configure with /config");
  }

  auto& cfg = Config::getInstance();
  auto& providers = cfg.getProvidersConfig();

  LLMProvider* llmProvider = createProvider(
    cfg.getAgentConfig().model,
    providers.openai.apiKey, providers.openai.apiBase,
    providers.anthropic.apiKey, providers.anthropic.apiBase,
    providers.openrouter.apiKey, providers.openrouter.apiBase,
    providers.zhipu.apiKey, providers.zhipu.apiBase,
    providers.groq.apiKey, providers.groq.apiBase
  );

  if (llmProvider == nullptr) {
    Serial.println("Warning: No LLM provider configured. Set API key via /config");
  } else {
    Serial.println("LLM provider initialized for: " + cfg.getAgentConfig().model);
  }

  registerAllTools();
  Serial.println("Tools registered: " + String(ToolRegistry::getInstance().getToolCount()));

  Serial.println("Initializing session manager...");
  SessionManager::getInstance().loadAllSessions();
  Serial.println("Loaded " + String(SessionManager::getInstance().getSessionCount()) + " session(s)");

  AgentLoop::getInstance().setLLMProvider(llmProvider);
  Serial.println("Agent loop initialized");

  SerialConsole::getInstance().begin();

  Serial.println("Ready!");
  Serial.println();
  Serial.print("🦞 ");
}

void loop() {
  WiFiManager::getInstance().update();
  SerialConsole::getInstance().update();
  AgentLoop::getInstance().process();

  delay(10);
}
