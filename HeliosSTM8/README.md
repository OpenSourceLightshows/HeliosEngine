# Helios Engine - STM8S001J3M3TR Port

This directory contains the STM8S001J3M3TR microcontroller implementation of the Helios Engine.

## Current Status

**✅ Ready for Hardware Testing**: The STM8 port compiles successfully with SDCC and is ready for testing on actual hardware.

**Completed:**
- ✅ Directory structure and Makefile
- ✅ STM8 hardware initialization (GPIO, timers, interrupts)
- ✅ Platform-specific LED PWM control
- ✅ Platform-specific button input handling
- ✅ Integration with core Helios engine
- ✅ AVR-specific code properly isolated with `#ifdef` guards
- ✅ SDCC compatibility (resolved typedef and struct-return issues)
- ✅ Storage optimized for 128-byte EEPROM (4 mode slots instead of 6)
- ✅ STM8 EEPROM read/write functions
- ✅ Complete successful build with no errors

**Pending:**
- ⚠️ Flash and test on actual STM8S001J3M3TR hardware
- ⚠️ Verify EEPROM read/write operations
- ⚠️ Test button input and LED output
- ⚠️ Implement proper low-power sleep modes
- ⚠️ Optimize delay functions (currently busy-wait)

## Hardware Specifications

**MCU:** STM8S001J3M3TR
- **Flash:** 8KB
- **RAM:** 1KB
- **EEPROM:** 128 bytes
- **Clock:** 16MHz internal RC oscillator
- **Package:** SO-8 (8-pin surface mount)

## Pin Configuration (SO-8 Package)

```
         STM8S001J3M3TR
         ┌─────────────┐
PD1/SWIM │1           8│ PD4 (Available)
     PD5 │2    TOP    7│ VDD (Power 3.3V)
     PD6 │3           6│ PB5 (LED Blue)
     VSS │4_Ground____5│ PD3 (LED Red)
         └─────────────┘
```

### Default Pin Assignment

| Pin | Port | Function      | Notes                           |
|-----|------|---------------|---------------------------------|
| 1   | PD1  | SWIM          | Programming interface (do not use) |
| 2   | PD5  | Button Input  | Active HIGH with internal pull-up |
| 3   | PD6  | LED Green     | PWM via Timer 1 Channel 2       |
| 4   | VSS  | Ground        | Connect to ground               |
| 5   | PD3  | LED Red       | PWM via Timer 1 Channel 1       |
| 6   | PB5  | LED Blue      | PWM via Timer 2 Channel 1       |
| 7   | VDD  | Power         | 3.3V supply (2.95V - 5.5V)     |
| 8   | PD4  | Available     | General purpose I/O             |

**Note:** Pin assignments can be customized by editing the `Makefile` pin configuration section.

## RGB LED Connection

Connect a common cathode RGB LED:
- **Red anode** → 150Ω resistor → Pin 5 (PD3)
- **Green anode** → 150Ω resistor → Pin 3 (PD6)
- **Blue anode** → 150Ω resistor → Pin 6 (PB5)
- **Common cathode** → Ground (Pin 4)

## Button Connection

Connect a momentary push button:
- One side → Pin 2 (PD5)
- Other side → VDD (Pin 7)

The internal pull-up is disabled; the button should pull the pin HIGH when pressed.

## Building the Firmware

### Prerequisites

Install SDCC (Small Device C Compiler):

```bash
# macOS
brew install sdcc

# Ubuntu/Debian
sudo apt-get install sdcc

# Windows
# Download from: http://sdcc.sourceforge.net/
```

Install stm8flash for programming:

```bash
# macOS
brew install stm8flash

# Ubuntu/Debian
sudo apt-get install stm8flash

# From source
git clone https://github.com/vdudouyt/stm8flash.git
cd stm8flash
make
sudo make install
```

### Compile

```bash
cd HeliosSTM8
make
```

This produces `helios.ihx` (Intel HEX format) ready for flashing.

### Flash to Device

Connect an ST-LINK/V2 programmer to the SWIM interface:
- **ST-LINK GND** → STM8 Pin 4 (VSS)
- **ST-LINK 3.3V** → STM8 Pin 7 (VDD)
- **ST-LINK SWIM** → STM8 Pin 1 (PD1/SWIM)
- **ST-LINK RST** → STM8 Pin 8 (PD4) or leave unconnected

Flash the firmware:

```bash
make flash
# or
make upload
```

## Memory Constraints

The STM8S001J3M3TR has very limited resources:
- **8KB Flash** - All code must fit
- **1KB RAM** - Very tight memory budget
- **128 bytes EEPROM** - Half the ATtiny85's EEPROM

### Optimization Notes

- Code is compiled with `--opt-code-size` for maximum size optimization
- **Storage format optimized:** 4 mode slots instead of 6 (uses 112 bytes of 128-byte EEPROM)
- Pattern and color data structures unchanged from AVR version
- No dynamic memory allocation used
- Extensive use of `uint8_t` to save RAM
- SDCC-specific workarounds for struct-return functions (uses pointer parameters)

## Differences from AVR Version

1. **Compiler:** Uses SDCC instead of AVR-GCC
2. **Interrupts:** Different syntax (`__interrupt(N)` instead of `ISR()`)
3. **Timers:** STM8 Timer 1 and Timer 2 for PWM (different registers)
4. **Clock:** 16MHz internal RC (no external crystal needed)
5. **EEPROM:** 128 bytes (half of ATtiny85)
6. **Mode Slots:** 4 mode slots instead of 6 (storage optimization)
7. **Struct Returns:** Uses pointer-based functions instead of struct returns (SDCC limitation)

## Troubleshooting

### Build Issues

**Error: `sdcc: command not found`**
- Install SDCC compiler

**Error: Cannot find register definitions**
- Ensure STM8 headers are installed with SDCC
- Check that `-mstm8` flag is present in CFLAGS

### Programming Issues

**Error: `stm8flash: command not found`**
- Install stm8flash utility

**Error: Cannot connect to device**
- Check SWIM connection (Pin 1)
- Verify power is connected (Pin 7 = VDD, Pin 4 = GND)
- Try holding reset (if connected) while programming
- Ensure ST-LINK firmware is up to date

**Error: Device is read-protected**
```bash
# Unlock the device (WARNING: erases all flash)
stm8flash -c stlinkv2 -p stm8s001j3 -u
```

## Development Workflow

1. Edit code in `main.c`, `stm8_init.c`, or core `../Helios/` files
2. Build: `make clean && make`
3. Flash: `make upload`
4. Test with physical button and LED

## Future Enhancements

- [ ] Low-power sleep modes
- [ ] Watchdog timer support
- [ ] EEPROM wear leveling
- [ ] Optimized storage format for 128-byte EEPROM
- [ ] Support for alternative pin configurations

## References

- [STM8S001J3 Datasheet](https://www.st.com/resource/en/datasheet/stm8s001j3.pdf)
- [STM8S Reference Manual](https://www.st.com/resource/en/reference_manual/cd00190271-stm8s-series-and-stm8af-series-8bit-microcontrollers-stmicroelectronics.pdf)
- [SDCC Compiler Manual](http://sdcc.sourceforge.net/doc/sdccman.pdf)
- [stm8flash Repository](https://github.com/vdudouyt/stm8flash)
