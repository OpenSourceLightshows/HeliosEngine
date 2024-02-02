#ifndef COLOR_H
#define COLOR_H

#include <inttypes.h>

#include "HeliosConfig.h"
#include "ColorConstants.h"
#ifdef HELIOS_CLI
#include <fstream>
#endif


#if ALTERNATIVE_HSV_RGB == 1
enum hsv_to_rgb_algorithm : uint8_t
{
  HSV_TO_RGB_GENERIC,
  HSV_TO_RGB_RAINBOW
};

// global hsv to rgb algorithm selector, switch this to control
// all hsv to rgb conversions
extern hsv_to_rgb_algorithm g_hsv_rgb_alg;
#endif

class ByteStream;
class RGBColor;

class HSVColor
{
public:
  HSVColor();
  HSVColor(uint8_t hue, uint8_t sat, uint8_t val);

  // assignment from uint32_t
  HSVColor(uint32_t dwVal);
  HSVColor &operator=(const uint32_t &rhs);

  // construction/assignment from RGB
  HSVColor(const RGBColor &rhs);
  HSVColor &operator=(const RGBColor &rhs);

  // equality operators
  bool operator==(const HSVColor &other) const;
  bool operator!=(const HSVColor &other) const;

  bool empty() const;
  void clear();

  uint32_t raw() const { return ((uint32_t)hue << 16) | ((uint32_t)sat << 8) | (uint32_t)val; }

  // public members
  uint8_t hue;
  uint8_t sat;
  uint8_t val;
};

class RGBColor
{
public:
  RGBColor();
  RGBColor(uint8_t red, uint8_t green, uint8_t blue);

  // assignment from uint32_t
  RGBColor(uint32_t dwVal);
  RGBColor &operator=(const uint32_t &rhs);

  RGBColor(const HSVColor &rhs);
  RGBColor &operator=(const HSVColor &rhs);

  // equality operators
  bool operator==(const RGBColor &other) const;
  bool operator!=(const RGBColor &other) const;

  bool empty() const;
  void clear();

  RGBColor adjustBrightness(uint8_t fadeBy);
  #ifdef HELIOS_CLI
  // Helios CLI Colors
  RGBColor scaleUpBrightness(float scaleFactor);
  #endif

  uint32_t raw() const { return ((uint32_t)red << 16) | ((uint32_t)green << 8) | (uint32_t)blue; }

  // public members
  uint8_t red;
  uint8_t green;
  uint8_t blue;
};

// Stolen from FastLED hsv to rgb full rainbox where all colours
// are given equal weight, this makes for-example yellow larger
// best to use this function as it is the legacy choice
RGBColor hsv_to_rgb_rainbow(const HSVColor &rhs);
// generic hsv to rgb conversion nothing special
RGBColor hsv_to_rgb_generic(const HSVColor &rhs);

// Convert rgb to hsv with generic fast method
HSVColor rgb_to_hsv_generic(const RGBColor &rhs);

#endif