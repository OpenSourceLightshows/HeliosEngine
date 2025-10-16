#include <math.h>

#include "Led.h"

#include "TimeControl.h"

#include "HeliosConfig.h"

#ifdef HELIOS_EMBEDDED
#ifdef HELIOS_ARDUINO
#include <arduino.h>
#else
#include <avr/sleep.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#endif
#define PWM_PIN_R PB0 /* Red channel (pin 5) */
#define PWM_PIN_G PB1 /* Green channel (pin 6) */
#define PWM_PIN_B PB4 /* Blue channel (pin 3) */
#endif

#define SCALE8(i, scale)  (((uint16_t)i * (uint16_t)(scale)) >> 8)

/* Forward declaration */
static void led_set_pwm(uint8_t pwmPin, uint8_t pwmValue, volatile uint8_t *controlRegister,
    uint8_t controlBit, volatile uint8_t *compareRegister);

/* array of led color values */
static rgb_color_t m_ledColor;
static rgb_color_t m_realColor;
/* global brightness */
static uint8_t m_brightness = DEFAULT_BRIGHTNESS;

uint8_t led_init(void)
{
  /* clear the led colors */
  rgb_init_from_raw(&m_ledColor, RGB_OFF);
  rgb_init_from_raw(&m_realColor, RGB_OFF);
#ifdef HELIOS_EMBEDDED
#ifdef HELIOS_ARDUINO
  pinMode(0, OUTPUT);
  pinMode(1, OUTPUT);
  pinMode(4, OUTPUT);
#else
  /* pin ctrl done in helios_init */
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

void led_set_rgb3(uint8_t r, uint8_t g, uint8_t b)
{
  rgb_color_t col;
  rgb_init3(&col, r, g, b);
  led_set_rgb(&col);
}

void led_clear(void)
{
  rgb_color_t off;
  rgb_init_from_raw(&off, RGB_OFF);
  led_set_rgb(&off);
}

void led_adjust_brightness(uint8_t fadeBy)
{
  rgb_adjust_brightness(&m_ledColor, fadeBy);
}

void led_strobe(uint16_t on_time, uint16_t off_time, const rgb_color_t *off_col, const rgb_color_t *on_col)
{
  if ((Time_getCurtime() % (on_time + off_time)) > on_time) {
    led_set_rgb(off_col);
  } else {
    led_set_rgb(on_col);
  }
}

void led_breath(uint8_t hue, uint32_t duration, uint8_t magnitude, uint8_t sat, uint8_t val)
{
  if (!duration) {
    /* don't divide by 0 */
    return;
  }
  /* Determine the phase in the cycle */
  uint32_t phase = Time_getCurtime() % (2 * duration);
  /* Calculate hue shift */
  int32_t hueShift;
  if (phase < duration) {
    /* Ascending phase - from hue to hue + magnitude */
    hueShift = (phase * magnitude) / duration;
  } else {
    /* Descending phase - from hue + magnitude to hue */
    hueShift = ((2 * duration - phase) * magnitude) / duration;
  }
  /* Apply hue shift - ensure hue stays within valid range */
  uint8_t shiftedHue = hue + hueShift;
  /* Apply the hsv color as a strobing hue shift */
  hsv_color_t hsv;
  rgb_color_t off, on;
  hsv_init3(&hsv, shiftedHue, sat, val);
  rgb_init_from_raw(&off, RGB_OFF);
  rgb_init_from_hsv(&on, &hsv);
  led_strobe(2, 13, &off, &on);
}

void led_hold(const rgb_color_t *col)
{
  led_set_rgb(col);
  led_update();
  Time_delayMilliseconds(250);
}

static void led_set_pwm(uint8_t pwmPin, uint8_t pwmValue, volatile uint8_t *controlRegister,
    uint8_t controlBit, volatile uint8_t *compareRegister)
{
#ifdef HELIOS_EMBEDDED
  if (pwmValue == 0) {
    /* digitalWrite(pin, LOW) */
    *controlRegister &= ~controlBit;  /* Disable PWM */
    PORTB &= ~(1 << pwmPin);  /* Set the pin low */
  } else if (pwmValue == 255) {
    /* digitalWrite(pin, HIGH) */
    *controlRegister &= ~controlBit;  /* Disable PWM */
    PORTB |= (1 << pwmPin);  /* Set the pin high */
  } else {
    /* analogWrite(pin, value) */
    *controlRegister |= controlBit;  /* Enable PWM */
    *compareRegister = pwmValue;  /* Set PWM duty cycle */
  }
#else
  (void)pwmPin;
  (void)pwmValue;
  (void)controlRegister;
  (void)controlBit;
  (void)compareRegister;
#endif
}

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
  /* write out the rgb values to analog pins */
#ifdef HELIOS_ARDUINO
  analogWrite(PWM_PIN_R, m_realColor.red);
  analogWrite(PWM_PIN_G, m_realColor.green);
  analogWrite(PWM_PIN_B, m_realColor.blue);
#else
  /* backup SREG and turn off interrupts */
  uint8_t oldSREG = SREG;
  cli();

  /* set the PWM for R/G/B output */
  led_set_pwm(PWM_PIN_R, m_realColor.red, &TCCR0A, (1 << COM0A1), &OCR0A);
  led_set_pwm(PWM_PIN_G, m_realColor.green, &TCCR0A, (1 << COM0B1), &OCR0B);
  led_set_pwm(PWM_PIN_B, m_realColor.blue, &GTCCR, (1 << COM1B1), &OCR1B);

  /* turn interrupts back on */
  SREG = oldSREG;
#endif
#endif
}

