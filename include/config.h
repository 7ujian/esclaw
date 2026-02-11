#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <ArduinoJson.h>

struct WiFiConfig {
  String ssid;
  String password;
  int timeout;
};

struct AgentConfig {
  String model;
  int maxTokens;
  float temperature;
  int maxToolIterations;
  int maxHistoryLength;
};

struct ProviderConfig {
  String apiKey;
  String apiBase;
};

struct ProvidersConfig {
  ProviderConfig openai;
  ProviderConfig anthropic;
  ProviderConfig openrouter;
  ProviderConfig zhipu;
  ProviderConfig groq;
};

struct WebServerConfig {
  bool enabled;
  int port;
};

struct LoggingConfig {
  int level;
};

class Config {
public:
  static Config& getInstance();

  bool load();
  bool save();
  void resetToDefaults();

  WiFiConfig& getWiFiConfig();
  AgentConfig& getAgentConfig();
  ProvidersConfig& getProvidersConfig();
  WebServerConfig& getWebServerConfig();
  LoggingConfig& getLoggingConfig();

  String getProviderKey(const String& provider);
  String getProviderBase(const String& provider);

private:
  Config();
  Config(const Config&) = delete;
  Config& operator=(const Config&) = delete;

  WiFiConfig wifiConfig_;
  AgentConfig agentConfig_;
  ProvidersConfig providersConfig_;
  WebServerConfig webServerConfig_;

  bool loadFromJson(const JsonObject& json);
  void saveToJson(JsonObject& json);
  void setDefaults();
};

#endif
