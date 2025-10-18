# Helios 8051 Port (CA51F152XX)

This directory contains the 8051 architecture port of the Helios LED control engine, specifically targeting the CA51F152XX microcontroller.

## Overview

The Helios engine has been ported from AVR ATtiny85 to the 8051-based CA51F152XX chip. This port maintains full compatibility with the core Helios functionality while adapting to the different architecture.

### Key Differences from ATtiny85

| Feature | ATtiny85 | CA51F152XX |
|---------|----------|------------|
| Architecture | AVR | 8051 |
| Flash Memory | 8KB | 16KB |
| RAM | 512 bytes | 256 bytes |
| EEPROM | 512 bytes | None (flash emulation) |
| ADC Resolution | 10-bit | 12-bit |
| PWM Channels | 2 (hardware) | 3 (PCA) |

## Hardware Configuration

### Pin Assignments

#### LED Outputs (RGB)
- **P1.0** - Red channel
- **P1.1** - Green channel
- **P1.2** - Blue channel

#### Button Input
- **P3.3** - Button input (INT1)

### Memory Layout

#### Flash Memory (16KB)
- `0x0000-0x3EFF` - Program code (15.75KB)
- `0x3F00-0x3FFF` - Storage area (256 bytes)

#### RAM (256 bytes)
- Internal RAM: 256 bytes
- Uses `__xdata` for storage buffer
- Const data stored in flash using `__code` attribute

## Building

### Prerequisites

1. **SDCC Compiler** (Small Device C Compiler)
   ```bash
   # macOS
   brew install sdcc

   # Ubuntu/Debian
   sudo apt-get install sdcc

   # Windows
   # Download from http://sdcc.sourceforge.net/
   ```

2. **Flash Programmer**
   - stcgal (recommended for STC MCUs)
   - Or your specific programmer tool

### Build Commands

```bash
cd Helios8051

# Build the firmware
make

# Build and upload to chip
make upload

# Clean build artifacts
make clean
```

### Compiler Settings

The Makefile uses SDCC with these optimizations:
- `--model-small` - Small memory model (256 bytes internal RAM)
- `--opt-code-size` - Optimize for code size
- `--stack-auto` - Automatic stack allocation
- `--xram-size 256` - External RAM size
- `--code-size 16384` - 16KB flash

## Memory Optimization

Due to the limited 256 bytes of RAM (half of ATtiny85), several optimizations were implemented:

### 1. Flash Storage for Constants

All constant data uses the `__code` attribute to store in flash instead of RAM:

```c
#ifdef HELIOS_8051
#define FLASH_CONST __code
#else
#define FLASH_CONST
#endif

static const uint32_t FLASH_CONST color_codes[] = {...};
```

### 2. Storage Buffer in XDATA

The 256-byte storage buffer uses external RAM:

```c
static uint8_t __xdata storage_buffer[STORAGE_SIZE];
```

### 3. Stack Optimization

Uses `--stack-auto` to minimize static memory allocation.

## Flash Storage Implementation

Unlike the ATtiny85 which has EEPROM, the CA51F152XX uses flash memory for persistent storage:

- **Storage Area**: Last 256 bytes of flash (0x3F00-0x3FFF)
- **Implementation**: RAM buffer with flash sync on writes
- **Note**: The flash write function requires vendor-specific programming sequences

### TODO: Implement Flash Programming

The `storage_flash_write()` function is currently a stub. You need to implement the actual flash programming sequence based on the CA51F152XX datasheet:

1. Disable interrupts
2. Unlock flash controller (IAP_CONTR)
3. Set target address (IAP_ADDRH/IAP_ADDRL)
4. Write data (IAP_DATA)
5. Trigger write command (IAP_CMD)
6. Wait for completion
7. Re-enable interrupts

## LED Control

### Current Implementation

The current implementation uses simple GPIO on/off based on brightness threshold:

```c
if (brightness > 127) {
    P1 |= (1 << pin);  // LED on
} else {
    P1 &= ~(1 << pin); // LED off
}
```

### Hardware PWM Enhancement

For true PWM control, you can implement the PCA (Programmable Counter Array):

1. Configure PCA module for PWM mode
2. Set PWM duty cycle using CCAPxH/CCAPxL registers
3. Enable PCA counter

Example (pseudo-code):
```c
// Configure PCA for PWM
CMOD = 0x00;           // PCA timer source
CL = 0x00;
CH = 0x00;
CCAPM0 = 0x42;         // PWM mode
CCAP0L = pwm_value;    // Set duty cycle
CR = 1;                // Start PCA
```

## Interrupts

### External Interrupt 1 (Button Wake)

```c
void int1_isr(void) __interrupt(2) {
    // Disable interrupt
    EX1 = 0;
    helios_wakeup();
}
```

### Timer Interrupts

- **Timer0**: Available for PWM/timekeeping
- **Timer1**: Available for timekeeping
- Interrupts enabled in `helios_init()`

## Power Management

The 8051 supports several power modes:

- **Normal Mode**: All peripherals active
- **Idle Mode**: CPU stopped, peripherals running
- **Power Down Mode**: All stopped, wake on interrupt

To implement power saving similar to ATtiny85:

```c
// Enter idle mode
PCON |= 0x01;  // Set IDL bit

// Enter power down mode
PCON |= 0x02;  // Set PD bit
```

## Uploading Firmware

### Using stcgal

```bash
# Upload via USB-to-serial adapter
stcgal -p /dev/ttyUSB0 -b 115200 helios.hex
```

### Configuration

Edit the Makefile to match your programmer:

```makefile
PROGRAMMER = stcgal
PROGRAMMER_PORT = /dev/ttyUSB0
PROGRAMMER_BAUD = 115200
```

## Troubleshooting

### Build Errors

**"undefined reference to `__mullong`"**
- SDCC requires library linking for 32-bit operations
- Add `-L /path/to/sdcc/lib` to LDFLAGS

**"code memory overflow"**
- Reduce code size by disabling debug features
- Use `--opt-code-size` flag
- Remove unused patterns/colors

### RAM Overflow

**"external RAM overflow"**
- Reduce STORAGE_SIZE if needed
- Move more constants to flash using `__code`
- Reduce NUM_MODE_SLOTS or NUM_COLOR_SLOTS

### Flash Programming Issues

If storage doesn't persist across power cycles:
- Implement proper `storage_flash_write()` function
- Verify flash address range (0x3F00-0x3FFF)
- Check IAP unlock sequence in datasheet

## Development Notes

### Differences from AVR Implementation

1. **Interrupts**: Use `__interrupt(n)` instead of `ISR()` macro
2. **I/O Ports**: Use `P0-P3` instead of `PORTB/DDRB`
3. **Flash Access**: Use `__code` pointers instead of `PROGMEM`
4. **No EEPROM**: Flash programming required for persistence
5. **Memory Model**: Explicit `__xdata`, `__idata`, `__code` qualifiers

### Testing

The CLI tool (`HeliosCLI/`) can still be used for testing patterns and logic before flashing to hardware:

```bash
cd ../HeliosCLI
make
./helios -c -i  # Interactive mode with color output
```

## Future Enhancements

1. **Hardware PWM**: Implement PCA-based PWM for smooth LED control
2. **Flash Programming**: Complete the IAP flash write implementation
3. **Power Optimization**: Add idle/power-down modes
4. **ADC Integration**: Use 12-bit ADC for enhanced features
5. **Additional Timers**: Leverage Timer2 if available

## References

- SDCC User Manual: http://sdcc.sourceforge.net/doc/sdccman.pdf
- 8051 Instruction Set: https://www.keil.com/support/man/docs/is51/
- CA51F152XX Datasheet: [Obtain from chip vendor]

## License

Same as main Helios project.
