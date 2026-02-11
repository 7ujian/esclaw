# ESPClaw - ESP32 AI Assistant

## Project Overview

ESPClaw is a lightweight AI assistant for ESP32 microcontroller, ported from PicoClaw (Go). It provides LLM-powered assistance with minimal resource usage, optimized for embedded environments.

## Architecture Overview

```
┌─────────────────────────────────────────────────────────────────┐
│                         ESP32 Hardware                           │
│  - Dual-core 240MHz                                             │
│  - 520KB SRAM                                                   │
│  - 4MB+ Flash (LittleFS)                                        │
│  - WiFi Connectivity                                             │
└─────────────────────────────────────────────────────────────────┘
                                │
┌─────────────────────────────────────────────────────────────────┐
│                       Main Application                          │
│  - setup()/loop() lifecycle                                     │
│  - WiFi Connection Manager                                      │
│  - Serial Console Interface                                     │
│  - Optional Web Server                                         │
└─────────────────────────────────────────────────────────────────┘
                                │
┌─────────────────────────────────────────────────────────────────┐
│                     Message Bus (Core)                          │
│  - Inbound/Outbound Queue                                      │
│  - Serial Message Handler                                      │
│  - WebSocket Message Handler (optional)                        │
└─────────────────────────────────────────────────────────────────┘
                                │
        ┌───────────────────────┴───────────────────────┐
        │                                               │
┌───────────────────┐                         ┌───────────────────┐
│  Agent Loop      │                         │  Tool System     │
│                  │                         │                   │
│  - Context       │                         │  - Read File     │
│    Builder      │                         │  - Write File    │
│  - LLM          │                         │  - List Dir      │
│    Provider     │                         │  - WiFi Config   │
│  - Tool         │                         │  - Device Info   │
│    Executor     │                         │  - Reboot        │
│  - Session      │                         │  - OTA Update    │
│    Manager      │                         │                   │
└───────────────────┘                         └───────────────────┘
        │                                               │
        └───────────────────────┬───────────────────────┘
                                │
┌─────────────────────────────────────────────────────────────────┐
│                   Storage Layer (LittleFS)                       │
│  - /config/config.json    - Configuration                      │
│  - /sessions/             - Session history                    │
│  - /workspace/            - User workspace                      │
│  - /memory/              - Long-term memory                    │
│  - /skills/              - Custom skills (optional)            │
└─────────────────────────────────────────────────────────────────┘
                                │
┌─────────────────────────────────────────────────────────────────┐
│                   External Services                              │
│  - LLM Provider APIs (HTTP)                                    │
│  - Optional: Web Search APIs                                    │
└─────────────────────────────────────────────────────────────────┘
```

## Core Modules

### 1. Main Application (`src/main.cpp`)

**Responsibilities:**
- Initialize hardware (Serial, WiFi, Flash)
- Load configuration
- Create subsystem instances
- Run main event loop

**Key Components:**
```cpp
void setup() {
  Serial.begin(115200);
  LittleFS.begin();
  WiFiManager::getInstance().connect();
  Config::load();
  MessageBus::getInstance();
  AgentLoop::getInstance();
}

void loop() {
  AgentLoop::getInstance().process();
}
```

### 2. Message Bus (`include/message_bus.h`)

**Purpose:** Decoupled communication between components

**Implementation:**
- Thread-safe queue for inbound/outbound messages
- Simple pub/sub pattern (no heavy framework)
- Message types: Serial, WebSocket (optional)

```cpp
struct InboundMessage {
  String channel;
  String senderId;
  String content;
  String sessionId;
};

struct OutboundMessage {
  String channel;
  String content;
};
```

**Constraints:**
- Limit queue size to prevent memory exhaustion
- Use circular buffer or fixed-size queue
- No dynamic allocation in critical paths

### 3. LLM Provider (`include/llm_provider.h`)

**Purpose:** Unified interface to multiple LLM APIs

**Supported Providers:**
- OpenAI (GPT models)
- Anthropic (Claude)
- OpenRouter (multi-model)
- Zhipu (GLM models)
- Groq (fast inference)
- Custom endpoints (vLLM, etc.)

**Implementation:**
- HTTP client wrapper using ArduinoHttpClient
- JSON request/response handling (ArduinoJson)
- Provider selection based on model name
- Error handling and retry logic

```cpp
class LLMProvider {
public:
  struct Response {
    String content;
    std::vector<ToolCall> toolCalls;
    String finishReason;
  };

  Response chat(const std::vector<Message>& messages,
                const std::vector<ToolDefinition>& tools,
                const String& model);
};
```

**Memory Considerations:**
- Stream JSON parsing to avoid loading full response in memory
- Limit response size (truncate if needed)
- Use fixed buffers where possible

### 4. Tool System (`include/tools.h`)

**Purpose:** Execute actions requested by LLM

**Implementation:**
- Base `Tool` interface
- Tool registry with string-based lookup
- JSON parameter parsing

```cpp
class Tool {
public:
  virtual String name() const = 0;
  virtual String description() const = 0;
  virtual String parameters() const = 0;
  virtual String execute(const JsonObject& args) = 0;
};
```

**Core Tools:**
1. `ReadFileTool` - Read files from LittleFS
2. `WriteFileTool` - Write files to LittleFS
3. `ListDirTool` - List directory contents
4. `WiFiConfigTool` - Configure WiFi credentials
5. `DeviceInfoTool` - Get device information
6. `RebootTool` - Restart device
7. `OTAUpdateTool` - Over-the-air firmware update

**Omitted from PicoClaw:**
- Shell execution (not applicable on ESP32)
- Web search (optional, can add later)
- Subagent spawning (too complex for ESP32)
- Edit tool (can use read+write)

### 5. Context Builder (`include/context_builder.h`)

**Purpose:** Build system prompt with context

**Components:**
- System instructions from `AGENTS.md`
- Available tools description
- Session history (limited to last N messages)
- Optional: Skills content
- Optional: Memory from `MEMORY.md`

**Implementation:**
```cpp
String ContextBuilder::build(const String& userMessage,
                            const String& sessionId) {
  String context = loadSystemPrompt();
  context += "\n\n## Available Tools\n";
  context += getToolsDescription();
  context += "\n\n## Session History\n";
  context += getSessionHistory(sessionId);
  context += "\n\n## Current Message\n";
  context += "User: " + userMessage + "\n";
  return context;
}
```

**Memory Optimization:**
- Limit history to last 5-10 messages
- Summarize older messages (optional)
- Truncate large messages

### 6. Session Manager (`include/session_manager.h`)

**Purpose:** Persist conversation history

**Implementation:**
- Store sessions in `/sessions/{sessionId}.json`
- Load/save using ArduinoJson
- Limit history size
- Implement summarization for long sessions

```cpp
struct Session {
  String sessionId;
  std::vector<Message> messages;
  String summary;
  uint32_t lastUpdated;
};
```

### 7. Agent Loop (`include/agent_loop.h`)

**Purpose:** Core AI processing loop

**Algorithm:**
1. Receive inbound message from bus
2. Build context
3. Call LLM with tool definitions
4. If tool calls present:
   - Execute tools
   - Feed results back to LLM
   - Repeat until no more tool calls
5. Return final response
6. Save to session
7. Publish outbound message

**Pseudo-code:**
```cpp
void AgentLoop::processMessage(const InboundMessage& msg) {
  auto session = sessionManager_.getSession(msg.sessionId);
  auto context = contextBuilder_.build(msg.content, msg.sessionId);

  for (int i = 0; i < MAX_ITERATIONS; i++) {
    auto response = llmProvider_.chat(context, tools, model);

    if (response.toolCalls.empty()) {
      session.messages.push_back({"user", msg.content});
      session.messages.push_back({"assistant", response.content});
      sessionManager_.save(session);
      bus_.publish({msg.channel, response.content});
      return;
    }

    for (auto& call : response.toolCalls) {
      auto result = tools_.execute(call.name, call.arguments);
      context += formatToolResult(call, result);
    }
  }
}
```

### 8. Configuration Manager (`include/config.h`)

**Purpose:** Load and manage configuration

**File Structure:**
```json
{
  "wifi": {
    "ssid": "",
    "password": "",
    "timeout": 30
  },
  "agent": {
    "model": "gpt-3.5-turbo",
    "maxTokens": 2048,
    "temperature": 0.7,
    "maxToolIterations": 5,
    "maxHistoryLength": 10
  },
  "providers": {
    "openai": {"apiKey": "", "apiBase": "https://api.openai.com/v1"},
    "anthropic": {"apiKey": "", "apiBase": "https://api.anthropic.com/v1"},
    "openrouter": {"apiKey": "", "apiBase": "https://openrouter.ai/api/v1"},
    "zhipu": {"apiKey": "", "apiBase": "https://open.bigmodel.cn/api/paas/v4"},
    "groq": {"apiKey": "", "apiBase": "https://api.groq.com/openai/v1"}
  },
  "webserver": {
    "enabled": false,
    "port": 80
  }
}
```

### 9. WiFi Manager (`include/wifi_manager.h`)

**Purpose:** Manage WiFi connection

**Features:**
- Auto-connect on startup
- Connection status monitoring
- Reconnect on disconnect
- Configuration via serial

### 10. Serial Console (`src/serial_console.cpp`)

**Purpose:** Interactive command line interface

**Commands:**
- `/chat <message>` - Send message to agent
- `/config <key> <value>` - Set configuration
- `/status` - Show system status
- `/reboot` - Reboot device
- `/ota <url>` - Start OTA update
- `/help` - Show available commands

## Data Flow

### Typical Interaction Flow

```
User (Serial)
    │
    ▼ "/chat What time is it?"
Serial Console
    │
    ▼ InboundMessage
Message Bus
    │
    ▼ consume()
Agent Loop
    │
    ├─▶ Context Builder
    │     │
    │     └─▶ Load AGENTS.md
    │     └─▶ Load tools description
    │     └─▶ Load session history
    │
    ├─▶ LLM Provider
    │     │
    │     └─▶ HTTP POST to API
    │     └─▶ Parse JSON response
    │
    ├─▶ Tool Executor (if needed)
    │     │
    │     └─▶ DeviceInfoTool
    │
    └─▶ Response
          │
          ▼
     OutboundMessage
          │
          ▼
Serial Console
    │
    ▼ "🦞 The current time is..."
User
```

## Memory Management Strategy

### Flash Storage (LittleFS)
- Configuration: `/config/config.json` (~2KB)
- Sessions: `/sessions/*.json` (limit to ~50KB total)
- Workspace: `/workspace/*` (user data)
- Memory: `/memory/MEMORY.md` (long-term context)

### RAM Allocation (520KB total)
- FreeRTOS heap: ~300KB available
- Arduino stack: ~50KB
- Buffers: ~20KB
- JSON parsing buffers: ~50KB (reused)
- WiFi stack: ~100KB (allocated by system)

### Optimization Techniques
1. **Pre-allocate buffers**: Avoid dynamic allocation in critical paths
2. **Stream processing**: Parse JSON incrementally using streams
3. **Limit history**: Keep only recent messages in memory
4. **Reuse buffers**: Reuse ArduinoJson documents across calls
5. **String fragmentation**: Use `String` sparingly; prefer char arrays
6. **Pool allocation**: Use static pools for message objects

## Concurrency Model

### FreeRTOS Tasks
- **Main Task**: `loop()` - Serial console, message dispatch
- **Network Task**: WiFi monitoring, HTTP client
- **Agent Task**: LLM processing (optional, can run on main)

### Synchronization
- Use semaphores for shared resource access
- Queue for message passing between tasks
- Avoid mutex in time-critical paths

## Security Considerations

### API Key Storage
- Store API keys in config.json (in flash)
- Consider encryption if device is exposed
- Provide option to enter keys via serial (not stored)

### WiFi Credentials
- Store in config.json (in flash)
- Support WPA2-Enterprise (optional)

### Input Validation
- Sanitize file paths (prevent directory traversal)
- Limit tool execution parameters
- Validate JSON before parsing

## Differences from PicoClaw

### Simplified/Removed Features
1. **Chat Platform Integrations** - No Telegram, Discord, QQ, etc.
2. **Complex Skills System** - Simplified to file-based skills only
3. **Shell Execution** - Not applicable on ESP32
4. **Web Search** - Optional, can add later
5. **Subagent Spawning** - Too complex for ESP32
6. **Cron Service** - Limited to simple timer-based tasks
7. **Voice Transcription** - Optional, requires additional hardware

### Enhanced Features for ESP32
1. **Hardware Integration** - GPIO control, sensors
2. **OTA Updates** - Wireless firmware updates
3. **Device Control** - Reboot, sleep mode, power management
4. **WiFi Config** - Network configuration via serial
5. **Web Interface** - Optional web UI for interaction

## Extension Points

### Adding New Tools
1. Inherit from `Tool` interface
2. Implement required methods
3. Register in `ToolRegistry`
4. Add to `platformio.ini` dependencies if needed

### Adding New LLM Providers
1. Add provider config to `Config` struct
2. Update `LLMProvider::createProvider()` factory
3. Test API compatibility

### Adding New Channels
1. Implement `Channel` interface (like SerialChannel)
2. Register in `MessageBus`
3. Handle inbound/outbound message routing

## Testing Strategy

### Unit Tests
- Tool execution with various inputs
- JSON parsing/serialization
- Configuration loading
- WiFi connection handling

### Integration Tests
- End-to-end message flow
- LLM provider integration
- Session persistence
- Multi-tool execution

### Hardware Tests
- Flash storage reliability
- WiFi connectivity under various conditions
- Memory usage profiling
- OTA update process
