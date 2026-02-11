#ifndef SERIAL_CONSOLE_H
#define SERIAL_CONSOLE_H

#include <Arduino.h>

class SerialConsole {
public:
  static SerialConsole& getInstance();

  void begin();
  void update();

  bool hasInput() const;
  String getInput();
  void print(const String& message);
  void println(const String& message);

private:
  SerialConsole();
  SerialConsole(const SerialConsole&) = delete;
  SerialConsole& operator=(const SerialConsole&) = delete;

  String inputBuffer_;
  bool inputReady_;

   void processCommand(const String& command);
   void showConfigHelp();
   void showCurrentConfig();
   void handleHelp();
   void handleStatus();
   void handleConfig(const String& args);
   void handleConfigFile();
   void handleReboot();
 };

#endif
