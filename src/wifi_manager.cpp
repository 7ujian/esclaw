#include "wifi_manager.h"
#include "config.h"
#include <WiFi.h>

WiFiManager::WiFiManager() : timeout_(30), lastConnectAttempt_(0), connected_(false) {
}

WiFiManager& WiFiManager::getInstance() {
  static WiFiManager instance;
  return instance;
}

void WiFiManager::configure(const String& ssid, const String& password) {
  ssid_ = ssid;
  password_ = password;
}

bool WiFiManager::connect() {
  if (ssid_.isEmpty()) {
    return false;
  }

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid_.c_str(), password_.c_str());

  unsigned long startMillis = millis();
  while (WiFi.status() != WL_CONNECTED && 
         millis() - startMillis < timeout_ * 1000) {
    delay(100);
  }

  connected_ = (WiFi.status() == WL_CONNECTED);

  if (connected_) {
    lastConnectAttempt_ = millis();
  }

  return connected_;
}

void WiFiManager::disconnect() {
  WiFi.disconnect();
  connected_ = false;
}

bool WiFiManager::isConnected() const {
  return connected_ && (WiFi.status() == WL_CONNECTED);
}

String WiFiManager::getIP() const {
  if (isConnected()) {
    return WiFi.localIP().toString();
  }
  return "Not connected";
}

String WiFiManager::getMAC() const {
  return WiFi.macAddress();
}

String WiFiManager::getSSID() const {
  if (isConnected()) {
    return WiFi.SSID();
  }
  return ssid_;
}

int8_t WiFiManager::getRSSI() const {
  if (isConnected()) {
    return WiFi.RSSI();
  }
  return 0;
}

bool WiFiManager::attemptConnection() {
  unsigned long now = millis();

  if (now - lastConnectAttempt_ < 5000) {
    return connected_;
  }

  if (WiFi.status() != WL_CONNECTED && !ssid_.isEmpty()) {
    WiFi.reconnect();
    lastConnectAttempt_ = now;
    delay(100);
  }

  connected_ = (WiFi.status() == WL_CONNECTED);
  return connected_;
}

void WiFiManager::update() {
  attemptConnection();
}
