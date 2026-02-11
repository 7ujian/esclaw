#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>

class WiFiManager {
public:
  static WiFiManager& getInstance();

  bool connect();
  void disconnect();
  bool isConnected() const;
  String getIP() const;
  String getMAC() const;
  String getSSID() const;
  int8_t getRSSI() const;

  void configure(const String& ssid, const String& password);

  void update();

private:
  WiFiManager();
  WiFiManager(const WiFiManager&) = delete;
  WiFiManager& operator=(const WiFiManager&) = delete;

  String ssid_;
  String password_;
  int timeout_;
  unsigned long lastConnectAttempt_;
  bool connected_;

  bool attemptConnection();
};

#endif
