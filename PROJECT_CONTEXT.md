# ESPClaw Project Context

## Project Overview

**Name**: ESPClaw 🦞 - ESP32 AI Assistant  
**Platform**: ESP32 (PlatformIO/Arduino Framework)  
**Reference**: PicoClaw (Go) by Sipeed  
**Version**: 0.1.0  
**Status**: Phase 3 Ready

---

## Current Status

### Completed ✅
- **Phase 1**: Foundation (Configuration, WiFi, Message Bus, Serial Console)
- **Phase 2**: LLM Integration (HTTP Client, LLM Provider, Multi-provider support)

### In Progress 🚧
- **Phase 3**: Tool System (Ready to start)

### Pending
- Phase 4: Agent Core
- Phase 5: Advanced Features
- Phase 6: Testing & Documentation

---

## Build Statistics

```
RAM:   [=         ] 16.1% (used 52,920 bytes from 327,680 bytes)
Flash: [======    ] 72.1% (used 944,805 bytes from 1,310,720 bytes)
```

**Total Source Code**: 2,209 lines

---

## Core Components

### Config System (`include/config.h`, `src/config.cpp`)
- JSON-based configuration in `/config/config.json`
- WiFi settings, agent settings, provider API keys
- Environment variable override support

### WiFi Manager (`include/wifi_manager.h`, `src/wifi_manager.cpp`)
- Auto-connect on startup
- Reconnect on disconnect
- Status monitoring (IP, MAC, RSSI)

### Message Bus (`include/message_bus.h`, `src/message_bus.cpp`)
- Thread-safe message queue (50 message max)
- Inbound/Outbound message support
- Channel-based routing

### Serial Console (`include/serial_console.h`, `src/serial_console.cpp`)
- Interactive CLI with 🦞 emoji prompt
- Commands: `/help`, `/status`, `/config`, `/reboot`, `/chat`

### LLM Provider (`include/llm_provider.h`, `src/llm_provider.cpp`)
- Support for: OpenAI, Anthropic, OpenRouter, Zhipu, Groq, custom endpoints
- JSON request/response handling
- Tool calls parsing
- Token usage tracking

---

## File Structure

```
esclaw/
├── include/
│   ├── agents_md.h          # System prompt
│   ├── config.h             # Configuration system
│   ├── message_bus.h        # Message queue
│   ├── serial_console.h     # CLI interface
│   └── llm_provider.h       # LLM interface
├── src/
│   ├── main.cpp             # Entry point
│   ├── config.cpp
│   ├── message_bus.cpp
│   ├── serial_console.cpp
│   ├── wifi_manager.cpp
│   └── llm_provider.cpp
├── platformio.ini
└── Documentation files...
```

---

## Build Commands

```bash
pio run                    # Build firmware
pio run --target upload    # Upload to device
pio run --target clean      # Clean build
pio run -t compiledb       # Regenerate compile_commands.json
pio device monitor         # Serial monitor
```

---

## Configuration Example

```json
{
  "wifi": {
    "ssid": "",
    "password": "",
    "timeout": 30
  },
  "agents": {
    "defaults": {
      "model": "gpt-3.5-turbo",
      "maxTokens": 2048,
      "temperature": 0.7
    }
  },
  "providers": {
    "openai": {"apiKey": "", "apiBase": "https://api.openai.com/v1"},
    "anthropic": {"apiKey": "", "apiBase": "https://api.anthropic.com/v1"},
    "openrouter": {"apiKey": "", "apiBase": "https://openrouter.ai/api/v1"},
    "zhipu": {"apiKey": "", "apiBase": "https://open.bigmodel.cn/api/paas/v4"},
    "groq": {"apiKey": "", "apiBase": "https://api.groq.com/openai/v1"}
  }
}
```

---

## Quick Start

1. **Build firmware**: `pio run`
2. **Upload to device**: `pio run --target upload`
3. **Open serial monitor**: `pio device monitor`
4. **Configure WiFi**: `/config wifi.ssid YourNetwork`
5. **Configure API key**: `/config providers.openai.apiKey YourKey`
6. **Test chat**: `/chat Hello`

---

## Serial Commands

| Command | Description |
|---------|-------------|
| `/chat <message>` | Send message to AI |
| `/config <key> <value>` | Set configuration |
| `/status` | Show system status |
| `/reboot` | Reboot device |
| `/help` | Show help |

---

## Dependencies

```
ArduinoJson @ 6.21.5
ArduinoHttpClient @ 0.5.0
LittleFS @ 2.0.0
WiFiClientSecure @ 2.0.0
```

---

## Phase 3 Plan (Next)

Implement Tool System:
- Tool base class and registry
- File tools (read, write, list)
- Device tools (info, reboot)
- Network tools (WiFi config)

**Estimated**: ~680 lines, +37KB RAM

---

## Documentation Files

- `PROJECT_PLAN.md` - 6-week implementation plan
- `PROJECT_ARCHITECTURE.md` - System architecture
- `AGENTS.md` - Coding guidelines
- `PHASE1_STATUS.md` - Phase 1 summary
- `PHASE2_STATUS.md` - Phase 2 summary
- `PHASE3_STATUS.md` - Phase 3 plan
- `README.md` - Quick start guide

---

**Last Updated**: February 11, 2026
