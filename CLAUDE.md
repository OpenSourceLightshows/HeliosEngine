# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Overview

Helios Engine is an embedded LED control system designed for the ATTiny85 microcontroller. The codebase is split into three main components:
- **Helios/**: Core engine code (C) shared between embedded and CLI
- **HeliosEmbedded/**: ATTiny85 firmware implementation
- **HeliosCLI/**: Command-line tool for simulation and testing (C++)
- **HeliosLib/**: Static library build of Helios core

## Build Commands

### Building the CLI Tool
```bash
cd HeliosCLI
make                    # Build the CLI executable
make clean              # Clean build artifacts
make bmps               # Generate BMP pattern visualizations
make pngs               # Generate PNG pattern visualizations
make svgs               # Generate SVG pattern visualizations
make clean_storage      # Delete Helios.storage file
```

### Building the Embedded Firmware
```bash
cd HeliosEmbedded
make                    # Build firmware for ATTiny85
make upload             # Compile and upload to ATTiny85 (sets fuses automatically)
make clean              # Clean build artifacts
```

### Building the Library
```bash
cd HeliosLib
make                    # Build static library
make wasm              # Build WebAssembly version
make clean             # Clean build artifacts
```

### Clock Speed Configuration
The ATTiny85 firmware supports 8MHz and 16MHz (1MHz not supported). Edit `HeliosEmbedded/Makefile` and set:
```makefile
CPU_SPEED = 8000000L    # For 8MHz
CPU_SPEED = 16000000L   # For 16MHz
```

### Fuse Settings
```bash
cd HeliosEmbedded
make set_8mhz_fuses     # Set fuses for 8MHz
make set_16mhz_fuses    # Set fuses for 16MHz
make get_fuses          # Read current fuse settings
```

## Testing

### Running All Tests
```bash
cd tests
./runtests.sh           # Run all tests
./runtests.sh -v        # Verbose mode
./runtests.sh -t=5      # Run specific test number
./runtests.sh -n        # Skip rebuild of helios executable
./runtests.sh -f        # Run with Valgrind for memory leak detection
```

### Test File Structure
Tests use `.test` files with this format:
```
Input=<command_sequence>
Brief=<test_description>
Args=<cli_arguments>
--------------------------------------------------------------------------------
<expected_output>
```

### CLI Input Commands
- `c` - Short click
- `l` - Long click
- `p` - Press and hold
- `r` - Release button
- `t` - Toggle press state
- `w` - Wait one tick (1ms at TICKRATE=1000)
- `q` - Quit simulation
- `[number]` - Repeat next command N times (e.g., `300w` = wait 300 ticks)

### Creating Tests
```bash
cd tests
./create_test.sh        # Interactive test creation
./record_test.sh <file> # Record a single test
./recordtests.sh        # Record multiple tests
```

## Architecture

### Core State Machine
Helios operates as a state machine defined in `Helios/Helios.h`:
- `STATE_MODES` - Normal mode operation
- `STATE_COLOR_SELECT_*` - Color selection menus (slot, quadrant, hue, sat, val)
- `STATE_PATTERN_SELECT` - Pattern selection menu
- `STATE_TOGGLE_CONJURE` - Toggle conjure mode (one-click on/off)
- `STATE_TOGGLE_LOCK` - Toggle light lock
- `STATE_SET_DEFAULTS` - Factory reset
- `STATE_SET_GLOBAL_BRIGHTNESS` - Brightness adjustment
- `STATE_SHIFT_MODE` - Mode slot shifting
- `STATE_RANDOMIZE` - Randomize colors/patterns

### Global Flags
Defined in `Helios/Helios.h`:
- `FLAG_LOCKED` - Device is locked, must unlock to turn on
- `FLAG_CONJURE` - Conjure mode enabled (single click toggles on/off)

### Key Components

**Pattern System** (`Helios/Pattern.h`, `Helios/Pattern.c`):
- Manages LED timing: on_dur, off_dur, gap_dur, dash_dur
- Supports blinking, blending (morphing), and complex dash patterns
- Works with colorsets to produce visual effects

**Colorset System** (`Helios/Colorset.h`, `Helios/Colorset.c`):
- Manages up to 6 colors per pattern (NUM_COLOR_SLOTS)
- Supports RGB and HSV color representations
- Pre-defined colors in `Helios/ColorConstants.h`

**Button Handling** (`Helios/Button.h`, `Helios/Button.c`):
- Detects short clicks (<400ms), long clicks, and holds (>1000ms)
- `SHORT_CLICK_THRESHOLD`, `MENU_HOLD_TIME`, `FORCE_SLEEP_TIME` in `HeliosConfig.h`

**Storage** (`Helios/Storage.h`, `Helios/Storage.c`):
- Persists 6 mode slots (NUM_MODE_SLOTS) and global flags
- EEPROM on ATTiny85 (256 bytes used of 512 available)
- CLI simulates storage with `Helios.storage` file

**LED Control** (`Helios/Led.h`, `Helios/Led.c`):
- Platform-specific RGB LED output
- Embedded: Direct ATTiny85 pin control
- CLI: Terminal color output or hex values

### Configuration Constants
All in `Helios/HeliosConfig.h`:
- `NUM_COLOR_SLOTS = 6` - Colors per pattern
- `NUM_MODE_SLOTS = 6` - Number of mode presets
- `TICKRATE = 1000` - Engine ticks per second
- `SHORT_CLICK_THRESHOLD = 400` - Short vs long click timing (ms)
- `MENU_HOLD_TIME = 1000` - Button hold time to open menus (ms)
- `FORCE_SLEEP_TIME = 7000` - Hold duration to force sleep (ms)
- `STORAGE_SIZE = 256` - EEPROM storage size (bytes)

### Version System
Version is automatically computed from git tags in the format `MAJOR.MINOR.BUILD`:
- Major/Minor from latest git tag
- Build number = last tag's build + commits since tag
- Defined at compile time via Makefile `compute_version` target

## Platform-Specific Code

**Embedded** (`HeliosEmbedded/main.c`):
- Initializes ATTiny85 hardware (LED pins, button, sleep modes)
- Main loop calls `helios_tick()` at TICKRATE
- Uses AVR sleep modes for power efficiency

**CLI** (`HeliosCLI/cli_main.cpp`):
- Simulates hardware with terminal I/O
- Reads input commands from stdin
- Outputs LED colors as hex or ANSI terminal colors
- Can generate BMP images of pattern sequences

## Common Development Workflows

### Adding a New Pattern
1. Define timing parameters in `Helios/Patterns.c` (on_dur, off_dur, gap_dur, etc.)
2. Add pattern to default pattern list
3. Rebuild CLI: `cd HeliosCLI && make`
4. Test pattern: `./helios -P <pattern_num> -C "red,blue"`
5. Create test in `tests/` for the new pattern
6. Generate visualization: `make bmps pngs`

### Modifying State Machine Behavior
1. State logic is in `Helios/Helios.c`
2. Each state has a handler function (e.g., `handle_state_modes()`)
3. Button events trigger state transitions
4. Test state transitions thoroughly with CLI before flashing to hardware

### Debugging with CLI
```bash
cd HeliosCLI
# Interactive mode with color output
./helios -c -i

# Test specific sequence
./helios -x <<< "300wcw300wcp1500wr300wq"

# Test with specific pattern and colors
./helios -P 1 -C "red,green,blue" -x <<< "500wq"
```

### Flashing Firmware
1. Connect ISP programmer to ATTiny85 (see README.md wiring)
2. Configure `HeliosEmbedded/Makefile` AVRDUDE settings for your programmer
3. Build and upload: `cd HeliosEmbedded && make upload`
4. First upload sets fuses automatically based on CPU_SPEED

## Important Notes

### Memory Constraints
ATTiny85 has only 8KB flash and 512 bytes SRAM. Code must be:
- Highly optimized (use `-Os`, `-flto`, `-ffunction-sections`)
- Avoid dynamic allocation
- Minimize string constants
- Use `uint8_t` and bitfields aggressively

### Timing Requirements
- Engine runs at TICKRATE (1000 ticks/sec = 1ms per tick)
- Pattern timings are in milliseconds/ticks
- Button timing constants must align with user expectations
- Sleep mode reduces power but adds wake latency

### Storage Format
- Each mode slot = 28 bytes (colorset + pattern args + CRC)
- Global flags = 1 byte
- Storage format version tied to HELIOS_VERSION_MAJOR
- Changing storage format requires major version bump

### Cross-Platform Compatibility
- Core Helios code is pure C for maximum portability
- Use `#ifdef HELIOS_CLI` for CLI-specific code
- Use `#ifdef HELIOS_EMBEDDED` for embedded-specific code
- Platform abstraction in LED and Button implementations

## CI/CD

GitHub Actions workflow (`.github/workflows/build.yml`):
- Builds on every push and PR to master branch
- Compiles both CLI and embedded firmware
- Runs full test suite
- Validates build on Linux
