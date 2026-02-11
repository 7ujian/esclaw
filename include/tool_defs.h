#ifndef TOOL_DEFS_H
#define TOOL_DEFS_H

#include <Arduino.h>
#include <ArduinoJson.h>

#define TOOL_DEFINITIONS_DOC_SIZE 2048

String getToolDefinitionsJson();

void populateToolDefinitions(JsonArray& tools);

#endif
