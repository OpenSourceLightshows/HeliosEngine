#include <math.h>

#include "Led.h"

#include "TimeControl.h"

#include "HeliosConfig.h"

#ifdef HELIOS_EMBEDDED
#ifdef HELIOS_ARDUINO
#include <arduino.h>
#elif defined(HELIOS_STM8)
// STM8 hardware registers for PWM
// TIM2 drives RED (CH2/PD3) + BLUE (CH3/PA3); TIM1 drives GREEN (CH3/PC3)
#else
#include <avr/sleep.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#endif
#ifndef HELIOS_STM8
#define PWM_PIN_R PB0 // Red channel (pin 5)
#define PWM_PIN_G PB1 // Green channel (pin 6)
#define PWM_PIN_B PB4 // Blue channel (pin 3)
#endif
#endif

#define SCALE8(i, scale)  (((uint16_t)i * (uint16_t)(scale)) >> 8)

// Forward declaration
static void led_set_pwm(uint8_t pwmPin, uint8_t pwmValue, volatile uint8_t *controlRegister,
    uint8_t controlBit, volatile uint8_t *compareRegister);

// array of led color values
static rgb_color_t m_ledColor;
static rgb_color_t m_realColor;
// global brightness
static uint8_t m_brightness = DEFAULT_BRIGHTNESS;

uint8_t led_init(void)
{
  // clear the led colors
  rgb_init_from_raw(&m_ledColor, RGB_OFF);
  rgb_init_from_raw(&m_realColor, RGB_OFF);
#ifdef HELIOS_EMBEDDED
#ifdef HELIOS_ARDUINO
  pinMode(0, OUTPUT);
  pinMode(1, OUTPUT);
  pinMode(4, OUTPUT);
#elif defined(HELIOS_STM8)
  // GPIO and Timer init done in stm8_init_gpio and stm8_init_timers
#else
  // AVR pin ctrl done in helios_init
#endif
#endif
  return 1;
}

void led_cleanup(void)
{
}

void led_set_rgb(const rgb_color_t *col)
{
  rgb_copy(&m_ledColor, col);
  m_realColor.red = SCALE8(m_ledColor.red, m_brightness);
  m_realColor.green = SCALE8(m_ledColor.green, m_brightness);
  m_realColor.blue = SCALE8(m_ledColor.blue, m_brightness);
}

#ifndef HELIOS_STM8
void led_set_rgb3(uint8_t r, uint8_t g, uint8_t b)
{
  rgb_color_t col;
  rgb_init3(&col, r, g, b);
  led_set_rgb(&col);
}
#endif

void led_clear(void)
{
  rgb_color_t off;
  rgb_init_from_raw(&off, RGB_OFF);
  led_set_rgb(&off);
}

#ifndef HELIOS_STM8
void led_adjust_brightness(uint8_t fadeBy)
{
  rgb_adjust_brightness(&m_ledColor, fadeBy);
}
#endif

void led_strobe(uint16_t on_time, uint16_t off_time, const rgb_color_t *off_col, const rgb_color_t *on_col)
{
  if ((time_get_current_time() % (on_time + off_time)) > on_time) {
    led_set_rgb(off_col);
  } else {
    led_set_rgb(on_col);
  }
}

#ifndef HELIOS_STM8
void led_breath(uint8_t hue, uint32_t duration, uint8_t magnitude, uint8_t sat, uint8_t val)
{
  if (!duration) {
    // don't divide by 0
    return;
  }
  // Determine the phase in the cycle
  uint32_t phase = time_get_current_time() % (2 * duration);
  // Calculate hue shift
  int32_t hueShift;
  if (phase < duration) {
    // Ascending phase - from hue to hue + magnitude
    hueShift = (phase * magnitude) / duration;
  } else {
    // Descending phase - from hue + magnitude to hue
    hueShift = ((2 * duration - phase) * magnitude) / duration;
  }
  // Apply hue shift - ensure hue stays within valid range
  uint8_t shiftedHue = hue + hueShift;
  // Apply the hsv color as a strobing hue shift
  hsv_color_t hsv;
  rgb_color_t off, on;
  hsv_init3(&hsv, shiftedHue, sat, val);
  rgb_init_from_raw(&off, RGB_OFF);
  rgb_init_from_hsv(&on, &hsv);
  led_strobe(2, 13, &off, &on);
}
#endif

#ifndef HELIOS_STM8
void led_hold(const rgb_color_t *col)
{
  led_set_rgb(col);
  led_update();
  time_delay_milliseconds(250);
}
#endif

#ifndef HELIOS_STM8
static void led_set_pwm(uint8_t pwmPin, uint8_t pwmValue, volatile uint8_t *controlRegister,
    uint8_t controlBit, volatile uint8_t *compareRegister)
{
#ifdef HELIOS_EMBEDDED
  if (pwmValue == 0) {
    // digitalWrite(pin, LOW)
    *controlRegister &= ~controlBit;  // Disable PWM
    PORTB &= ~(1 << pwmPin);  // Set the pin low
  } else if (pwmValue == 255) {
    // digitalWrite(pin, HIGH)
    *controlRegister &= ~controlBit;  // Disable PWM
    PORTB |= (1 << pwmPin);  // Set the pin high
  } else {
    // analogWrite(pin, value)
    *controlRegister |= controlBit;  // Enable PWM
    *compareRegister = pwmValue;  // Set PWM duty cycle
  }
#else
  (void)pwmPin;
  (void)pwmValue;
  (void)controlRegister;
  (void)controlBit;
  (void)compareRegister;
#endif
}
#endif

rgb_color_t led_get(void)
{
  return m_ledColor;
}

uint8_t led_get_brightness(void)
{
  return m_brightness;
}

void led_set_brightness(uint8_t brightness)
{
  m_brightness = brightness;
}

void led_update(void)
{
#ifdef HELIOS_EMBEDDED
  // write out the rgb values to analog pins
#ifdef HELIOS_ARDUINO
  analogWrite(PWM_PIN_R, m_realColor.red);
  analogWrite(PWM_PIN_G, m_realColor.green);
  analogWrite(PWM_PIN_B, m_realColor.blue);
#elif defined(HELIOS_STM8)
  // STM8 - write 8-bit PWM duty to the LOW byte of each compare register
  // (ARRH=0/ARRL=255, so the high compare bytes stay 0)
  // RED   = PD3 / TIM2_CH2
  *(volatile uint8_t *)0x5314 = m_realColor.red;   // TIM2_CCR2L
  // GREEN = PC3 / TIM1_CH3
  *(volatile uint8_t *)0x526A = m_realColor.green; // TIM1_CCR3L
  // BLUE  = PA3 / TIM2_CH3
  *(volatile uint8_t *)0x5316 = m_realColor.blue;  // TIM2_CCR3L
#else
  // AVR - backup SREG and turn off interrupts
  uint8_t oldSREG = SREG;
  cli();

  // set the PWM for R/G/B output
  led_set_pwm(PWM_PIN_R, m_realColor.red, &TCCR0A, (1 << COM0A1), &OCR0A);
  led_set_pwm(PWM_PIN_G, m_realColor.green, &TCCR0A, (1 << COM0B1), &OCR0B);
  led_set_pwm(PWM_PIN_B, m_realColor.blue, &GTCCR, (1 << COM1B1), &OCR1B);

  // turn interrupts back on
  SREG = oldSREG;
#endif
#endif
}

