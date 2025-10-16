# Helios Engine - 8051 Architecture Port (CA51F152XX)

## Overview

This directory contains the 8051 architecture port of the Helios LED Engine, specifically targeting the **CA51F152XX** microcontroller. This port replaces the original AVR (ATTiny85) architecture while maintaining full compatibility with the Helios software stack.

## Hardware Specifications

### CA51F152XX vs ATTiny85

| Feature | CA51F152XX (8051) | ATTiny85 (AVR) |
|---------|-------------------|----------------|
| Architecture | 8051 | AVR |
| Flash Memory | 16KB | 8KB |
| RAM | 256 bytes | 512 bytes |
| ADC Resolution | 12-bit | 10-bit |
| PWM Channels | 3 | 2 |
| Touch Sensing | Built-in capacitive | Not available |
| Clock Speed | 12MHz (typical) | 8MHz (typical) |

### Key Differences

1. **More Flash Memory**: The CA51F152XX has 16KB of flash vs 8KB on ATTiny85, allowing for more patterns and features.

2. **Less RAM**: The CA51F152XX has only 256 bytes of RAM vs 512 bytes on ATTiny85. This requires careful memory management:
   - Use `CODE_ATTR` to place constants in flash memory
   - Minimize stack usage
   - Be cautious with large local variables

3. **Built-in Touch Sensing**: The CA51F152XX includes capacitive touch sensing hardware, which could be used for button-free operation in future versions.

4. **Three PWM Channels**: Native support for RGB LED control with individual PWM channels.

## Pin Configuration

### Default Pin Assignments

- **P1.0**: Red LED channel (PWM0)
- **P1.1**: Green LED channel (PWM1)
- **P1.2**: Blue LED channel (PWM2)
- **P1.3**: Button input (with interrupt)

These pin assignments can be modified in `Helios/CA51Hardware.h`.

## Building the Firmware

### Prerequisites

1. **SDCC (Small Device C Compiler)**
   ```bash
   # Ubuntu/Debian
   sudo apt-get install sdcc

   # macOS
   brew install sdcc

   # Windows
   # Download from http://sdcc.sourceforge.net/
   ```

2. **Git** (for version tracking)

### Compilation

```bash
cd HeliosEmbedded
make -f Makefile.8051
```

This will generate `helios_8051.hex` which can be uploaded to the CA51F152XX.

### Build Output

The build process creates:
- `helios_8051.ihx` - Intel HEX format (intermediate)
- `helios_8051.hex` - Final hex file for programming
- `*.rel` files - Object files for each source
- `*.sym` files - Symbol files for debugging

## Programming the CA51F152XX

### Using stcgal (Recommended for STC chips)

```bash
make -f Makefile.8051 upload
```

Or manually:
```bash
stcgal -p /dev/ttyUSB0 helios_8051.hex
```

### Using Other Programmers

The CA51F152XX family may support different programming methods depending on the specific variant. Consult your chip's datasheet for supported programming interfaces.

## Memory Optimization for 8051

Due to the limited 256 bytes of RAM, the code uses several optimization strategies:

### 1. Flash Storage for Constants

Use the `CODE_ATTR` attribute for constant data:

```cpp
const uint8_t myArray[] CODE_ATTR = {1, 2, 3, 4, 5};
```

This places the array in code (flash) memory instead of RAM.

### 2. Minimize Stack Usage

- Avoid large local variables
- Use static or global variables for large buffers
- Be careful with recursive functions

### 3. Reentrant Functions

SDCC requires special handling for reentrant functions:

```cpp
void myFunction(void) REENTRANT {
    // Function code
}
```

## Architecture-Specific Code

### Conditional Compilation

The codebase uses architecture-specific defines:

```cpp
#ifdef HELIOS_8051
    // 8051-specific code
#elif defined(HELIOS_AVR)
    // AVR-specific code
#endif
```

### Key Architecture Files

- `Helios/CA51Hardware.h` - Hardware abstraction for CA51F152XX
- `Helios/HeliosConfig.h` - Configuration with 8051 support
- `HeliosEmbedded/Makefile.8051` - 8051 build configuration

## Features

### Supported Features

- ✅ Full RGB LED control via PWM
- ✅ Button input with debouncing
- ✅ Multiple mode storage in flash
- ✅ Color selection menus
- ✅ Pattern selection menus
- ✅ Power-down sleep mode with button wake
- ✅ Non-volatile pattern storage
- ✅ Brightness control

### Future Enhancements

- ⏳ Capacitive touch button support (hardware capability exists)
- ⏳ 12-bit ADC for advanced color sensing
- ⏳ Additional patterns leveraging extra flash space
- ⏳ Over-the-air updates

## Debugging

### SDCC Debug Symbols

Build with debug symbols:
```bash
make -f Makefile.8051 CFLAGS+="-debug"
```

### Serial Debug Output

The CA51F152XX supports UART communication. To enable serial debugging:

1. Add UART initialization in `Helios.cpp`
2. Connect a USB-to-serial adapter to the UART pins
3. Use a terminal program (115200 baud recommended)

## Troubleshooting

### Build Errors

**Error: "out of memory"**
- Solution: Reduce RAM usage by moving constants to flash with `CODE_ATTR`
- Check stack usage with `--stack-auto` flag

**Error: "undefined reference"**
- Solution: Ensure all source files are included in Makefile
- Check that function declarations match definitions

### Programming Errors

**Error: "Device not responding"**
- Check connections
- Verify power supply (3.3V or 5V depending on variant)
- Ensure programmer is compatible with CA51F152XX

**Error: "Verification failed"**
- Reduce baud rate
- Check for interference on programming lines
- Verify chip is not code-protected

## Performance Notes

### Clock Speed

The default configuration assumes a 12MHz system clock. If your CA51F152XX runs at a different speed:

1. Update `CPU_SPEED` in `Makefile.8051`
2. Adjust timer calculations in `TimeControl.cpp` if needed

### Power Consumption

The 8051 architecture typically has different power characteristics than AVR:

- Active mode: ~5-10mA (depends on clock speed)
- Idle mode: ~2-4mA
- Power-down mode: ~1-5µA

Actual values depend on the specific CA51F152XX variant and operating conditions.

## License

This code is part of the Helios LED Engine project and follows the same license as the main project.

## Contributing

When contributing to the 8051 port:

1. Test on actual CA51F152XX hardware
2. Ensure AVR compatibility is maintained
3. Document memory usage for RAM-intensive changes
4. Update this README for significant changes

## References

- [SDCC Documentation](http://sdcc.sourceforge.net/doc/sdccman.pdf)
- [8051 Architecture Reference](https://www.keil.com/dd/docs/datashts/intel/8051arch.pdf)
- CA51F152XX Datasheet (consult manufacturer)

## Support

For issues specific to the 8051 port, please include:
- CA51F152XX variant and clock speed
- SDCC version (`sdcc --version`)
- Build error messages or programming logs
- Hardware connections and programmer used

