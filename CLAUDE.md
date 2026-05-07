# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Helios Engine is an ATTiny85-based RGB LED controller firmware derived from Vortex Engine. It provides pattern generation, color management, and menu navigation through a single-button interface. The project includes both embedded firmware and a CLI tool for testing.

## Build Commands

### Embedded Firmware (ATTiny85)
```bash
cd HeliosEmbedded
make                    # Build firmware (generates helios.hex)
make upload             # Build and upload to ATTiny85 via ISP programmer
make clean              # Clean build artifacts
make set_fuses          # Set fuses based on CPU_SPEED (8MHz/16MHz/1MHz)
make extract_version    # Read firmware version from chip
make helios_release VERSION=x.y.z  # Create git tag and trigger GitHub release
```

### CLI Tool
```bash
cd HeliosCLI
make                    # Build CLI tool (generates helios executable)
make clean              # Clean build artifacts
make clean_storage      # Remove Helios.storage file
make pngs               # Generate BMP + PNG visualizations of all patterns
make svgs               # Generate SVG visualizations from BMP files
```

### HeliosLib (Desktop / WASM)
```bash
cd HeliosLib
make                    # Build helios.a static library
make wasm               # Build HeliosLib.js + HeliosLib.wasm via Emscripten (em++)
make clean              # Clean build artifacts
```

### Testing
```bash
cd tests
./runtests.sh           # Run all tests
./runtests.sh -v        # Verbose mode
./runtests.sh -t=5      # Run specific test number
./runtests.sh -n        # Skip rebuild before testing
./runtests.sh -f        # Run with Valgrind (memory leak detection)
./create_test.sh        # Create new test interactively
./record_test.sh        # Record test output
```

## Architecture

### Core Components (`Helios/` directory)

The engine uses a modular architecture with shared code between embedded and CLI builds:

- **Helios**: Main controller managing system state, menus, and mode transitions
- **Pattern**: Instance holding a `PatternArgs` + `Colorset`; runs the blink/blend state machine each tick
- **Patterns**: Factory class — `Patterns::make_pattern(id, pat)` constructs named patterns; `make_default(index, pat)` loads slot defaults
- **Colorset**: Manages up to 6 colors per pattern
- **Button**: Handles input timing detection (short/long/hold clicks)
- **Led**: Controls RGB LED output (PWM on embedded, simulated in CLI)
- **Storage**: EEPROM persistence (256 bytes on ATTiny85, file-based in CLI)
- **Time** (`TimeControl.h`): Global tick counter (1 tick = 1ms); `Time::getCurtime()` is the engine clock
- **Timer** (`Timer.h`): Per-pattern single-alarm timer used inside `Pattern::play()` to drive blink timing
- **Random**: PRNG used by the randomize feature

### State Machine

Helios operates as a state machine with states defined in `Helios.h`:
- `STATE_MODES`: Normal mode playback
- `STATE_COLOR_SELECT_*`: Color picker menu (slot → quadrant → hue/sat/val)
- `STATE_PATTERN_SELECT`: Pattern selection menu
- `STATE_TOGGLE_*`: Feature toggles (conjure mode, lock)
- `STATE_SET_DEFAULTS`: Factory reset
- `STATE_SHIFT_MODE`: Mode cycling
- `STATE_RANDOMIZE`: Randomize pattern/colors

### Button Interface Timing

Button timing is critical (defined in `HeliosConfig.h`):
- Short click: < 400ms (cycle modes, menu navigation)
- Long click: 400-1000ms (confirm selection)
- Hold: > 1000ms (open menus, cycle menu options)
- Force sleep: > 7000ms (exit menus without saving)

### Storage Layout

EEPROM stores 6 mode slots, each containing:
- Pattern parameters (6 bytes): on_dur, off_dur, gap_dur, dash_dur, group_size, blend_speed
- Colorset: up to 6 RGB colors (18 bytes) + count (1 byte)
- Pattern flags (1 byte)
- CRC (1 byte)
Total: 27 bytes per slot = 162 bytes for 6 slots

### Pattern Parameters

Each pattern is defined by timing values in milliseconds:
- `on_dur`: LED on duration
- `off_dur`: LED off duration
- `gap_dur`: Gap between color cycles
- `dash_dur`: Dash element length
- `group_size`: Colors to group together
- `blend_speed`: Morph/blend transition speed (0 = discrete colors, >0 = blend)

## Testing System

Tests use a custom format (`.test` files in `tests/tests/`):

```
Input=<command_sequence>
Brief=<description>
Args=<cli_arguments>
--------------------------------------------------------------------------------
<expected_hex_output>
```

### Input Commands
- `c`: Short click
- `l`: Long click
- `p`: Press and hold
- `r`: Release
- `t`: Toggle press
- `w`: Wait 1 tick (1ms)
- `q`: Quit
- `[number]`: Repeat next command (e.g., `300w` = wait 300ms)

Example: `300wc300wc300wq` waits 300ms, clicks, waits 300ms, clicks, waits 300ms, quits.

## Build System Details

### Version Numbering

Version is auto-computed from git tags in format `x.y.z`:
- Major version (x): Save format changes
- Minor version (y): Bugfixes/features maintaining save compatibility
- Build number (z): Commits since last tag

### CPU Speed Configuration

Set in `HeliosEmbedded/Makefile` via `CPU_SPEED`:
- `8000000L` (8MHz, default): Low power, internal oscillator
- `16000000L` (16MHz): Better timing, requires external crystal
- `1000000L` (1MHz): Not recommended, insufficient for firmware

Fuses are automatically set via `make set_fuses` based on `CPU_SPEED`.

### Platform-Specific Builds

Makefiles detect OS (Windows/Linux/Darwin) and configure toolchain paths. On macOS, distinguishes between Intel and Apple Silicon for Homebrew paths.

## CLI Tool Usage

The `helios` CLI simulates the embedded firmware for testing:

```bash
./helios [options] < input_commands
```

Options:
- `-x`: Hex color output (FF0000)
- `-c`: Console color codes
- `-q`: Quiet mode
- `-l`: Lockstep mode (step only on input)
- `-i`: In-place output (interactive)
- `-C <colors>`: Initial colorset (e.g., "red,green,0x0000ff")
- `-P <pattern>`: Initial pattern (index or name)
- `-b <file>`: Generate BMP output

## Hardware Specifications

- **MCU**: ATTiny85 (8KB flash, 512B SRAM, 512B EEPROM)
- **LED**: RGB common cathode (120Ω resistors per channel)
- **Power**: 2x CR1620 coin cells (6V total)
- **Button**: Tactile switch with 10kΩ pull-up/down
- **Board**: 16mm x 17mm

## Development Notes

### ATTiny85 Resource Constraints

- Only lower 256 bytes of EEPROM used (upper half has flash limitations)
- Compiler optimizations critical: `-Os -flto -ffunction-sections -fdata-sections`
- Short enums and packed structs reduce memory footprint
- No exceptions, no threadsafe statics

### Conditional Compilation

- `HELIOS_EMBEDDED`: Embedded-specific code (timers, LED PWM, sleep)
- `HELIOS_CLI`: CLI-specific code (file I/O, console output)
- Code in `Helios/` must work for both targets

### Color System

Colors support RGB and HSV representations:
- RGB: Direct hardware output
- HSV: Used in color picker menu with configurable saturation/value presets
- 20 pre-defined colors in `ColorConstants.h` organized in 5 pages

### Special Features

- **Conjure Mode**: Single-click on/off toggle
- **Light Lock**: Prevents accidental activation
- **Autoplay**: Auto-cycle through modes
- **Factory Reset**: Restore defaults via menu
- **Global Brightness**: 4 levels (255/170/85/30)
