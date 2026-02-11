# AGENTS.md

This file contains guidelines for agentic coding assistants working in this ESP32 PlatformIO project.

## Build, Test, and Lint Commands

### Build
```bash
pio run
```

### Upload to Device
```bash
pio run --target upload
```

### Clean Build
```bash
pio run --target clean
```

### Run Tests
```bash
pio test
```

### Run Single Test
```bash
pio test --filter=test_name
```

### Serial Monitor
```bash
pio device monitor
```

### Linting/Type Checking
This C++ project does not have formal lint/typecheck commands. Follow the style guidelines below.

## Project Structure
- `src/` - Main source files (.cpp/.ino)
- `include/` - Header files (.h) shared across project
- `lib/` - Project-specific libraries
- `test/` - Unit tests using PlatformIO Unity framework
- `platformio.ini` - PlatformIO configuration

## Code Style Guidelines

### Imports and Headers
- Use `#include` for local headers: `#include "myheader.h"`
- Use `#include <>` for system/external libraries: `#include <Arduino.h>`
- Include guards format: `#ifndef PROJECT_FILENAME_H` / `#define PROJECT_FILENAME_H` / `#endif`
- Order: Arduino headers, local headers, standard C++ headers

### Naming Conventions
- **Classes/Structs**: `PascalCase` (e.g., `MotorController`)
- **Functions/Methods**: `camelCase` (e.g., `readSensor()`)
- **Variables**: `camelCase` (e.g., `sensorValue`)
- **Constants**: `UPPER_SNAKE_CASE` (e.g., `MAX_PIN_VALUE`)
- **Private members**: `camelCase_` trailing underscore
- **Files**: `snake_case` for headers/sources (e.g., `motor_driver.h`)

### Formatting
- Use 2-space indentation
- Maximum line length: 120 characters
- Use braces for all control structures (even single-line)
- Space after keywords: `if (condition) {` not `if(condition){`
- Space around operators: `x = y + z`

### Types
- Use PlatformIO/ESP32 types: `uint8_t`, `uint16_t`, `uint32_t`, `int32_t`
- Prefer `const` references for complex types
- Use `constexpr` for compile-time constants
- Avoid `void*` - use specific types or templates

### Error Handling
- Return error codes from functions; document possible values
- Use `Serial.print()` or `Serial.println()` for error messages
- Check return values from API calls (e.g., Wire.endTransmission())
- Use assertions in debug builds for critical invariants

### ESP32/Arduino Specific
- Initialize hardware in `setup()`
- Main logic in `loop()` - keep it non-blocking when possible
- Use `millis()` for timing, not `delay()` in production code
- FreeRTOS tasks: Create with sufficient stack size
- Use `xSemaphoreTake()` / `xSemaphoreGive()` for mutex access
- Memory management: Prefer static allocation; avoid dynamic allocation in critical paths

### Comments
- Use `//` for single-line comments above code
- Use `/* */` only for multi-line file headers or blocks
- Document public functions with parameter descriptions
- No inline comments stating the obvious

### PlatformIO Config
- Environment: `[env:esp32dev]`
- Platform: `espressif32`
- Framework: `arduino`
- Board: `esp32dev`

### Testing
- Use PlatformIO Unity framework for unit tests
- Place tests in `test/` directory
- Test files: `test_*.cpp`
- Mock hardware dependencies where appropriate

## LSP Support

### For compile_commands.json
```
pio run -t compiledb
```

### For .clangd
Create a .clangd file 
```
CompileFlags:
  Add: [-Qunused-arguments]
  Remove: [-mlongcalls, -fstrict-volatile-bitfields, -fno-tree-switch-conversion]
```

