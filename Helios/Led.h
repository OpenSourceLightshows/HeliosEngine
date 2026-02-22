#ifndef LED_CONTROL_H
#define LED_CONTROL_H

#include <inttypes.h>

#include "Colortypes.h"

class Time;
class Helios;

class Led
{
public:
  explicit Led(Helios &helios);
  bool init();
  void cleanup();

  // control individual LED, these are appropriate to use in internal pattern logic
  void set(RGBColor col);
  void set(uint8_t r, uint8_t g, uint8_t b);

  // Turn off individual LEDs, these are appropriate to use in internal pattern logic
  void clear() { set(RGB_OFF); }

  // Dim individual LEDs, these are appropriate to use in internal pattern logic
  void adjustBrightness(uint8_t fadeBy);

  // strobe between two colors with a simple on/off timing
  void strobe(uint16_t on_time, uint16_t off_time, RGBColor col1, RGBColor col2);

  // breath the hue on an index
  // warning: these use hsv to rgb in realtime!
  void breath(uint8_t hue, uint32_t duration = 1000, uint8_t magnitude = 60,
      uint8_t sat = 255, uint8_t val = 255);

  // a very specialized api to hold all leds on a color for 250ms
  void hold(RGBColor col);

  // get the RGBColor of an Led index
  RGBColor get() const { return m_ledColor; }

  // global brightness
  uint8_t getBrightness() const { return m_brightness; }
  void setBrightness(uint8_t brightness);

  // actually update the LEDs and show the changes
  void update();

private:
  void setPWM(uint8_t pwmPin, uint8_t pwmValue, volatile uint8_t &controlRegister,
      uint8_t controlBit, volatile uint8_t &compareRegister);

  // the global brightness
  uint8_t m_brightness;
  // led color
  RGBColor m_ledColor;
  RGBColor m_realColor;
  Helios &m_helios;
};

#endif
