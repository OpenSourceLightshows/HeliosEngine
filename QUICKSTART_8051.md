# Helios Engine 8051 - Quick Start Guide

## 🚀 Getting Started in 5 Minutes

This guide will get you building and testing the 8051 version of Helios Engine quickly.

## Prerequisites Check

```bash
# Check if you have SDCC installed
sdcc --version
# Should output: SDCC 4.x.x or higher

# Check if you have git
git --version

# Check if you have make
make --version
```

If any of these fail, see the [Installation](#installation) section below.

## Build Your First 8051 Binary

```bash
# Clone or navigate to the repo
cd HeliosEngine/HeliosEmbedded

# Build for 8051
make -f Makefile.8051

# You should see:
# == Success building Helios v1.0.0 for 8051 ==
```

That's it! You now have `helios_8051.hex` ready to program.

## Test Without Hardware (CLI Mode)

```bash
# Build and test on your computer
cd ../HeliosCLI
make
./helios

# Try some commands:
# c = short click
# l = long click
# w = wait (tick forward)
# q = quit
```

## Program to Hardware

### Option 1: Using stcgal (STC chips)

```bash
cd HeliosEngine/HeliosEmbedded

# Auto-detect and program
make -f Makefile.8051 upload

# Or manually specify port
stcgal -p /dev/ttyUSB0 helios_8051.hex
```

### Option 2: Using Other Programmers

Consult your programmer's documentation. You'll need `helios_8051.hex`.

## Verify It Works

1. **Power on**: LED should turn on with default pattern
2. **Short click**: Cycles through modes
3. **Long hold + release**: Enter menus
4. **Very long hold**: Force sleep

If everything works, you're done! 🎉

---

## Installation

### SDCC Compiler

#### Ubuntu/Debian
```bash
sudo apt-get update
sudo apt-get install sdcc
```

#### macOS
```bash
brew install sdcc
```

#### Windows
1. Download from http://sdcc.sourceforge.net/
2. Run installer
3. Add to PATH

### stcgal Programmer (Optional)

For STC 8051 variants:

```bash
pip install stcgal
```

---

## Common Issues

### "sdcc: command not found"

**Fix**: Install SDCC (see above) and ensure it's in your PATH.

### "out of memory" error

**Fix**: You've used too much RAM. Move constants to flash:

```cpp
// Change this:
const uint8_t data[] = {1,2,3};

// To this:
const uint8_t data[] CODE_ATTR = {1,2,3};
```

### "undefined reference to..."

**Fix**: Check that all .cpp files are listed in Makefile.8051

### Device not responding

**Fixes**:
- Check power supply
- Verify connections
- Try slower baud rate
- Check if chip is code-protected

---

## Quick Reference

### Pin Connections

| Pin | Function | Connect To |
|-----|----------|------------|
| P1.0 | Red LED | Red LED anode (+ resistor) |
| P1.1 | Green LED | Green LED anode (+ resistor) |
| P1.2 | Blue LED | Blue LED anode (+ resistor) |
| P1.3 | Button | Button to GND (with pullup) |
| VCC | Power | 3.3V or 5V (check datasheet) |
| GND | Ground | Common ground |

### Build Commands

```bash
# Clean and build
make -f Makefile.8051 clean && make -f Makefile.8051

# Build with verbose output
make -f Makefile.8051 CFLAGS+="-debug"

# Just compile (no linking)
make -f Makefile.8051 main.rel
```

### Useful SDCC Flags

| Flag | Purpose |
|------|---------|
| `--opt-code-size` | Optimize for size |
| `--opt-code-speed` | Optimize for speed |
| `--debug` | Include debug symbols |
| `--verbose` | Show compilation steps |

---

## Next Steps

### Learn More
- Read `MIGRATION_8051.md` for technical details
- Check `README_8051.md` for full documentation
- Review `CA51Hardware.h` for hardware configuration

### Customize
1. Change LED pins in `CA51Hardware.h`
2. Add your own patterns in `Patterns.cpp`
3. Modify button timing in `HeliosConfig.h`

### Contribute
1. Test on your hardware
2. Report issues
3. Submit improvements
4. Share your patterns

---

## Help & Support

### Documentation
- `README_8051.md` - Full 8051 documentation
- `MIGRATION_8051.md` - Technical migration guide
- `ARCHITECTURE_CONVERSION_SUMMARY.md` - Complete overview

### Debugging

**Enable serial debug** (if supported):
```cpp
// In Helios.cpp init()
Serial.begin(115200);
Serial.println("Helios starting...");
```

**Check memory usage**:
```bash
# After building
sdcc --print-size helios_8051.ihx
```

**Verify hex file**:
```bash
# View hex contents
hexdump -C helios_8051.hex | less
```

### Getting Help

1. Check the FAQ in `README_8051.md`
2. Review troubleshooting in `MIGRATION_8051.md`
3. Check pin assignments in `CA51Hardware.h`
4. Verify clock speed matches your hardware

---

## Success Checklist

- [ ] SDCC installed and working
- [ ] Project builds without errors
- [ ] CLI tests pass
- [ ] Hex file generated
- [ ] Hardware connections verified
- [ ] Chip programmed successfully
- [ ] LED lights up
- [ ] Button responds
- [ ] Patterns switch correctly
- [ ] Sleep/wake works

If all boxes checked: **Congratulations!** 🎊

You've successfully built and deployed Helios Engine on 8051 architecture.

---

## Quick Tips

💡 **Use CLI for rapid testing** - Much faster than flashing hardware
💡 **Start simple** - Test basic LED on/off before complex patterns
💡 **Check RAM usage** - Keep an eye on that 256-byte limit
💡 **Use flash storage** - Put big const arrays in CODE space
💡 **Test sleep mode** - Verify power consumption is low

---

**Version**: 1.0.0
**Last Updated**: October 2025
**License**: See main project LICENSE file

