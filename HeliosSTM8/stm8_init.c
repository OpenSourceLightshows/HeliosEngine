#include "stm8_init.h"
#include <stdint.h>

#ifdef HELIOS_STM8
// STM8S register definitions
#define CLK_CKDIVR  (*(volatile uint8_t *)0x50C6)
#define CLK_PCKENR1 (*(volatile uint8_t *)0x50C7)
#define CLK_PCKENR2 (*(volatile uint8_t *)0x50CA)

// GPIO Port D registers
#define PD_ODR      (*(volatile uint8_t *)0x500F)
#define PD_IDR      (*(volatile uint8_t *)0x5010)
#define PD_DDR      (*(volatile uint8_t *)0x5011)
#define PD_CR1      (*(volatile uint8_t *)0x5012)
#define PD_CR2      (*(volatile uint8_t *)0x5013)

// GPIO Port C registers
#define PC_ODR      (*(volatile uint8_t *)0x500A)
#define PC_IDR      (*(volatile uint8_t *)0x500B)
#define PC_DDR      (*(volatile uint8_t *)0x500C)
#define PC_CR1      (*(volatile uint8_t *)0x500D)
#define PC_CR2      (*(volatile uint8_t *)0x500E)

// GPIO Port B registers
#define PB_ODR      (*(volatile uint8_t *)0x5005)
#define PB_IDR      (*(volatile uint8_t *)0x5006)
#define PB_DDR      (*(volatile uint8_t *)0x5007)
#define PB_CR1      (*(volatile uint8_t *)0x5008)
#define PB_CR2      (*(volatile uint8_t *)0x5009)

// Timer 1 registers (for PWM)
#define TIM1_CR1    (*(volatile uint8_t *)0x5250)
#define TIM1_PSCRH  (*(volatile uint8_t *)0x5260)
#define TIM1_PSCRL  (*(volatile uint8_t *)0x5261)
#define TIM1_ARRH   (*(volatile uint8_t *)0x5262)
#define TIM1_ARRL   (*(volatile uint8_t *)0x5263)
#define TIM1_CCR1H  (*(volatile uint8_t *)0x5265)
#define TIM1_CCR1L  (*(volatile uint8_t *)0x5266)
#define TIM1_CCR2H  (*(volatile uint8_t *)0x5267)
#define TIM1_CCR2L  (*(volatile uint8_t *)0x5268)
#define TIM1_CCER1  (*(volatile uint8_t *)0x525C)
#define TIM1_CCMR1  (*(volatile uint8_t *)0x5258)
#define TIM1_CCMR2  (*(volatile uint8_t *)0x5259)
#define TIM1_BKR    (*(volatile uint8_t *)0x526D)

// Timer 2 registers (for PWM on additional channels)
#define TIM2_CR1    (*(volatile uint8_t *)0x5300)
#define TIM2_PSCR   (*(volatile uint8_t *)0x530E)
#define TIM2_ARRH   (*(volatile uint8_t *)0x530F)
#define TIM2_ARRL   (*(volatile uint8_t *)0x5310)
#define TIM2_CCR1H  (*(volatile uint8_t *)0x5311)
#define TIM2_CCR1L  (*(volatile uint8_t *)0x5312)
#define TIM2_CCR2H  (*(volatile uint8_t *)0x5313)
#define TIM2_CCR2L  (*(volatile uint8_t *)0x5314)
#define TIM2_CCER1  (*(volatile uint8_t *)0x5308)
#define TIM2_CCMR1  (*(volatile uint8_t *)0x5307)
#define TIM2_CCMR2  (*(volatile uint8_t *)0x5309)

// External interrupt registers
#define EXTI_CR1    (*(volatile uint8_t *)0x50A0)
#define EXTI_CR2    (*(volatile uint8_t *)0x50A1)

void stm8_init_clock(void)
{
  // Set clock divider to /1 (16MHz from internal RC oscillator)
  CLK_CKDIVR = 0x00;

  // Enable peripheral clocks
  CLK_PCKENR1 = 0xFF; // Enable all peripherals in group 1
  CLK_PCKENR2 = 0xFF; // Enable all peripherals in group 2
}

void stm8_init_gpio(void)
{
  // Configure LED pins as outputs (Push-Pull)
  // Based on schematic: RED=PC5, GREEN=PC4, BLUE=PB4

  // LED R - PC5 (TIM2_CH1)
  PC_DDR |= (1 << 5);  // Set as output
  PC_CR1 |= (1 << 5);  // Push-pull mode
  PC_CR2 |= (1 << 5);  // Fast mode (10MHz)

  // LED G - PC4 (TIM2_CH2)
  PC_DDR |= (1 << 4);  // Set as output
  PC_CR1 |= (1 << 4);  // Push-pull mode
  PC_CR2 |= (1 << 4);  // Fast mode

  // LED B - PB4 (TIM1_CH1N)
  PB_DDR |= (1 << 4);  // Set as output
  PB_CR1 |= (1 << 4);  // Push-pull mode
  PB_CR2 |= (1 << 4);  // Fast mode

  // Configure button pin as input with pull-down
  // Button - PB5 (active HIGH with pull-down resistor on schematic)
  PB_DDR &= ~(1 << 5); // Set as input
  PB_CR1 &= ~(1 << 5); // Disable pull-up (external pull-down on schematic)
  PB_CR2 |= (1 << 5);  // Enable interrupt

  // Initialize all LED pins to OFF
  PC_ODR &= ~((1 << 5) | (1 << 4));
  PB_ODR &= ~(1 << 4);
}

void stm8_init_timers(void)
{
  // Configure Timer 2 for PWM on Red and Green LEDs
  // Based on schematic: RED=PC5 (TIM2_CH1), GREEN=PC4 (TIM2_CH2)

  // Prescaler: 16MHz / 16 = 1MHz
  TIM2_PSCR = 0x04; // Prescaler = 16 (2^4)

  // Auto-reload: 1MHz / 256 = ~3.9kHz PWM frequency
  TIM2_ARRH = 0;
  TIM2_ARRL = 255;

  // Configure PWM Mode 1 for channels 1 and 2
  TIM2_CCMR1 = 0x60; // PWM mode 1 on channel 1 (RED - PC5)
  TIM2_CCMR2 = 0x60; // PWM mode 1 on channel 2 (GREEN - PC4)

  // Enable output on channels 1 and 2
  TIM2_CCER1 = 0x11; // CC1E and CC2E

  // Start timer
  TIM2_CR1 = 0x01; // CEN bit

  // Configure Timer 1 for PWM on Blue LED (PB4)
  // Using complementary output TIM1_CH1N on PB4

  // Prescaler: 16MHz / 16 = 1MHz
  TIM1_PSCRH = 0;
  TIM1_PSCRL = 15; // Prescaler = 16

  // Auto-reload register: 1MHz / 256 = ~3.9kHz PWM frequency
  TIM1_ARRH = 0;
  TIM1_ARRL = 255;

  // Configure PWM Mode 1 for channel 1
  TIM1_CCMR1 = 0x60; // PWM mode 1 on channel 1

  // Enable complementary output on channel 1N (PB4)
  TIM1_CCER1 = 0x04; // CC1NE (complementary output enable)

  // Main output enable
  TIM1_BKR = 0x80; // MOE bit

  // Start timer
  TIM1_CR1 = 0x01; // CEN bit
}

void stm8_init_interrupts(void)
{
  // Configure external interrupt for button (PB5)
  // EXTI_CR1 controls ports A and B
  // PB5 is controlled by bits [3:2] of EXTI_CR1
  // 00 = Interrupt on falling edge and low level
  // 01 = Interrupt on rising edge only
  // 10 = Interrupt on falling edge only
  // 11 = Interrupt on rising and falling edges
  EXTI_CR1 = (EXTI_CR1 & 0xF3) | (0x03 << 2); // Rising and falling edges

  // Enable global interrupts
  __asm__("rim"); // Enable interrupts (STM8 instruction)
}

#endif // HELIOS_STM8
