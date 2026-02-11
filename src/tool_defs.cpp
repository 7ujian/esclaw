#include "tool_defs.h"
#include "tools.h"

void populateToolDefinitions(JsonArray& tools) {
  ToolRegistry::getInstance().getToolDefinitions(tools);
}
