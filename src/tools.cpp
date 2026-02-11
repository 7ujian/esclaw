#include "../include/tools.h"
#include <LittleFS.h>

class ReadFileTool : public Tool {
public:
  String getName() const override {
    return "read_file";
  }

  String getDescription() const override {
    return "Read the contents of a file from the device's file system";
  }

  void getParameters(JsonArray& properties, JsonArray& required) const override {
    JsonObject prop = properties.createNestedObject();
    prop["type"] = "string";
    prop["description"] = "The file path to read (relative to workspace)";
    prop["name"] = "path";
    required.add("path");
  }

  ToolResult execute(const JsonVariantConst& args) override {
    ToolResult result;

    if (!args.containsKey("path")) {
      result.success = false;
      result.error = "Missing required parameter: path";
      return result;
    }

    String path = args["path"].as<String>();
    String fullPath = "/workspace/" + sanitizePath(path);

    if (!isValidPath(fullPath)) {
      result.success = false;
      result.error = "Invalid path: " + path;
      return result;
    }

    File file = LittleFS.open(fullPath, "r");
    if (!file) {
      result.success = false;
      result.error = "File not found: " + path;
      return result;
    }

    if (file.size() > 10000) {
      result.success = false;
      result.error = "File too large: " + path + " (max 10KB)";
      file.close();
      return result;
    }

    String content;
    while (file.available()) {
      content += (char)file.read();
    }
    file.close();

    result.success = true;
    result.content = content;

    return result;
  }
};

class WriteFileTool : public Tool {
public:
  String getName() const override {
    return "write_file";
  }

  String getDescription() const override {
    return "Write content to a file on the device's file system";
  }

  void getParameters(JsonArray& properties, JsonArray& required) const override {
    JsonObject prop1 = properties.createNestedObject();
    prop1["type"] = "string";
    prop1["description"] = "The file path to write (relative to workspace)";
    prop1["name"] = "path";

    JsonObject prop2 = properties.createNestedObject();
    prop2["type"] = "string";
    prop2["description"] = "The content to write to the file";
    prop2["name"] = "content";

    required.add("path");
    required.add("content");
  }

  ToolResult execute(const JsonVariantConst& args) override {
    ToolResult result;

    if (!args.containsKey("path") || !args.containsKey("content")) {
      result.success = false;
      result.error = "Missing required parameters: path, content";
      return result;
    }

    String path = args["path"].as<String>();
    String content = args["content"].as<String>();
    String fullPath = "/workspace/" + sanitizePath(path);

    if (!isValidPath(fullPath)) {
      result.success = false;
      result.error = "Invalid path: " + path;
      return result;
    }

    if (content.length() > 10000) {
      result.success = false;
      result.error = "Content too large (max 10KB)";
      return result;
    }

    File file = LittleFS.open(fullPath, "w");
    if (!file) {
      result.success = false;
      result.error = "Failed to create file: " + path;
      return result;
    }

    size_t written = file.write((uint8_t*)content.c_str(), content.length());
    file.close();

    if (written != content.length()) {
      result.success = false;
      result.error = "Failed to write complete file: " + path;
      return result;
    }

    result.success = true;
    result.content = "File written successfully: " + path;

    return result;
  }
};

class ListDirTool : public Tool {
public:
  String getName() const override {
    return "list_dir";
  }

  String getDescription() const override {
    return "List files and directories in a given path";
  }

  void getParameters(JsonArray& properties, JsonArray& required) const override {
    JsonObject prop1 = properties.createNestedObject();
    prop1["type"] = "string";
    prop1["description"] = "The directory path to list (default: /workspace)";
    prop1["name"] = "path";

    JsonObject prop2 = properties.createNestedObject();
    prop2["type"] = "boolean";
    prop2["description"] = "List files recursively (default: false)";
    prop2["name"] = "recursive";
  }

  ToolResult execute(const JsonVariantConst& args) override {
    ToolResult result;

    String pathArg = args["path"] | "";
    bool recursive = args["recursive"] | false;

    String path = pathArg.isEmpty() ? "/workspace" : "/workspace/" + sanitizePath(pathArg);

    if (!isValidPath(path) && !path.equals("/workspace")) {
      result.success = false;
      result.error = "Invalid path: " + pathArg;
      return result;
    }

    File dir = LittleFS.open(path);
    if (!dir || !dir.isDirectory()) {
      result.success = false;
      result.error = "Directory not found: " + pathArg;
      if (dir) dir.close();
      return result;
    }

    String output = "Contents of " + path + ":\n";
    output += "------------------------\n";

    listDirectory(dir, output, "", recursive);

    dir.close();

    result.success = true;
    result.content = output;

    return result;
  }

private:
  void listDirectory(File& dir, String& output, const String& prefix, bool recursive) {
    File file = dir.openNextFile();
    while (file) {
      String fileName = file.name();
      if (!fileName.startsWith("/workspace")) {
        file.close();
        file = dir.openNextFile();
        continue;
      }

      String displayName = fileName.substring(String("/workspace/").length());
      if (displayName.startsWith(prefix) && !prefix.isEmpty()) {
        displayName = displayName.substring(prefix.length());
      }
      if (displayName.startsWith("/")) {
        displayName = displayName.substring(1);
      }

      if (file.isDirectory()) {
        output += "  [DIR]  " + displayName + "/\n";
        if (recursive) {
          File subDir = LittleFS.open(file.name());
          listDirectory(subDir, output, displayName + "/", recursive);
          subDir.close();
        }
      } else {
        output += "  [FILE] " + displayName + " (" + String(file.size()) + " bytes)\n";
      }
      file.close();
      file = dir.openNextFile();
    }
  }
};

class DeviceInfoTool : public Tool {
public:
  String getName() const override {
    return "device_info";
  }

  String getDescription() const override {
    return "Get device information including CPU, memory, and WiFi status";
  }

  void getParameters(JsonArray& properties, JsonArray& required) const override {
  }

  ToolResult execute(const JsonVariantConst& args) override {
    ToolResult result;

    String info = "Device Information:\n";
    info += "------------------\n";
    info += "Chip: ESP32\n";
    info += "CPU Cores: 2\n";
    info += "CPU Frequency: " + String(ESP.getCpuFreqMHz()) + " MHz\n";
    info += "Flash Size: " + String(ESP.getFlashChipSize() / 1024 / 1024) + " MB\n";
    info += "Free Heap: " + String(ESP.getFreeHeap()) + " bytes\n";
    info += "Free PSRAM: " + String(ESP.getFreePsram()) + " bytes\n";
    info += "Uptime: " + String(millis() / 1000) + " seconds\n";

    result.success = true;
    result.content = info;

    return result;
  }
};

class RebootTool : public Tool {
public:
  String getName() const override {
    return "reboot";
  }

  String getDescription() const override {
    return "Restart the device after a brief delay";
  }

  void getParameters(JsonArray& properties, JsonArray& required) const override {
    JsonObject prop = properties.createNestedObject();
    prop["type"] = "integer";
    prop["description"] = "Delay in seconds before reboot (default: 2)";
    prop["name"] = "delay";
  }

  ToolResult execute(const JsonVariantConst& args) override {
    ToolResult result;

    int delaySec = args["delay"] | 2;

    if (delaySec < 1) delaySec = 1;
    if (delaySec > 60) delaySec = 60;

    result.success = true;
    result.content = "Rebooting in " + String(delaySec) + " seconds...";

    delay(delaySec * 1000);
    ESP.restart();

    return result;
  }
};

class WiFiConfigTool : public Tool {
public:
  String getName() const override {
    return "wifi_config";
  }

  String getDescription() const override {
    return "Configure WiFi settings and reconnect";
  }

  void getParameters(JsonArray& properties, JsonArray& required) const override {
    JsonObject prop1 = properties.createNestedObject();
    prop1["type"] = "string";
    prop1["description"] = "WiFi network name";
    prop1["name"] = "ssid";

    JsonObject prop2 = properties.createNestedObject();
    prop2["type"] = "string";
    prop2["description"] = "WiFi password";
    prop2["name"] = "password";
  }

  ToolResult execute(const JsonVariantConst& args) override {
    ToolResult result;

    String ssid = args["ssid"] | "";
    String password = args["password"] | "";

    if (ssid.isEmpty()) {
      result.success = false;
      result.error = "SSID is required";
      return result;
    }

    extern void updateWiFiConfig(const String& ssid, const String& password);
    updateWiFiConfig(ssid, password);

    result.success = true;
    result.content = "WiFi configured. Reconnecting to: " + ssid;

    return result;
  }
};

void registerAllTools() {
  ToolRegistry::getInstance().registerTool(new ReadFileTool());
  ToolRegistry::getInstance().registerTool(new WriteFileTool());
  ToolRegistry::getInstance().registerTool(new ListDirTool());
  ToolRegistry::getInstance().registerTool(new DeviceInfoTool());
  ToolRegistry::getInstance().registerTool(new RebootTool());
  ToolRegistry::getInstance().registerTool(new WiFiConfigTool());
}
