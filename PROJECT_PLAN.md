# ESPClaw Implementation Plan

## Project Status

- **Original Project**: PicoClaw (Go) - Ultra-lightweight AI assistant
- **Target Platform**: ESP32 (Dual-core 240MHz, 520KB SRAM, 4MB+ Flash)
- **Target Language**: C++ (Arduino Framework)
- **Build System**: PlatformIO

## Project Structure

```
esclaw/
├── .opencode/
│   └── plan/                 # Planning documents
├── include/                  # Header files
│   ├── agent_loop.h
│   ├── config.h
│   ├── context_builder.h
│   ├── llm_provider.h
│   ├── message_bus.h
│   ├── session_manager.h
│   ├── tools.h
│   ├── tool_registry.h
│   ├── wifi_manager.h
│   └── serial_console.h
├── src/
│   ├── main.cpp              # Entry point
│   ├── agent_loop.cpp
│   ├── config.cpp
│   ├── context_builder.cpp
│   ├── llm_provider.cpp
│   ├── message_bus.cpp
│   ├── session_manager.cpp
│   ├── tools/
│   │   ├── read_file.cpp
│   │   ├── write_file.cpp
│   │   ├── list_dir.cpp
│   │   ├── wifi_config.cpp
│   │   ├── device_info.cpp
│   │   ├── reboot.cpp
│   │   └── ota_update.cpp
│   ├── tool_registry.cpp
│   ├── wifi_manager.cpp
│   └── serial_console.cpp
├── lib/                      # PlatformIO libraries
├── test/                     # Unit tests
├── platformio.ini            # Build configuration
├── AGENTS.md                 # Agent guidelines
└── PROJECT_ARCHITECTURE.md   # Architecture documentation
```

## Platform Dependencies

Update `platformio.ini`:

```ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino
board_build.filesystem = littlefs

lib_deps =
    bblanchon/ArduinoJson@^6.21.0
    arduino-libraries/ArduinoHttpClient@^0.5.0
    mathieucarbou/AsyncTCP@^1.1.1
    mathieucarbou/ESPAsyncWebServer@^1.2.3

build_flags =
    -DARDUINOJSON_USE_LONG_LONG=1
    -DARDUINOJSON_ENABLE_NAN=1
    -DCORE_DEBUG_LEVEL=3
```

## Implementation Phases

### Phase 1: Foundation (Week 1)

**Goal**: Basic project setup and core infrastructure

#### Tasks:

1. **PlatformIO Configuration**
   - [ ] Update `platformio.ini` with dependencies
   - [ ] Configure LittleFS
   - [ ] Set build flags

2. **Basic Structure**
   - [ ] Create directory structure
   - [ ] Create empty header files
   - [ ] Create AGENTS.md template

3. **Configuration System**
   - [ ] Implement `Config` class
   - [ ] Implement `load()` from JSON
   - [ ] Implement `save()` to JSON
   - [ ] Create default configuration
   - [ ] Test configuration persistence

4. **WiFi Manager**
   - [ ] Implement `WiFiManager` class
   - [ ] Auto-connect on startup
   - [ ] Status monitoring
   - [ ] Reconnect logic
   - [ ] Serial command for WiFi config

5. **Message Bus**
   - [ ] Implement `MessageBus` class
   - [ ] `InboundMessage` / `OutboundMessage` structs
   - [ ] Thread-safe queue (using FreeRTOS)
   - [ ] Publish/Subscribe pattern

6. **Serial Console**
   - [ ] Implement basic command parser
   - [ ] `/help` command
   - [ ] `/status` command
   - [ ] `/config` command
   - [ ] `/reboot` command

**Deliverable**: Project can boot, connect to WiFi, and accept serial commands.

---

### Phase 2: LLM Integration (Week 2)

**Goal**: Connect to LLM APIs and process responses

#### Tasks:

1. **HTTP Client Wrapper**
   - [ ] Wrap `ArduinoHttpClient` for easier use
   - [ ] Handle timeouts and retries
   - [ ] Error handling

2. **LLM Provider**
   - [ ] Implement `LLMProvider` base class
   - [ ] Implement `HTTPProvider` class
   - [ ] JSON request building
   - [ ] JSON response parsing (streaming)
   - [ ] Provider factory (model-based selection)

3. **Testing**
   - [ ] Test with OpenAI API
   - [ ] Test with Anthropic API
   - [ ] Test with OpenRouter
   - [ ] Test with Zhipu API
   - [ ] Test with Groq API

**Deliverable**: Can send messages to LLM and receive responses.

---

### Phase 3: Tool System (Week 3)

**Goal**: Implement tool execution framework and core tools

#### Tasks:

1. **Tool Interface**
   - [ ] Define `Tool` abstract class
   - [ ] Implement `ToolRegistry`
   - [ ] JSON parameter parsing
   - [ ] Tool lookup by name

2. **File System Tools**
   - [ ] `ReadFileTool` - Read from LittleFS
   - [ ] `WriteFileTool` - Write to LittleFS
   - [ ] `ListDirTool` - List directories
   - [ ] Path sanitization

3. **Device Tools**
   - [ ] `DeviceInfoTool` - Get ESP32 info
   - [ ] `RebootTool` - Restart device

4. **Network Tools**
   - [ ] `WiFiConfigTool` - Configure WiFi

**Deliverable**: Tool system functional with basic tools.

---

### Phase 4: Agent Core (Week 4)

**Goal**: Implement main AI processing loop

#### Tasks:

1. **Context Builder**
   - [ ] Load `AGENTS.md`
   - [ ] Load tools descriptions
   - [ ] Load session history
   - [ ] Build system prompt

2. **Session Manager**
   - [ ] Implement `Session` struct
   - [ ] Load/save sessions from LittleFS
   - [ ] Limit history size
   - [ ] Session ID generation

3. **Agent Loop**
   - [ ] Implement main processing algorithm
   - [ ] Tool execution loop
   - [ ] Response handling
   - [ ] Error handling
   - [ ] Session persistence

4. **Integration**
   - [ ] Connect Serial Console to Agent
   - [ ] Full message flow test

**Deliverable**: End-to-end chat with AI via serial console.

---

### Phase 5: Advanced Features (Week 5)

**Goal**: Add advanced capabilities

#### Tasks:

1. **OTA Update**
   - [ ] Implement `OTAUpdateTool`
   - [ ] HTTP download
   - [ ] Flash update
   - [ ] Rollback support

2. **Memory Management**
   - [ ] Session summarization
   - [ ] Large message truncation
   - [ ] Memory monitoring

3. **Web Server (Optional)**
   - [ ] Async web server setup
   - [ ] Web chat interface
   - [ ] Configuration UI

**Deliverable**: OTA updates working, memory optimized.

---

### Phase 6: Testing & Documentation (Week 6)

**Goal**: Ensure reliability and usability

#### Tasks:

1. **Unit Tests**
   - [ ] Tool execution tests
   - [ ] JSON parsing tests
   - [ ] Configuration tests

2. **Integration Tests**
   - [ ] End-to-end message flow
   - [ ] Multi-tool scenarios
   - [ ] Session persistence

3. **Documentation**
   - [ ] README with setup instructions
   - [ ] Configuration guide
   - [ ] API documentation
   - [ ] Troubleshooting guide

4. **Performance Tuning**
   - [ ] Memory profiling
   - [ ] Optimization
   - [ ] Stress testing

**Deliverable**: Production-ready codebase with documentation.

---

## Memory Budget

| Component | Target RAM | Notes |
|-----------|-----------|-------|
| FreeRTOS Heap | 280KB | Available for allocation |
| WiFi Stack | 100KB | System allocation |
| Arduino Core | 50KB | System allocation |
| Message Queue | 10KB | ~100 messages |
| JSON Buffer | 50KB | Reused across calls |
| Agent State | 20KB | Context, session, tools |
| HTTP Client | 30KB | Temporary during request |
| **Total Used** | **~260KB** | **Leaves ~260KB free** |

## Code Style Guidelines

### Headers
```cpp
#ifndef CLASS_NAME_H
#define CLASS_NAME_H

#include "required_headers.h"

class ClassName {
public:
  ClassName();
  ~ClassName();

  void method();

private:
  int memberVariable_;
};

#endif
```

### Implementation
```cpp
#include "class_name.h"

ClassName::ClassName() {
}

void ClassName::method() {
}
```

### Naming
- Classes: `PascalCase`
- Methods: `camelCase()`
- Variables: `camelCase`
- Constants: `UPPER_SNAKE_CASE`
- Private members: `camelCase_`

### Documentation
- Document public APIs with parameter descriptions
- No inline comments for obvious code
- Use `//` for single-line comments above code

## Risk Assessment

| Risk | Impact | Mitigation |
|------|--------|------------|
| Memory exhaustion | High | Strict memory budget, monitoring, limits |
| WiFi disconnection | Medium | Auto-reconnect, queue messages |
| API rate limits | Medium | Retry with backoff, provider fallback |
| JSON parsing errors | Low | Error handling, validation |
| Flash wear | Low | Wear leveling, minimize writes |

## Success Criteria

- [x] Connects to WiFi automatically
- [ ] Responds to queries via serial console
- [ ] Executes file operations
- [ ] Persists sessions across reboots
- [ ] Supports multiple LLM providers
- [ ] Uses < 300KB RAM at peak
- [ ] Boots in < 5 seconds
- [ ] Provides OTA updates
- [ ] Has comprehensive documentation
- [ ] Passes all unit tests

## Future Enhancements (Out of Scope)

- Web interface
- MQTT integration
- GPIO/sensor tools
- Local LLM support (Edge computing)
- Web search integration
- Skill system (similar to PicoClaw)
- Cron/scheduled tasks
- Voice interface

## Notes

- Prioritize simplicity and reliability over features
- Test each phase before moving to next
- Keep memory usage in mind at all times
- Use LittleFS for all persistent data
- Follow ESP32 best practices
- Document all APIs for future maintainers
