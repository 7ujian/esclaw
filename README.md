# ESPClaw - ESP32 AI Assistant

[![Build Status](https://img.shields.io/badge/build-passing-brightgreen)](https://github.com/7ujian/esclaw)
[![License](https://img.shields.io/badge/license-MIT-blue)](LICENSE)
[![PlatformIO](https://img.shields.io/badge/PlatformIO-ESP32-orange)](https://platformio.org/)

ESPClaw is a lightweight AI assistant for ESP32 microcontroller, ported from [PicoClaw](https://github.com/sipeed/picoclaw) (Go). It provides LLM-powered assistance with minimal resource usage, optimized for embedded environments.

## Features

- **Lightweight**: Optimized for ESP32 (520KB SRAM, 4MB+ Flash) - Only ~50KB RAM usage
- **Multi-LLM Support**: OpenAI, Anthropic, OpenRouter, Zhipu (GLM), Groq, and custom endpoints
- **Tool System**: File operations, device control, system info
- **Persistent Storage**: Session history via LittleFS (auto-saved)
- **Serial Interface**: Interactive command-line interface with /help
- **WiFi Management**: Auto-connect and configuration via commands
- **Session Management**: Multi-session support with automatic cleanup

## Requirements

- ESP32 development board
- PlatformIO IDE or CLI
- WiFi network connection
- LLM API key (OpenAI, Anthropic, etc.)

## Project Status

| Phase | Status | Progress |
|-------|--------|----------|
| Phase 1: Foundation | ✅ Complete | 100% |
| Phase 2: LLM Integration | ✅ Complete | 100% |
| Phase 3: Tool System | ✅ Complete | 100% |
| Phase 4: Agent Core | ✅ Complete | 100% |
| Phase 5: Advanced Features | 🚧 In Progress | 0% |
| Phase 6: Testing & Docs | ⏳ Pending | 0% |

**Overall Progress**: ~80% (4 of 6 phases complete)

## Build Status

```
RAM:   [==        ]  15.1% (49,636 bytes from 327,680 bytes)
Flash: [========  ]  75.0% (983,061 bytes from 1,310,720 bytes)
```

## Quick Start

### 1. Install Dependencies

```bash
pio lib install
```

### 2. Configure

Edit `src/config.cpp` or use serial commands to set:

- WiFi SSID and password
- LLM API key and provider
- Model selection

### 3. Build and Upload

```bash
pio run --target upload
```

### 4. Monitor Serial

```bash
pio device monitor
```

## Serial Commands

| Command | Description |
|---------|-------------|
| `/chat <message>` | Send message to AI |
| `/config <key> <value>` | Set configuration |
| `/status` | Show system status |
| `/reboot` | Restart device |
| `/help` | Show available commands |

## Project Structure

- `include/` - Header files
- `src/` - Source files
- `lib/` - PlatformIO libraries
- `test/` - Unit tests
- `platformio.ini` - Build configuration

## Documentation

- [STATUS.md](STATUS.md) - Project status and progress
- [PROJECT_ARCHITECTURE.md](PROJECT_ARCHITECTURE.md) - System architecture
- [PROJECT_PLAN.md](PROJECT_PLAN.md) - Implementation plan
- [AGENTS.md](AGENTS.md) - Code style guidelines
- [GLM_LLM.md](GLM_LLM.md) - GLM API integration guide

## Configuration

Configuration is stored in `/config/config.json` on the device's flash filesystem:

```json
{
  "wifi": {
    "ssid": "YourWiFiSSID",
    "password": "YourPassword"
  },
  "agent": {
    "model": "gpt-3.5-turbo",
    "maxTokens": 2048
  },
  "providers": {
    "openai": {
      "apiKey": "your-api-key"
    }
  }
}
```

## Supported LLM Providers

- **OpenAI**: GPT models
- **Anthropic**: Claude models
- **OpenRouter**: Multi-model access
- **Zhipu**: GLM models
- **Groq**: Fast inference
- **Custom**: Any OpenAI-compatible endpoint

## Memory Usage

- Target: < 300KB RAM at peak
- Flash: ~200KB for firmware
- Storage: Sessions and config in LittleFS

## License

MIT License

## Credits

Based on [PicoClaw](https://github.com/sipeed/picoclaw) by Sipeed
Inspired by [NanoBot](https://github.com/HKUDS/nanobot)
