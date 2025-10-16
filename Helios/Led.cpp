#include <math.h>

#include "Led.h"

#include "TimeControl.h"

#include "HeliosConfig.h"
#include "Helios.h"

#ifdef HELIOS_EMBEDDED
  #ifdef HELIOS_8051
    #include "CA51Hardware.h"
  #elif defined(HELIOS_ARDUINO)
    #include <arduino.h>
  #elif defined(HELIOS_AVR)
    #include <avr/sleep.h>
    #include <avr/io.h>
    #include <avr/interrupt.h>
    #define PWM_PIN_R PB0 // Red channel (pin 5)
    #define PWM_PIN_G PB1 // Green channel (pin 6)
    #define PWM_PIN_B PB4 // Blue channel (pin 3)
  #endif
#endif

#define SCALE8(i, scale)  (((uint16_t)i * (uint16_t)(scale)) >> 8)

// array of led color values
RGBColor Led::m_ledColor = RGB_OFF;
RGBColor Led::m_realColor = RGB_OFF;
// global brightness
uint8_t Led::m_brightness = DEFAULT_BRIGHTNESS;

bool Led::init()
{
  // clear the led colors
  m_ledColor = RGB_OFF;
  m_realColor = RGB_OFF;
#ifdef HELIOS_EMBEDDED
  #ifdef HELIOS_8051
    // CA51F152XX: Configure PWM pins as push-pull output
    SET_PIN_OUTPUT(1, PWM_PIN_R);
    SET_PIN_OUTPUT(1, PWM_PIN_G);
    SET_PIN_OUTPUT(1, PWM_PIN_B);

    // Initialize PWM module
    // PWMCFG: Configure PWM mode and frequency
    // Assuming 8-bit PWM with appropriate prescaler
    PWMCFG = 0x00;  // Reset PWM configuration

    // PWMCR: Enable PWM channels 0, 1, 2
    PWMCR = 0x07;   // Enable PWM0, PWM1, PWM2

    // Set initial duty cycles to 0
    PWM0 = 0;
    PWM1 = 0;
    PWM2 = 0;
  #elif defined(HELIOS_ARDUINO)
    pinMode(0, OUTPUT);
    pinMode(1, OUTPUT);
    pinMode(4, OUTPUT);
  #elif defined(HELIOS_AVR)
    // pin ctrl done in Helios::init for AVR
  #endif
#endif
  return true;
}

void Led::cleanup()
{
}

void Led::set(RGBColor col)
{
  m_ledColor = col;
  m_realColor.red = SCALE8(m_ledColor.red, m_brightness);
  m_realColor.green = SCALE8(m_ledColor.green, m_brightness);
  m_realColor.blue = SCALE8(m_ledColor.blue, m_brightness);
}

void Led::set(uint8_t r, uint8_t g, uint8_t b)
{
  set(RGBColor(r, g, b));
}

void Led::adjustBrightness(uint8_t fadeBy)
{
  m_ledColor.adjustBrightness(fadeBy);
}

void Led::strobe(uint16_t on_time, uint16_t off_time, RGBColor off_col, RGBColor on_col)
{
  set(((Time::getCurtime() % (on_time + off_time)) > on_time) ? off_col : on_col);
}

void Led::breath(uint8_t hue, uint32_t duration, uint8_t magnitude, uint8_t sat, uint8_t val)
{
  if (!duration) {
    // don't divide by 0
    return;
  }
  // Determine the phase in the cycle
  uint32_t phase = Time::getCurtime() % (2 * duration);
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
  strobe(2, 13, RGB_OFF, HSVColor(shiftedHue, sat, val));
}

void Led::hold(RGBColor col)
{
  set(col);
  update();
  Time::delayMilliseconds(250);
}

void Led::setPWM(uint8_t pwmPin, uint8_t pwmValue, volatile uint8_t &controlRegister,
    uint8_t controlBit, volatile uint8_t &compareRegister)
{
#ifdef HELIOS_EMBEDDED
  #ifdef HELIOS_8051
    // CA51F152XX: Direct PWM control
    // pwmPin is 0, 1, or 2 for PWM channels
    // We can directly set the duty cycle register
    (void)controlRegister;  // Unused for 8051
    (void)controlBit;       // Unused for 8051
    (void)compareRegister;  // Unused for 8051

    if (pwmValue == 0) {
      // Turn off PWM and set pin low
      SET_PIN_LOW(1, pwmPin);
    } else if (pwmValue == 255) {
      // Turn off PWM and set pin high
      SET_PIN_HIGH(1, pwmPin);
    } else {
      // Set PWM duty cycle
      switch (pwmPin) {
        case 0: PWM0 = pwmValue; break;  // Red
        case 1: PWM1 = pwmValue; break;  // Green
        case 2: PWM2 = pwmValue; break;  // Blue
      }
    }
  #elif defined(HELIOS_AVR)
    // AVR ATtiny85 PWM control
    if (pwmValue == 0) {
      // digitalWrite(pin, LOW)
      controlRegister &= ~controlBit;  // Disable PWM
      PORTB &= ~(1 << pwmPin);  // Set the pin low
    } else if (pwmValue == 255) {
      // digitalWrite(pin, HIGH)
      controlRegister &= ~controlBit;  // Disable PWM
      PORTB |= (1 << pwmPin);  // Set the pin high
    } else {
      // analogWrite(pin, value)
      controlRegister |= controlBit;  // Enable PWM
      compareRegister = pwmValue;  // Set PWM duty cycle
    }
  #endif
#endif
}

void Led::update()
{
#ifdef HELIOS_EMBEDDED
  // write out the rgb values to analog pins
  #ifdef HELIOS_8051
    // CA51F152XX: Direct PWM updates
    uint8_t oldIE;
    SAVE_INTERRUPTS(oldIE);
    DISABLE_INTERRUPTS();

    // Directly set PWM duty cycles
    PWM0 = m_realColor.red;    // Red channel
    PWM1 = m_realColor.green;  // Green channel
    PWM2 = m_realColor.blue;   // Blue channel

    RESTORE_INTERRUPTS(oldIE);
  #elif defined(HELIOS_ARDUINO)
    analogWrite(PWM_PIN_R, m_realColor.red);
    analogWrite(PWM_PIN_G, m_realColor.green);
    analogWrite(PWM_PIN_B, m_realColor.blue);
  #elif defined(HELIOS_AVR)
    // backup SREG and turn off interrupts
    uint8_t oldSREG = SREG;
    cli();

    // set the PWM for R/G/B output
    setPWM(PWM_PIN_R, m_realColor.red, TCCR0A, (1 << COM0A1), OCR0A);
    setPWM(PWM_PIN_G, m_realColor.green, TCCR0A, (1 << COM0B1), OCR0B);
    setPWM(PWM_PIN_B, m_realColor.blue, GTCCR, (1 << COM1B1), OCR1B);

    // turn interrupts back on
    SREG = oldSREG;
  #endif
#endif
}
