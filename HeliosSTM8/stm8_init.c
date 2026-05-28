#include "stm8_init.h"
#include <stdint.h>

#ifdef HELIOS_STM8
// ----------------------------------------------------------------------------
// STM8S001J3 (SO-8, low-density value line) register map.
// Pin assignment (verified vs datasheet DS12129 + board netlist 2026-05-28):
//   Pin 1 PD6  -> Button (active HIGH, external pull-down R4)
//   Pin 5 PA3  -> BLUE  LED  (TIM2_CH3)
//   Pin 7 PC3  -> GREEN LED  (TIM1_CH3)
//   Pin 8 PD3  -> RED   LED  (TIM2_CH2)  [shares pin with PD1/SWIM programming]
// ----------------------------------------------------------------------------

// Clock
#define CLK_CKDIVR  (*(volatile uint8_t *)0x50C6)
#define CLK_PCKENR1 (*(volatile uint8_t *)0x50C7)
#define CLK_PCKENR2 (*(volatile uint8_t *)0x50CA)

// GPIO Port A (BLUE on PA3)
#define PA_ODR      (*(volatile uint8_t *)0x5000)
#define PA_DDR      (*(volatile uint8_t *)0x5002)
#define PA_CR1      (*(volatile uint8_t *)0x5003)
#define PA_CR2      (*(volatile uint8_t *)0x5004)

// GPIO Port C (GREEN on PC3)
#define PC_ODR      (*(volatile uint8_t *)0x500A)
#define PC_DDR      (*(volatile uint8_t *)0x500C)
#define PC_CR1      (*(volatile uint8_t *)0x500D)
#define PC_CR2      (*(volatile uint8_t *)0x500E)

// GPIO Port D (RED on PD3, Button on PD6)
#define PD_ODR      (*(volatile uint8_t *)0x500F)
#define PD_IDR      (*(volatile uint8_t *)0x5010)
#define PD_DDR      (*(volatile uint8_t *)0x5011)
#define PD_CR1      (*(volatile uint8_t *)0x5012)
#define PD_CR2      (*(volatile uint8_t *)0x5013)

// Timer 1 (advanced) - GREEN on CH3 (PC3)
#define TIM1_CR1    (*(volatile uint8_t *)0x5250)
#define TIM1_PSCRH  (*(volatile uint8_t *)0x5260)
#define TIM1_PSCRL  (*(volatile uint8_t *)0x5261)
#define TIM1_ARRH   (*(volatile uint8_t *)0x5262)
#define TIM1_ARRL   (*(volatile uint8_t *)0x5263)
#define TIM1_CCMR3  (*(volatile uint8_t *)0x525A)
#define TIM1_CCER2  (*(volatile uint8_t *)0x525D)
#define TIM1_BKR    (*(volatile uint8_t *)0x526D)

// Timer 2 (general) - RED on CH2 (PD3), BLUE on CH3 (PA3)
// Low-density TIM2 layout (verified vs STM8S003 SFR headers + RM0016):
#define TIM2_CR1    (*(volatile uint8_t *)0x5300)
#define TIM2_CCMR2  (*(volatile uint8_t *)0x5308)
#define TIM2_CCMR3  (*(volatile uint8_t *)0x5309)
#define TIM2_CCER1  (*(volatile uint8_t *)0x530A)
#define TIM2_CCER2  (*(volatile uint8_t *)0x530B)
#define TIM2_PSCR   (*(volatile uint8_t *)0x530E)
#define TIM2_ARRH   (*(volatile uint8_t *)0x530F)
#define TIM2_ARRL   (*(volatile uint8_t *)0x5310)

// External interrupt: Port D sensitivity is EXTI_CR1[7:6]
#define EXTI_CR1    (*(volatile uint8_t *)0x50A0)

void stm8_init_clock(void)
{
  CLK_CKDIVR = 0x00;   // 16MHz HSI, /1
  CLK_PCKENR1 = 0xFF;
  CLK_PCKENR2 = 0xFF;
}

// Pin 8 carries both PD3 (RED PWM) and PD1 (SWIM). Once PD3 is driven push-pull
// an ST-LINK can no longer take the pin for programming. Holding the button at
// power-on opens an approx. 5s SWIM window before the LED pins are configured;
// a normal boot skips the delay and lights up immediately.
void stm8_check_programming_mode(void)
{
  volatile uint16_t i, j;
  PD_DDR &= ~(1 << 6);   // PD6 (button) input
  PD_CR1 &= ~(1 << 6);   // floating (external pull-down R4)
  if (PD_IDR & (1 << 6)) // active HIGH: pressed at boot -> programming window
  {
    for (i = 0; i < 3000; i++)
      for (j = 0; j < 3000; j++)
        __asm__("nop");
  }
}

void stm8_init_gpio(void)
{
  // RGB LED outputs: push-pull, fast slope. Common-cathode LED, anodes driven
  // HIGH through current-limit resistors; PWM duty sets brightness.
  // RED - PD3 (TIM2_CH2)
  PD_DDR |= (1 << 3);
  PD_CR1 |= (1 << 3);
  PD_CR2 |= (1 << 3);
  // GREEN - PC3 (TIM1_CH3)
  PC_DDR |= (1 << 3);
  PC_CR1 |= (1 << 3);
  PC_CR2 |= (1 << 3);
  // BLUE - PA3 (TIM2_CH3)
  PA_DDR |= (1 << 3);
  PA_CR1 |= (1 << 3);
  PA_CR2 |= (1 << 3);

  // Button - PD6 input, floating (external pull-down R4, active HIGH)
  PD_DDR &= ~(1 << 6);
  PD_CR1 &= ~(1 << 6);
  PD_CR2 |= (1 << 6);   // enable EXTI on PD6 (wake on press)

  // LEDs off
  PD_ODR &= ~(1 << 3);
  PC_ODR &= ~(1 << 3);
  PA_ODR &= ~(1 << 3);
}

void stm8_init_timers(void)
{
  // TIM2: RED (CH2/PD3) + BLUE (CH3/PA3). 16MHz/16 = 1MHz, ARR=255 -> ~3.9kHz.
  TIM2_PSCR = 0x04;    // prescaler 2^4 = 16
  TIM2_ARRH = 0;
  TIM2_ARRL = 255;
  TIM2_CCMR2 = 0x60;   // CH2 PWM mode 1 (RED)
  TIM2_CCMR3 = 0x60;   // CH3 PWM mode 1 (BLUE)
  TIM2_CCER1 = 0x10;   // CC2E (RED output enable)
  TIM2_CCER2 = 0x01;   // CC3E (BLUE output enable)
  TIM2_CR1 = 0x01;     // CEN

  // TIM1: GREEN (CH3/PC3).
  TIM1_PSCRH = 0;
  TIM1_PSCRL = 15;     // prescaler 15+1 = 16
  TIM1_ARRH = 0;
  TIM1_ARRL = 255;
  TIM1_CCMR3 = 0x60;   // CH3 PWM mode 1 (GREEN)
  TIM1_CCER2 = 0x01;   // CC3E (GREEN output enable)
  TIM1_BKR = 0x80;     // MOE: advanced timer requires main output enable
  TIM1_CR1 = 0x01;     // CEN
}

void stm8_init_interrupts(void)
{
  // Button on Port D: sensitivity in EXTI_CR1[7:6], 11 = both edges
  EXTI_CR1 = (EXTI_CR1 & 0x3F) | (0x03 << 6);
  __asm__("rim");
}

#endif // HELIOS_STM8
