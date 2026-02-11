#include "../include/tools.h"
#include <LittleFS.h>
#include <cstring>

ToolRegistry::ToolRegistry()
  : toolCount_(0) {
  for (int i = 0; i < MAX_TOOLS; i++) {
    tools_[i] = nullptr;
  }
}

ToolRegistry& ToolRegistry::getInstance() {
  static ToolRegistry instance;
  return instance;
}

bool ToolRegistry::registerTool(Tool* tool) {
  if (toolCount_ >= MAX_TOOLS) {
    return false;
  }
  tools_[toolCount_++] = tool;
  return true;
}

Tool* ToolRegistry::getTool(const String& name) {
  for (int i = 0; i < toolCount_; i++) {
    if (tools_[i]->getName() == name) {
      return tools_[i];
    }
  }
  return nullptr;
}

ToolResult ToolRegistry::executeTool(const String& name, const JsonVariantConst& args) {
  Tool* tool = getTool(name);
  if (!tool) {
    ToolResult result;
    result.success = false;
    result.error = "Tool not found: " + name;
    return result;
  }
  return tool->execute(args);
}

void ToolRegistry::getToolDefinitions(JsonArray& toolsArray) {
  for (int i = 0; i < toolCount_; i++) {
    JsonObject toolObj = toolsArray.createNestedObject();
    toolObj["name"] = tools_[i]->getName();
    toolObj["description"] = tools_[i]->getDescription();

    JsonObject params = toolObj.createNestedObject("parameters");
    params["type"] = "object";

    JsonArray props = params.createNestedArray("properties");
    JsonArray required = params.createNestedArray("required");

    tools_[i]->getParameters(props, required);
  }
}

int ToolRegistry::getToolCount() const {
  return toolCount_;
}

String sanitizePath(const String& path) {
  String sanitized = path;

  while (sanitized.startsWith("/")) {
    sanitized = sanitized.substring(1);
  }

  while (sanitized.startsWith("../")) {
    sanitized = sanitized.substring(3);
  }

  int pos = 0;
  while ((pos = sanitized.indexOf("/../")) != -1) {
    int start = sanitized.lastIndexOf("/", pos - 1);
    if (start == -1) {
      start = 0;
    }
    sanitized = sanitized.substring(0, start) + sanitized.substring(pos + 3);
  }

  return sanitized;
}

bool isValidPath(const String& path) {
  if (path.isEmpty()) {
    return false;
  }

  if (path.startsWith("/config") || path.startsWith("/sessions") ||
      path.startsWith("/system")) {
    return false;
  }

  if (path.indexOf("..") != -1) {
    return false;
  }

  return true;
}
