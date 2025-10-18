// CA51F152XX Register Definitions
// ShenZhen JinRui Technology Co., Ltd.
// For use with SDCC compiler

#ifndef CA51F152_H
#define CA51F152_H

#include <8051.h>

// GPIO Mode Control Registers
// Port mode: P1M0=0, P1M1=0: Bidirectional
//           P1M0=0, P1M1=1: Push-pull output
//           P1M0=1, P1M1=0: Input only (high impedance)
//           P1M1=1, P1M1=1: Open drain

__sfr __at (0x91) P1M0;     // Port 1 mode control register 0
__sfr __at (0x92) P1M1;     // Port 1 mode control register 1
__sfr __at (0xB1) P3M0;     // Port 3 mode control register 0
__sfr __at (0xB2) P3M1;     // Port 3 mode control register 1

// Flash Control Registers (Extended SFRs in xdata space)
#define MECON  (*(__xdata unsigned char *)(0xFC00))  // Memory control
#define FSCMD  (*(__xdata unsigned char *)(0xFC01))  // Flash command register
#define FSDAT  (*(__xdata unsigned char *)(0xFC02))  // Flash data register
#define LOCK   (*(__xdata unsigned char *)(0xFC03))  // Flash lock register
#define PADRD  (*(__xdata unsigned char *)(0xFC04))  // Program area division register
#define PTSL   (*(__xdata unsigned char *)(0xFC05))  // Target address pointer low
#define PTSH   (*(__xdata unsigned char *)(0xFC06))  // Target address pointer high

// Flash command definitions
#define CMD_RESET           0       // Reset command
#define CMD_DATA_READ       1       // Read data area
#define CMD_DATA_WRITE      2       // Write data area
#define CMD_DATA_ERASE      3       // Erase data area sector
#define CMD_PROGRAM_READ    5       // Read program area
#define CMD_PROGRAM_WRITE   6       // Write program area
#define CMD_PROGRAM_ERASE   7       // Erase program area sector
#define CMD_SET_LATCH       8       // Set erase/write latch

// Flash lock/unlock definitions
#define CMD_DATA_AREA_UNLOCK    0x2A    // Unlock data area
#define CMD_PROGRAM_AREA_UNLOCK 0x29    // Unlock program area
#define CMD_FLASH_LOCK          0xAA    // Lock flash

// Timer Control (extended beyond standard 8051)
// Standard 8051 has TMOD, TCON, TH0, TL0, TH1, TL1
// CA51F152 may have additional timer registers

// PWM Control Registers (if available on CA51F152)
// Note: These need to be verified from the CA51F152 datasheet
// __sfr __at (0xXX) PWM0;     // PWM channel 0 control
// __sfr __at (0xXX) PWM1;     // PWM channel 1 control
// __sfr __at (0xXX) PWM2;     // PWM channel 2 control

// ADC Control Registers (12-bit ADC)
// Note: These addresses need to be verified from the CA51F152 datasheet
// __sfr __at (0xXX) ADCCON;   // ADC control register
// __sfr __at (0xXX) ADCDATAH; // ADC data high byte
// __sfr __at (0xXX) ADCDATAL; // ADC data low byte

// Interrupt Priority (extended)
// Standard 8051: IP (0xB8)
// Extended interrupts may have IP2, IP3, etc.

// Touch Key Registers (if applicable)
// __sfr __at (0xXX) TKCON;    // Touch key control

// Low Voltage Detection
// __sfr __at (0xXX) LVDCON;   // LVD control register

#endif // CA51F152_H
