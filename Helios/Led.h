#ifndef LED_CONTROL_H
#define LED_CONTROL_H

#include <inttypes.h>

#include "Colortypes.h"

class Led
{
  // private unimplemented constructor
  Led();


  // opting for static class here because there should only ever be one
  // Led control object and I don't like singletons
  static bool init();
  static void cleanup();

  // control individual LED, these are appropriate to use in internal pattern logic
  static void set(RGBColor col);
  static void set(uint8_t r, uint8_t g, uint8_t b);

  // Turn off individual LEDs, these are appropriate to use in internal pattern logic
  static void clear() { set(RGB_OFF); }

  // Dim individual LEDs, these are appropriate to use in internal pattern logic
  static void adjustBrightness(uint8_t fadeBy);

  // strobe between two colors with a simple on/off timing
  static void strobe(uint16_t on_time, uint16_t off_time, RGBColor col1, RGBColor col2);

  // breath the hue on an index
  // warning: these use hsv to rgb in realtime!
  static void breath(uint8_t hue, uint32_t duration = 1000, uint8_t magnitude = 60,
      uint8_t sat = 255, uint8_t val = 255);

  // a very specialized api to hold all leds on a color for 250ms
  static void hold(RGBColor col);

  // get the RGBColor of an Led index
  static RGBColor get() { return m_ledColor; }

  // global brightness
  static uint8_t getBrightness() { return m_brightness; }
  static void setBrightness(uint8_t brightness) { m_brightness = brightness; }

  // actually update the LEDs and show the changes
  static void update();


  static void setPWM(uint8_t pwmPin, uint8_t pwmValue, volatile uint8_t &controlRegister,
      uint8_t controlBit, volatile uint8_t &compareRegister);

  // the global brightness
  static uint8_t m_brightness;
  // led color
  static RGBColor m_ledColor;
  static RGBColor m_realColor;
};

#endif
