#include "serial_console.h"
#include "config.h"
#include "wifi_manager.h"
#include "message_bus.h"
#include "logger.h"
#include <Arduino.h>
#include <LittleFS.h>

SerialConsole::SerialConsole() : inputReady_(false) {
}

SerialConsole& SerialConsole::getInstance() {
  static SerialConsole instance;
  return instance;
}

void SerialConsole::begin() {
  Serial.begin(115200);
  Serial.println();
  Serial.println("🦞 ESPClaw v0.1.0");
  Serial.println("Type /help for available commands");
  Serial.println();
}

bool SerialConsole::hasInput() const {
  return inputReady_;
}

String SerialConsole::getInput() {
  String input = inputBuffer_;
  inputBuffer_ = "";
  inputReady_ = false;
  return input;
}

void SerialConsole::print(const String& message) {
  Serial.print(message);
}

void SerialConsole::println(const String& message) {
  Serial.println(message);
}

void SerialConsole::update() {
  while (Serial.available()) {
    int c = Serial.read();
    
    if (c == '\r' || c == '\n') {
      if (inputBuffer_.length() > 0) {
        inputReady_ = true;
        processCommand(inputBuffer_);
        inputBuffer_ = "";
        inputReady_ = false;
      }
    } else if (c == '\b' || c == 127) {
      if (inputBuffer_.length() > 0) {
        inputBuffer_.remove(inputBuffer_.length() - 1);
        Serial.print("\b \b");
      }
    } else if (c >= 0 && c <= 255) {
      inputBuffer_ += (char)c;
      Serial.write((char)c);
    }
  }
}

String maskApiKey(const String& key) {
  if (key.length() <= 8) return "***";
  return key.substring(0, 8) + "..." + key.substring(key.length() - 4);
}

void SerialConsole::showCurrentConfig() {
  Config& config = Config::getInstance();
  
  Serial.println("Current Configuration:");
  Serial.println("=====================");
  Serial.println();
  
  Serial.println("[WiFi]");
  Serial.println("  SSID: " + config.getWiFiConfig().ssid);
  if (config.getWiFiConfig().password.isEmpty()) {
    Serial.println("  Password: ");
  } else {
    Serial.println("  Password: ***");
  }
  Serial.println("  Timeout: " + String(config.getWiFiConfig().timeout) + "s");
  Serial.println();
  
  Serial.println("[Agent]");
  Serial.println("  Model: " + config.getAgentConfig().model);
  Serial.println("  Max Tokens: " + String(config.getAgentConfig().maxTokens));
  Serial.println("  Temperature: " + String(config.getAgentConfig().temperature));
  Serial.println("  Max Tool Iterations: " + String(config.getAgentConfig().maxToolIterations));
  Serial.println("  Max History Length: " + String(config.getAgentConfig().maxHistoryLength));
  Serial.println();
  
  Serial.println("[Providers]");
  Serial.println("  OpenAI API Key: " + maskApiKey(config.getProvidersConfig().openai.apiKey));
  Serial.println("  OpenAI Base: " + config.getProvidersConfig().openai.apiBase);
  Serial.println();
  Serial.println("  Anthropic API Key: " + maskApiKey(config.getProvidersConfig().anthropic.apiKey));
  Serial.println("  Anthropic Base: " + config.getProvidersConfig().anthropic.apiBase);
  Serial.println();
  Serial.println("  OpenRouter API Key: " + maskApiKey(config.getProvidersConfig().openrouter.apiKey));
  Serial.println("  OpenRouter Base: " + config.getProvidersConfig().openrouter.apiBase);
  Serial.println();
  Serial.println("  Zhipu API Key: " + maskApiKey(config.getProvidersConfig().zhipu.apiKey));
  Serial.println("  Zhipu Base: " + config.getProvidersConfig().zhipu.apiBase);
  Serial.println();
  Serial.println("  Groq API Key: " + maskApiKey(config.getProvidersConfig().groq.apiKey));
  Serial.println("  Groq Base: " + config.getProvidersConfig().groq.apiBase);
  Serial.println();
  
  Serial.println("[Web Server]");
  Serial.println("  Enabled: " + String(config.getWebServerConfig().enabled ? "Yes" : "No"));
  Serial.println("  Port: " + String(config.getWebServerConfig().port));
}

void SerialConsole::showConfigHelp() {
  Serial.println("Configuration Keys:");
  Serial.println("------------------");
  Serial.println("WiFi:");
  Serial.println("  wifi.ssid <value>       Set WiFi SSID");
  Serial.println("  wifi.password <value>    Set WiFi password");
  Serial.println();
  Serial.println("Agent:");
  Serial.println("  agent.model <value>       Set AI model (e.g., gpt-3.5-turbo, glm-4)");
  Serial.println();
  Serial.println("LLM Providers:");
  Serial.println("  providers.openai.apiKey <key>        Set OpenAI API key");
  Serial.println("  providers.anthropic.apiKey <key>     Set Anthropic API key");
  Serial.println("  providers.openrouter.apiKey <key>    Set OpenRouter API key");
  Serial.println("  providers.zhipu.apiKey <key>       Set Zhipu AI API key");
  Serial.println("  providers.groq.apiKey <key>        Set Groq API key");
  Serial.println();
  Serial.println("Logging:");
  Serial.println("  log.level <0-2>          Set log level (0=none, 1=basic, 2=verbose)");
  Serial.println();
  Serial.println("Usage: /config <key> <value>");
  Serial.println("Example: /config providers.zhipu.apiKey sk-xxxxx");
}

void SerialConsole::handleConfig(const String& args) {
  int spaceIndex = args.indexOf(' ');

  if (spaceIndex <= 0) {
    showConfigHelp();
    return;
  }

  String key = args.substring(0, spaceIndex);
  String value = args.substring(spaceIndex + 1);

  Config& config = Config::getInstance();

  if (key == "wifi.ssid") {
    config.getWiFiConfig().ssid = value;
    Serial.println("WiFi SSID set to: " + value);
  } else if (key == "wifi.password") {
    config.getWiFiConfig().password = value;
    Serial.println("WiFi password updated");
  } else if (key == "agent.model") {
    config.getAgentConfig().model = value;
    Serial.println("Model set to: " + value);
  } else if (key == "log.level") {
    int level = value.toInt();
    if (level >= 0 && level <= 2) {
      Logger::getInstance().setLevel((LogLevel)level);
      Serial.println("Log level set to: " + String(level) + " (0=none, 1=basic, 2=verbose)");
    } else {
      Serial.println("Invalid log level. Use 0, 1, or 2");
      return;
    }
  } else if (key.startsWith("providers.")) {
    String provider = key.substring(10);
    int dotIndex = provider.indexOf('.');
    if (dotIndex > 0) {
      String providerName = provider.substring(0, dotIndex);
      String providerKey = provider.substring(dotIndex + 1);

      if (providerKey == "apiKey") {
        if (providerName == "openai") config.getProvidersConfig().openai.apiKey = value;
        else if (providerName == "anthropic") config.getProvidersConfig().anthropic.apiKey = value;
        else if (providerName == "openrouter") config.getProvidersConfig().openrouter.apiKey = value;
        else if (providerName == "zhipu") config.getProvidersConfig().zhipu.apiKey = value;
        else if (providerName == "groq") config.getProvidersConfig().groq.apiKey = value;
        else {
          Serial.println("Unknown provider");
          return;
        }
        Serial.println(providerName + " API key updated");
      }
    }
  } else {
    Serial.println("Unknown configuration key");
    return;
  }

  if (config.save()) {
    Serial.println("Configuration saved");
  } else {
    Serial.println("Failed to save configuration");
  }
}

void SerialConsole::handleConfigFile() {
  const char* CONFIG_PATH = "/config/config.json";
  
  File file = LittleFS.open(CONFIG_PATH, "r");
  if (!file) {
    Serial.println("Config file not found: " + String(CONFIG_PATH));
    return;
  }
  
  Serial.println("Config File Content:");
  Serial.println("====================");
  Serial.println();
  
  while (file.available()) {
    String line = file.readStringUntil('\n');
    Serial.println(line);
  }
  
  file.close();
}

void SerialConsole::handleHelp() {
  Serial.println("Available commands:");
  Serial.println("  /chat <message>    Send message to AI");
  Serial.println("  /config <key> <value>  Set configuration (use /config for list)");
  Serial.println("  /configfile        Show config.json file content");
  Serial.println("  /status             Show system status");
  Serial.println("  /reboot            Reboot device");
  Serial.println("  /help              Show this help message");
}

void SerialConsole::handleStatus() {
  Serial.println("System Status:");
  Serial.println("----------------");
  
  WiFiManager& wifi = WiFiManager::getInstance();
  Serial.print("WiFi: ");
  if (wifi.isConnected()) {
    Serial.print("Connected (");
    Serial.print(wifi.getSSID());
    Serial.print(", ");
    Serial.print(wifi.getIP());
    Serial.print(", RSSI: ");
    Serial.print(wifi.getRSSI());
    Serial.println(" dBm)");
  } else {
    Serial.println("Disconnected");
  }

  Serial.println("Free Heap: " + String(ESP.getFreeHeap()) + " bytes");
  Serial.println("Uptime: " + String(millis() / 1000) + " seconds");
  Serial.println("Log Level: " + String(Logger::getInstance().getLevel()));
  
  Config& config = Config::getInstance();
  Serial.println("Model: " + config.getAgentConfig().model);
}

void SerialConsole::handleReboot() {
  Serial.println("Rebooting...");
  delay(1000);
  ESP.restart();
}

void SerialConsole::processCommand(const String& command) {
  if (command.startsWith("/")) {
    String cmd = command.substring(1);
    int spaceIndex = cmd.indexOf(' ');

    String action;
    String args;

    if (spaceIndex > 0) {
      action = cmd.substring(0, spaceIndex);
      args = cmd.substring(spaceIndex + 1);
    } else {
      action = cmd;
    }

    action.toLowerCase();

    if (action == "help") {
      handleHelp();
    } else if (action == "status") {
      handleStatus();
    } else if (action == "config") {
      handleConfig(args);
    } else if (action == "configfile") {
      handleConfigFile();
    } else if (action == "reboot") {
      handleReboot();
    } else if (action == "chat") {
      String message = args;
      InboundMessage msg;
      msg.channel = "serial";
      msg.senderId = "user";
      msg.content = message;
      msg.sessionId = "default";
      msg.timestamp = millis();
      MessageBus::getInstance().publishInbound(msg);
    } else {
      Serial.println("Unknown command. Type /help for available commands.");
    }
  } else {
    Serial.println("Commands must start with /");
  }

  Serial.println();
  Serial.println("🦞 ");
}
