#ifndef TOOLS_H
#define TOOLS_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <FS.h>

struct ToolResult {
  bool success;
  String content;
  String error;
};

class Tool {
public:
  virtual ~Tool() {}

  virtual String getName() const = 0;
  virtual String getDescription() const = 0;
  virtual void getParameters(JsonArray& properties, JsonArray& required) const = 0;
  virtual ToolResult execute(const JsonVariantConst& args) = 0;
  virtual ToolResult executeString(const String& argsJson) { 
    ToolResult r;
    r.success = false;
    r.error = "Not implemented";
    return r;
  };
};

class ToolRegistry {
public:
  static ToolRegistry& getInstance();

  bool registerTool(Tool* tool);
  Tool* getTool(const String& name);
  ToolResult executeTool(const String& name, const JsonVariantConst& args);
  ToolResult executeToolString(const String& name, const String& argsJson);

  void getToolDefinitions(JsonArray& toolsArray);
  int getToolCount() const;

private:
  ToolRegistry();
  ToolRegistry(const ToolRegistry&) = delete;
  ToolRegistry& operator=(const ToolRegistry&) = delete;

  static const int MAX_TOOLS = 10;
  Tool* tools_[MAX_TOOLS];
  int toolCount_;
};

String sanitizePath(const String& path);
bool isValidPath(const String& path);

#endif
