# Helios Engine

## Introduction

![Build](https://github.com/OpenSourceLightshows/HeliosEngine/actions/workflows/build.yml/badge.svg?branch=master)
[![Latest Release](https://img.shields.io/github/v/tag/OpenSourceLightshows/HeliosEngine?label=Latest%20Release)](https://github.com/OpenSourceLightshows/HeliosEngine/releases/latest)

Welcome to the Helios Engine project! This repository contains the 'engine' and the 'firmware' for the Helios microlight. Helios Engine is the core infrastructure designed to control a single LED and produce a variety of patterns and effects, and the Helios microlight is the first example of Helios at work.

With it's core derived from [Vortex Engine](https://github.com/StoneOrbits/VortexEngine) but modified to accommodate for the ATTiny85's limited resources, Helios Engine puts a ***twist*** on Vortex Engine to create an envionment tailored for microlight development specifically on the Attiny85.

## Features

- Extensive list of available flashing patterns
  - Standard patterns (strobe, blink, glow, etc.)
  - Blending patterns for smooth color transitions
  - Morphing patterns:
    - Color morphing (blends between colors)
    - Duration morphing (dynamically shifts between on/off times while maintaining a constant total period)
- Customizable colorsets with up to 6 slots and over 260 colors
- Ability to shift modes
- Colorset/Pattern randomizer feature
- Light lock to prevent accidental activation
- Factory reset to restore chip settings
- Conjure mode for one-click on-off functionality

## Installation

To install and run this firmware on your ATTiny85, follow these steps:

1. **Prerequisites**: Ensure you have a working setup for programming an ATTiny85, including an ISP programmer.
2. **Clone the Repository**: Use `git clone` to download this repository to your local machine.
3. **Setup Toolchain**: Make sure you have AVR-GCC and AVRDUDE installed to compile and upload the firmware.
4. **Compile the Firmware**: Navigate to the firmware directory and run `make` to compile the firmware.
5. **Upload to ATTiny85**: Use AVRDUDE with your ISP programmer to upload the compiled firmware to the ATTiny85.

There are some make targets for uploading with avrdude, such as 'make upload', but you will need to manually configure the makefile to target the correct COM port.

## Project Structure

The project is organized into several key directories:

- `Helios/`: Contains the core firmware code shared between the embedded version and the CLI.
- `HeliosEmbedded/`: Contains code specific to the ATTiny85 implementation.
- `HeliosCLI/`: Contains the CLI tool implementation and related utilities.
- `tests/`: Contains integration tests and test scripts.


### Continuous Integration

The project uses GitHub Actions for continuous integration. The workflow defined in `.github/workflows/build.yml` automatically builds the project and runs the test suite on every push and pull request to the main branch.

## Uploading to ATTiny85

To upload the Helios Engine firmware to an ATTiny85 microcontroller, follow these steps:

### Prerequisites

1. An ATTiny85 chip
2. An ISP programmer (e.g., USBtinyISP, AVR ISP mkII, or Arduino as ISP)
3. AVRdude installed on your system

### Wiring

Connect your ISP programmer to the ATTiny85 as follows:

- MOSI to PIN 5 of ATTiny85
- MISO to PIN 6 of ATTiny85
- SCK to PIN 7 of ATTiny85
- RESET to PIN 1 of ATTiny85
- VCC to PIN 8 of ATTiny85
- GND to PIN 4 of ATTiny85

### Compiling the Firmware

1. Navigate to the `HeliosEmbedded` directory
2. Run `make` to compile the firmware

This will generate a `helio` file in the same directory.

### Uploading Using the Makefile

The project includes a Makefile with an `upload` target for convenience. To use it:

1. Open the Makefile in the `HeliosEmbedded` directory
2. Locate the `AVRDUDE` variable and ensure it's set correctly for your system and programmer
3. Run:

```bash
make upload
```

This will compile the firmware if necessary and then upload it to the ATTiny85.

### Troubleshooting

- Ensure your programmer is properly recognized by your system
- Double-check all connections between the programmer and the ATTiny85
- If you're using an Arduino as ISP, make sure you've uploaded the ArduinoISP sketch to it first

## Clock Speed and Compilation

The Helios Engine firmware is designed to work with ATTiny85 chips running at either 8 MHz or 16 MHz. The default clock speed is set to 8 MHz, but this can be easily changed in the Makefile.

### Setting the Clock Speed

To set the clock speed:

1. Open the `Makefile` in the `HeliosEmbedded` directory.
2. Locate the `F_CPU` variable.
3. Set it to either 8000000 for 8 MHz or 16000000 for 16 MHz.

For example:

```makefile
For 8 MHz
F_CPU = 8000000
For 16 MHz
F_CPU = 16000000
```

After changing the `F_CPU` variable, save the file and recompile the firmware using `make`.

### Compatibility Notes

- The firmware is fully compatible with both 8 MHz and 16 MHz clock speeds.
- Running the ATTiny85 at 1 MHz is not supported, as the firmware requires more processing power than a 1 MHz clock can provide.
- Using a 1 MHz clock will result in incorrect timing and may cause the firmware to malfunction.

### Compiling for Different Clock Speeds

After setting the desired clock speed in the Makefile, compile the firmware as usual:

```bash
make
```

This will generate a `helios.hex` file optimized for the specified clock speed.

### Fuse Settings for Clock Speed

Remember to set the appropriate fuse settings for your chosen clock speed:

- For 8 MHz (internal oscillator): Low Fuse should be 0xE2
- For 16 MHz (external crystal): Low Fuse should be 0xFF

You can set the fuses using avrdude as described in the "Uploading to ATTiny85" section.

### Performance Considerations

- 16 MHz will provide slightly better performance and more precise timing, but will consume considerably more power.
- 8 MHz is sufficient for most use cases and consumes less power.
- 1 MHz is not supported, as the firmware requires more processing power than a 1 MHz clock can provide.
- Choose the clock speed based on your specific requirements for performance and power consumption.

Remember to adjust your upload process and any timing-related code if you change the clock speed. The `F_CPU` definition ensures that timing-dependent functions (like delays) are calculated correctly for the chosen clock speed.

