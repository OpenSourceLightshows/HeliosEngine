#ifndef CA51_HARDWARE_H
#define CA51_HARDWARE_H

// ============================================================================
// CA51F152XX Hardware Configuration for Helios Engine
// ============================================================================
// This file provides hardware abstraction for the CA51F152XX 8051 architecture
// replacing the ATTiny85 AVR architecture

#ifdef HELIOS_8051

#include <stdint.h>

// CA51F152XX has more flash but less RAM than ATTiny85
// Flash: 16KB (vs 8KB on ATTiny85)
// RAM: 256 bytes (vs 512 bytes on ATTiny85)
// We need to be very careful with RAM usage and move constants to code space

// ============================================================================
// Pin Definitions
// ============================================================================

// RGB LED Pins (using PWM-capable pins)
#define PWM_PIN_R   0  // P1.0 - Red channel (PWM0)
#define PWM_PIN_G   1  // P1.1 - Green channel (PWM1)
#define PWM_PIN_B   2  // P1.2 - Blue channel (PWM2)

// Button Pin
#define BUTTON_PIN  3  // P1.3 - Button input
#define BUTTON_PORT 1  // Port 1

// ============================================================================
// Register Definitions for CA51F152XX
// ============================================================================

// GPIO Ports
#ifndef P0
#define P0    (*(volatile uint8_t *)0x80)  // Port 0
#endif
#ifndef P1
#define P1    (*(volatile uint8_t *)0x90)  // Port 1
#endif
#ifndef P2
#define P2    (*(volatile uint8_t *)0xA0)  // Port 2
#endif
#ifndef P3
#define P3    (*(volatile uint8_t *)0xB0)  // Port 3
#endif

// Port Configuration Registers
#define P0M0  (*(volatile uint8_t *)0x94)  // Port 0 Mode 0
#define P0M1  (*(volatile uint8_t *)0x93)  // Port 0 Mode 1
#define P1M0  (*(volatile uint8_t *)0x92)  // Port 1 Mode 0
#define P1M1  (*(volatile uint8_t *)0x91)  // Port 1 Mode 1
#define P2M0  (*(volatile uint8_t *)0x96)  // Port 2 Mode 0
#define P2M1  (*(volatile uint8_t *)0x95)  // Port 2 Mode 1
#define P3M0  (*(volatile uint8_t *)0xB2)  // Port 3 Mode 0
#define P3M1  (*(volatile uint8_t *)0xB1)  // Port 3 Mode 1

// PWM Control Registers
#define PWMCFG   (*(volatile uint8_t *)0xF1)  // PWM Configuration
#define PWMCR    (*(volatile uint8_t *)0xF2)  // PWM Control
#define PWM0     (*(volatile uint8_t *)0xF3)  // PWM0 Duty Cycle
#define PWM1     (*(volatile uint8_t *)0xF4)  // PWM1 Duty Cycle
#define PWM2     (*(volatile uint8_t *)0xF5)  // PWM2 Duty Cycle

// Timer Registers
#define TCON     (*(volatile uint8_t *)0x88)  // Timer Control
#define TMOD     (*(volatile uint8_t *)0x89)  // Timer Mode
#define TL0      (*(volatile uint8_t *)0x8A)  // Timer 0 Low
#define TL1      (*(volatile uint8_t *)0x8B)  // Timer 1 Low
#define TH0      (*(volatile uint8_t *)0x8C)  // Timer 0 High
#define TH1      (*(volatile uint8_t *)0x8D)  // Timer 1 High

// Interrupt Registers
#define IE       (*(volatile uint8_t *)0xA8)  // Interrupt Enable
#define IP       (*(volatile uint8_t *)0xB8)  // Interrupt Priority
#define IPH      (*(volatile uint8_t *)0xB7)  // Interrupt Priority High

// Power Control
#define PCON     (*(volatile uint8_t *)0x87)  // Power Control

// Interrupt Enable Bits
#define EA       0x80  // Enable All Interrupts
#define ET0      0x02  // Timer 0 Interrupt Enable
#define ET1      0x08  // Timer 1 Interrupt Enable
#define EX0      0x01  // External Interrupt 0 Enable
#define EX1      0x04  // External Interrupt 1 Enable

// TCON Bits
#define TR0      0x10  // Timer 0 Run
#define TR1      0x40  // Timer 1 Run
#define TF0      0x20  // Timer 0 Overflow Flag
#define TF1      0x80  // Timer 1 Overflow Flag
#define IT0      0x01  // External Interrupt 0 Type
#define IE0      0x02  // External Interrupt 0 Flag
#define IT1      0x04  // External Interrupt 1 Type
#define IE1      0x08  // External Interrupt 1 Flag

// PCON Bits
#define IDL      0x01  // Idle Mode
#define PD       0x02  // Power Down Mode

// ============================================================================
// Flash/EEPROM Configuration
// ============================================================================

// CA51F152XX uses flash memory for data storage instead of EEPROM
// We'll use the upper flash area for non-volatile storage
#define FLASH_DATA_START  0x3F00  // Start of data area in flash
#define FLASH_PAGE_SIZE   128      // Flash page size in bytes

// Flash Control Registers
#define FLASHCR   (*(volatile uint8_t *)0xE7)  // Flash Control
#define FLASHKEY  (*(volatile uint8_t *)0xE6)  // Flash Key Register

// ============================================================================
// Macros for Interrupt Control
// ============================================================================

// Enable/Disable interrupts (equivalent to AVR sei/cli)
#define ENABLE_INTERRUPTS()   EA = 1
#define DISABLE_INTERRUPTS()  EA = 0

// Store and restore interrupt state
#define SAVE_INTERRUPTS(x)    (x) = IE
#define RESTORE_INTERRUPTS(x) IE = (x)

// ============================================================================
// Port Configuration Macros
// ============================================================================

// Port modes:
// M1 M0 | Mode
// 0  0  | Quasi-bidirectional (default)
// 0  1  | Push-pull output
// 1  0  | Input only (high-impedance)
// 1  1  | Open-drain output

#define SET_PIN_OUTPUT(port, pin) \
  do { \
    P##port##M0 |= (1 << (pin)); \
    P##port##M1 &= ~(1 << (pin)); \
  } while(0)

#define SET_PIN_INPUT(port, pin) \
  do { \
    P##port##M0 &= ~(1 << (pin)); \
    P##port##M1 |= (1 << (pin)); \
  } while(0)

#define SET_PIN_HIGH(port, pin) \
  P##port |= (1 << (pin))

#define SET_PIN_LOW(port, pin) \
  P##port &= ~(1 << (pin))

#define READ_PIN(port, pin) \
  ((P##port & (1 << (pin))) != 0)

// ============================================================================
// Global Timer Variables (for microsecond tracking)
// ============================================================================

extern volatile uint32_t timer0_overflow_count;

// ============================================================================
// Hardware-Specific Attributes for RAM Optimization
// ============================================================================

// Use these attributes to move constant data to code space (flash)
// This is critical since we only have 256 bytes of RAM

#ifdef __SDCC
  // SDCC compiler attributes
  #define CODE_ATTR __code        // Place in code (flash) memory
  #define XDATA_ATTR __xdata      // Place in external RAM (if available)
  #define IDATA_ATTR __idata      // Place in internal RAM
  #define REENTRANT __reentrant   // Reentrant function

  // Interrupt function attribute
  #define ISR_ATTR(vector) __interrupt vector __using 1
#else
  // For non-SDCC compilers (like CLI builds)
  #define CODE_ATTR
  #define XDATA_ATTR
  #define IDATA_ATTR
  #define REENTRANT
  #define ISR_ATTR(vector)
#endif

#endif // HELIOS_8051

#endif // CA51_HARDWARE_H

