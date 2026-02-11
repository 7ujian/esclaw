#include "config.h"
#include "logger.h"
#include <LittleFS.h>
#include <ArduinoJson.h>

const char CONFIG_PATH[] = "/config/config.json";

Config::Config() {
  setDefaults();
}

Config& Config::getInstance() {
  static Config instance;
  return instance;
}

void Config::setDefaults() {
  wifiConfig_.ssid = "";
  wifiConfig_.password = "";
  wifiConfig_.timeout = 30;

  agentConfig_.model = "gpt-3.5-turbo";
  agentConfig_.maxTokens = 2048;
  agentConfig_.temperature = 0.7;
  agentConfig_.maxToolIterations = 5;
  agentConfig_.maxHistoryLength = 10;

  providersConfig_.openai.apiKey = "";
  providersConfig_.openai.apiBase = "https://api.openai.com/v1";
  providersConfig_.anthropic.apiKey = "";
  providersConfig_.anthropic.apiBase = "https://api.anthropic.com/v1";
  providersConfig_.openrouter.apiKey = "";
  providersConfig_.openrouter.apiBase = "https://openrouter.ai/api/v1";
  providersConfig_.zhipu.apiKey = "";
  providersConfig_.zhipu.apiBase = "https://open.bigmodel.cn/api/paas/v4";
  providersConfig_.groq.apiKey = "";
  providersConfig_.groq.apiBase = "https://api.groq.com/openai/v1";

  webServerConfig_.enabled = false;
  webServerConfig_.port = 80;
}

bool Config::load() {
  if (!LittleFS.exists(CONFIG_PATH)) {
    LOG("Config file not found, creating default");
    setDefaults();
    return save();
  }

  File file = LittleFS.open(CONFIG_PATH, "r");
  if (!file) {
    return false;
  }

  size_t fileSize = file.size();
  LOG("Config file size: " + String(fileSize) + " bytes");

  if (fileSize > 1024) {
    LOG("Config file too large for buffer");
    file.close();
    return false;
  }

  StaticJsonDocument<1024> doc;
  DeserializationError error = deserializeJson(doc, file);
  file.close();

  if (error) {
    LOG("JSON parse error: " + String(error.c_str()));
    return false;
  }

  LOG("JSON parsed successfully");
  setDefaults();
  loadFromJson(doc.as<JsonObject>());
  LOG("Config loaded. SSID: " + wifiConfig_.ssid);
  return true;
}

bool Config::loadFromJson(const JsonObject& json) {
  if (json.containsKey("wifi")) {
    JsonObject wifi = json["wifi"].as<JsonObject>();
    if (wifi.containsKey("ssid")) wifiConfig_.ssid = wifi["ssid"].as<String>();
    if (wifi.containsKey("password")) wifiConfig_.password = wifi["password"].as<String>();
    if (wifi.containsKey("timeout")) wifiConfig_.timeout = wifi["timeout"].as<int>();
  }

  if (json.containsKey("agent")) {
    JsonObject agent = json["agent"].as<JsonObject>();
    if (agent.containsKey("model")) agentConfig_.model = agent["model"].as<String>();
    if (agent.containsKey("maxTokens")) agentConfig_.maxTokens = agent["maxTokens"].as<int>();
    if (agent.containsKey("temperature")) agentConfig_.temperature = agent["temperature"].as<float>();
    if (agent.containsKey("maxToolIterations")) agentConfig_.maxToolIterations = agent["maxToolIterations"].as<int>();
    if (agent.containsKey("maxHistoryLength")) agentConfig_.maxHistoryLength = agent["maxHistoryLength"].as<int>();
  }

  if (json.containsKey("providers")) {
    JsonObject providers = json["providers"].as<JsonObject>();

    if (providers.containsKey("openai")) {
      JsonObject openai = providers["openai"].as<JsonObject>();
      if (openai.containsKey("apiKey")) providersConfig_.openai.apiKey = openai["apiKey"].as<String>();
      if (openai.containsKey("apiBase")) providersConfig_.openai.apiBase = openai["apiBase"].as<String>();
    }

    if (providers.containsKey("anthropic")) {
      JsonObject anthropic = providers["anthropic"].as<JsonObject>();
      if (anthropic.containsKey("apiKey")) providersConfig_.anthropic.apiKey = anthropic["apiKey"].as<String>();
      if (anthropic.containsKey("apiBase")) providersConfig_.anthropic.apiBase = anthropic["apiBase"].as<String>();
    }

    if (providers.containsKey("openrouter")) {
      JsonObject openrouter = providers["openrouter"].as<JsonObject>();
      if (openrouter.containsKey("apiKey")) providersConfig_.openrouter.apiKey = openrouter["apiKey"].as<String>();
      if (openrouter.containsKey("apiBase")) providersConfig_.openrouter.apiBase = openrouter["apiBase"].as<String>();
    }

    if (providers.containsKey("zhipu")) {
      JsonObject zhipu = providers["zhipu"].as<JsonObject>();
      if (zhipu.containsKey("apiKey")) providersConfig_.zhipu.apiKey = zhipu["apiKey"].as<String>();
      if (zhipu.containsKey("apiBase")) providersConfig_.zhipu.apiBase = zhipu["apiBase"].as<String>();
    }

    if (providers.containsKey("groq")) {
      JsonObject groq = providers["groq"].as<JsonObject>();
      if (groq.containsKey("apiKey")) providersConfig_.groq.apiKey = groq["apiKey"].as<String>();
      if (groq.containsKey("apiBase")) providersConfig_.groq.apiBase = groq["apiBase"].as<String>();
    }
  }

  if (json.containsKey("webserver")) {
    JsonObject webserver = json["webserver"].as<JsonObject>();
    if (webserver.containsKey("enabled")) webServerConfig_.enabled = webserver["enabled"].as<bool>();
    if (webserver.containsKey("port")) webServerConfig_.port = webserver["port"].as<int>();
  }

  return true;
}

bool Config::save() {
  StaticJsonDocument<2048> doc;
  JsonObject json = doc.to<JsonObject>();
  saveToJson(json);

  LittleFS.mkdir("/config");

  File file = LittleFS.open(CONFIG_PATH, "w");
  if (!file) {
    return false;
  }

  serializeJsonPretty(doc, file);
  file.close();
  return true;
}

void Config::saveToJson(JsonObject& json) {
  JsonObject wifi = json.createNestedObject("wifi");
  wifi["ssid"] = wifiConfig_.ssid;
  wifi["password"] = wifiConfig_.password;
  wifi["timeout"] = wifiConfig_.timeout;

  JsonObject agent = json.createNestedObject("agent");
  agent["model"] = agentConfig_.model;
  agent["maxTokens"] = agentConfig_.maxTokens;
  agent["temperature"] = agentConfig_.temperature;
  agent["maxToolIterations"] = agentConfig_.maxToolIterations;
  agent["maxHistoryLength"] = agentConfig_.maxHistoryLength;

  JsonObject providers = json.createNestedObject("providers");

  JsonObject openai = providers.createNestedObject("openai");
  openai["apiKey"] = providersConfig_.openai.apiKey;
  openai["apiBase"] = providersConfig_.openai.apiBase;

  JsonObject anthropic = providers.createNestedObject("anthropic");
  anthropic["apiKey"] = providersConfig_.anthropic.apiKey;
  anthropic["apiBase"] = providersConfig_.anthropic.apiBase;

  JsonObject openrouter = providers.createNestedObject("openrouter");
  openrouter["apiKey"] = providersConfig_.openrouter.apiKey;
  openrouter["apiBase"] = providersConfig_.openrouter.apiBase;

  JsonObject zhipu = providers.createNestedObject("zhipu");
  zhipu["apiKey"] = providersConfig_.zhipu.apiKey;
  zhipu["apiBase"] = providersConfig_.zhipu.apiBase;

  JsonObject groq = providers.createNestedObject("groq");
  groq["apiKey"] = providersConfig_.groq.apiKey;
  groq["apiBase"] = providersConfig_.groq.apiBase;

  JsonObject webserver = json.createNestedObject("webserver");
  webserver["enabled"] = webServerConfig_.enabled;
  webserver["port"] = webServerConfig_.port;
}

void Config::resetToDefaults() {
  setDefaults();
  save();
}

WiFiConfig& Config::getWiFiConfig() {
  return wifiConfig_;
}

AgentConfig& Config::getAgentConfig() {
  return agentConfig_;
}

ProvidersConfig& Config::getProvidersConfig() {
  return providersConfig_;
}

WebServerConfig& Config::getWebServerConfig() {
  return webServerConfig_;
}

String Config::getProviderKey(const String& provider) {
  if (provider == "openai") return providersConfig_.openai.apiKey;
  if (provider == "anthropic") return providersConfig_.anthropic.apiKey;
  if (provider == "openrouter") return providersConfig_.openrouter.apiKey;
  if (provider == "zhipu") return providersConfig_.zhipu.apiKey;
  if (provider == "groq") return providersConfig_.groq.apiKey;
  return "";
}

String Config::getProviderBase(const String& provider) {
  if (provider == "openai") return providersConfig_.openai.apiBase;
  if (provider == "anthropic") return providersConfig_.anthropic.apiBase;
  if (provider == "openrouter") return providersConfig_.openrouter.apiBase;
  if (provider == "zhipu") return providersConfig_.zhipu.apiBase;
  if (provider == "groq") return providersConfig_.groq.apiBase;
  return "";
}
