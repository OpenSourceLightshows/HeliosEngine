# STM8S001J3M3TR Implementation Notes

## Overview

This document describes the STM8 port of the Helios Engine and the platform-specific implementations.

## Architecture

The STM8 port follows the same structure as the AVR (ATtiny85) port:

```
HeliosSTM8/
├── Makefile              # SDCC-based build system
├── main.c                # Entry point, calls helios_init() and helios_tick()
├── stm8_init.c/.h        # Hardware initialization (clocks, GPIO, timers, interrupts)
└── README.md             # Hardware specs and build instructions
```

## Hardware Initialization

### Clock System
- Configured for 16MHz internal RC oscillator
- No external crystal required
- Peripheral clocks enabled for all modules

### GPIO Configuration
- **LED Outputs:** PD3 (Red), PD6 (Green), PB5 (Blue)
  - Configured as push-pull outputs
  - Fast mode enabled (10MHz)

- **Button Input:** PD5
  - Configured as input with pull-up
  - External interrupt enabled for wake-from-sleep

### PWM Timers
- **Timer 1:** Controls Red (PD3/CH1) and Green (PD6/CH2) LEDs
  - Prescaler: /16 (16MHz → 1MHz)
  - Auto-reload: 255 (3.9kHz PWM frequency)
  - PWM Mode 1, 8-bit resolution

- **Timer 2:** Controls Blue (PB5/CH1) LED
  - Prescaler: /16 (16MHz → 1MHz)
  - Auto-reload: 255 (3.9kHz PWM frequency)
  - PWM Mode 1, 8-bit resolution

### Interrupts
- External interrupt on PD5 (button)
  - Rising and falling edge detection
  - Wakes device from WFI (Wait For Interrupt) low-power mode

## Platform-Specific Code

### [Led.cpp](../Helios/Led.cpp)

Added STM8-specific PWM output in `led_update()`:
```c
#elif defined(HELIOS_STM8)
  // STM8 PWM output using Timer 1 and Timer 2
  TIM1_CCR1L = m_realColor.red;    // Red on PD3
  TIM1_CCR2L = m_realColor.green;  // Green on PD6
  TIM2_CCR1L = m_realColor.blue;   // Blue on PB5
#endif
```

### [Button.cpp](../Helios/Button.cpp)

Added STM8-specific button reading and interrupt handler:
```c
#elif defined(HELIOS_STM8)
  // Read button state from PD5
  #define PD_IDR (*(volatile uint8_t *)0x5010)
  return (PD_IDR & (1 << BUTTON_PIN)) != 0;
#endif
```

Interrupt handler:
```c
#ifdef HELIOS_STM8
void button_exti_isr(void) __interrupt(3) {
  helios_wakeup();
}
#endif
```

### [Helios.cpp](../Helios/Helios.cpp)

Wrapped AVR-specific includes and hardware initialization:
```c
#ifdef HELIOS_EMBEDDED
#ifndef HELIOS_STM8
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#endif
#endif
```

Added STM8 sleep mode using WFI instruction:
```c
#ifdef HELIOS_STM8
  led_clear();
  button_enable_wake();
  __asm__("wfi");  // Wait For Interrupt
  helios_init_components();
#endif
```

## Memory Constraints

### STM8S001J3M3TR vs ATtiny85

| Resource | ATtiny85 | STM8S001J3 | Notes |
|----------|----------|------------|-------|
| Flash    | 8KB      | 8KB        | Same |
| RAM      | 512B     | 1KB        | 2x more RAM |
| EEPROM   | 512B     | 128B       | 1/4 the EEPROM |

### Impact on Storage

The 128-byte EEPROM limit means:
- **ATtiny85:** 6 mode slots × 28 bytes = 168 bytes (fits in 512B)
- **STM8:** May need to reduce to 4 mode slots or optimize storage format

Potential optimizations:
1. Reduce `NUM_MODE_SLOTS` from 6 to 4
2. Compress pattern data structure
3. Use bit-packing for boolean flags
4. Reduce color resolution from 8-bit to 6-bit per channel

## Compiler Differences

### SDCC vs AVR-GCC

| Feature | AVR-GCC | SDCC | Status |
|---------|---------|------|--------|
| File extensions | `.c` and `.cpp` | `.c` only | ⚠️ Using `-x c` flag |
| Forward typedefs | Flexible | Strict | ⚠️ Needs refactoring |
| Interrupt syntax | `ISR(VECTOR)` | `__interrupt(N)` | ✅ Implemented |
| Inline assembly | `asm()` | `__asm__()` | ✅ Implemented |
| Sleep mode | `sleep_mode()` | `wfi` | ✅ Implemented |

### Known SDCC Issues

1. **Multiple typedef declarations:** SDCC doesn't allow duplicate `typedef struct X X;` even as forward declarations across different headers. The codebase has this pattern in:
   - `Helios.h` (pattern_t, colorset_t)
   - `Pattern.h` (pattern_t)
   - `Storage.h` (pattern_t)
   - `Patterns.h` (pattern_t)
   - `Random.h` (random_t)

2. **C++ file extensions:** SDCC doesn't recognize `.cpp` files. Using `-x c` flag to force C compilation.

3. **Strict C99 compliance:** SDCC is more strict about C standard compliance than GCC.

## Next Steps

To complete the STM8 port:

1. **Refactor headers:** Create a single `HeliosTypes.h` with all forward declarations, or use `#ifndef` guards around typedefs to prevent redefinition errors

2. **Test EEPROM:** Implement and test STM8 EEPROM read/write functions in `Storage.cpp`

3. **Optimize storage:** Reduce storage format to fit in 128 bytes

4. **Memory profiling:** Verify RAM usage fits within 1KB budget

5. **Hardware testing:** Flash to actual STM8S001J3M3TR and verify:
   - RGB LED PWM output
   - Button input and debouncing
   - Pattern playback
   - Sleep/wake functionality

## References

- [STM8S001J3 Datasheet](https://www.st.com/resource/en/datasheet/stm8s001j3.pdf)
- [STM8S Reference Manual RM0016](https://www.st.com/resource/en/reference_manual/cd00190271-stm8s-series-and-stm8af-series-8bit-microcontrollers-stmicroelectronics.pdf)
- [SDCC Compiler Manual](http://sdcc.sourceforge.net/doc/sdccman.pdf)
- [SDCC STM8 Specifics](http://sdcc.sourceforge.net/doc/sdccman.pdf#subsection.3.7.4)
