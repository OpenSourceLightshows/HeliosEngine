#include "Colortypes.h"

#if ALTERNATIVE_HSV_RGB == 1
// global hsv to rgb algorithm selector
hsv_to_rgb_algorithm g_hsv_rgb_alg = HSV_TO_RGB_GENERIC;
#endif

HSVColor::HSVColor() :
  hue(0),
  sat(0),
  val(0)
{
}

HSVColor::HSVColor(uint8_t hue, uint8_t sat, uint8_t val) :
  hue(hue), sat(sat), val(val)
{
}

HSVColor::HSVColor(uint32_t dwVal) :
  HSVColor()
{
  *this = dwVal;
}

// assignment from uint32_t
HSVColor &HSVColor::operator=(const uint32_t &rhs)
{
  hue = ((rhs >> 16) & 0xFF);
  sat = ((rhs >> 8) & 0xFF);
  val = (rhs & 0xFF);
  return *this;
}

// construction/assignment from RGB
HSVColor::HSVColor(const RGBColor &rhs)
{
  *this = rhs;
}

HSVColor &HSVColor::operator=(const RGBColor &rhs)
{
  // always use generic
  *this = rgb_to_hsv_generic(rhs);
  return *this;
}

bool HSVColor::operator==(const HSVColor &other) const
{
  return (other.raw() == raw());
}

bool HSVColor::operator!=(const HSVColor &other) const
{
  return (other.raw() != raw());
}

bool HSVColor::empty() const
{
  return !hue && !sat && !val;
}

void HSVColor::clear()
{
  hue = 0;
  sat = 0;
  val = 0;
}

// ==========
//  RGBColor

RGBColor::RGBColor() :
  red(0),
  green(0),
  blue(0)
{
}

RGBColor::RGBColor(uint8_t red, uint8_t green, uint8_t blue) :
  red(red), green(green), blue(blue)
{
}

RGBColor::RGBColor(uint32_t dwVal) :
  RGBColor()
{
  *this = dwVal;
}

// assignment from uint32_t
RGBColor &RGBColor::operator=(const uint32_t &rhs)
{
  red = ((rhs >> 16) & 0xFF);
  green = ((rhs >> 8) & 0xFF);
  blue = (rhs & 0xFF);
  return *this;
}

RGBColor::RGBColor(const HSVColor &rhs)
{
  *this = rhs;
}

RGBColor &RGBColor::operator=(const HSVColor &rhs)
{
#if ALTERNATIVE_HSV_RGB == 1
  switch (g_hsv_rgb_alg) {
  case HSV_TO_RGB_RAINBOW:
    *this = hsv_to_rgb_rainbow(rhs);
    break;
  case HSV_TO_RGB_GENERIC:
    *this = hsv_to_rgb_generic(rhs);
    break;
  }
#else
  *this = hsv_to_rgb_generic(rhs);
#endif
  return *this;
}

bool RGBColor::operator==(const RGBColor &other) const
{
  return (other.raw() == raw());
}

bool RGBColor::operator!=(const RGBColor &other) const
{
  return (other.raw() != raw());
}

bool RGBColor::empty() const
{
  return !red && !green && !blue;
}

void RGBColor::clear()
{
  red = 0;
  green = 0;
  blue = 0;
}

// scale down the brightness of a color by some fade amount
RGBColor RGBColor::adjustBrightness(uint8_t fadeBy)
{
  red = (((int)red) * (int)(256 - fadeBy)) >> 8;
  green = (((int)green) * (int)(256 - fadeBy)) >> 8;
  blue = (((int)blue) * (int)(256 - fadeBy)) >> 8;
  return *this;
}

#ifdef HELIOS_CLI
// Adjust brightness to ensure visibility on screens, without floating-point arithmetic and without using max function
RGBColor RGBColor::bringUpBrightness(uint8_t min_brightness) {
  // Adjust each color component using FSCALE8 and ensure it doesn't drop below MIN_BRIGHTNESS
  HSVColor col = *this;
  if (col.val == 0) {
    return col;
  }
  if (col.val < min_brightness) {
    col.val = min_brightness;
  }
  return col;
}
// scale a uint8 by a float value, don't use this on embedded!
#define FSCALE8(x, scale) (uint8_t)(((float)x * scale) > 255 ? 255 : ((float)x * scale))
// return a scaled up the brightness version of the current color
RGBColor RGBColor::scaleBrightness(float scale)
{
  // scale each color up by the given amount and return a new color
  return RGBColor(FSCALE8(red, scale), FSCALE8(green, scale), FSCALE8(blue, scale));
}
#endif
// ========================================================
//  Below are various functions for converting hsv <-> rgb

#if ALTERNATIVE_HSV_RGB == 1
#define SCALE8(i, scale)  (((uint16_t)i * (uint16_t)(scale)) >> 8)
#define FIXFRAC8(N,D) (((N)*256)/(D))

// Stolen from FastLED hsv to rgb full rainbox where all colours
// are given equal weight, this makes for-example yellow larger
// best to use this function as it is the legacy choice
RGBColor hsv_to_rgb_rainbow(const HSVColor &rhs)
{
  RGBColor col;
  // Yellow has a higher inherent brightness than
  // any other color; 'pure' yellow is perceived to
  // be 93% as bright as white.  In order to make
  // yellow appear the correct relative brightness,
  // it has to be rendered brighter than all other
  // colors.
  // Level Y1 is a moderate boost, the default.
  // Level Y2 is a strong boost.
  const uint8_t Y1 = 1;
  const uint8_t Y2 = 0;

  // G2: Whether to divide all greens by two.
  // Depends GREATLY on your particular LEDs
  const uint8_t G2 = 0;

  // Gscale: what to scale green down by.
  // Depends GREATLY on your particular LEDs
  const uint8_t Gscale = 185;


  uint8_t hue = rhs.hue;
  uint8_t sat = rhs.sat;
  uint8_t val = rhs.val;

  uint8_t offset = hue & 0x1F; // 0..31

  // offset8 = offset * 8
  uint8_t offset8 = offset;
  offset8 <<= 3;

  uint8_t third = SCALE8(offset8, (256 / 3)); // max = 85
  uint8_t r, g, b;
  if (!(hue & 0x80)) {
    // 0XX
    if (!(hue & 0x40)) {
      // 00X
      //section 0-1
      if (!(hue & 0x20)) {
        // 000
        //case 0: // R -> O
        r = 255 - third;
        g = third;
        b = 0;
      } else {
        // 001
        //case 1: // O -> Y
        if (Y1) {
          r = 171;
          g = 85 + third;
          b = 0;
        }
        if (Y2) {
          r = 170 + third;
          //uint8_t twothirds = (third << 1);
          uint8_t twothirds = SCALE8(offset8, ((256 * 2) / 3)); // max=170
          g = 85 + twothirds;
          b = 0;
        }
      }
    } else {
      //01X
      // section 2-3
      if (!(hue & 0x20)) {
        // 010
        //case 2: // Y -> G
        if (Y1) {
          //uint8_t twothirds = (third << 1);
          uint8_t twothirds = SCALE8(offset8, ((256 * 2) / 3)); // max=170
          r = 171 - twothirds;
          g = 170 + third;
          b = 0;
        }
        if (Y2) {
          r = 255 - offset8;
          g = 255;
          b = 0;
        }
      } else {
        // 011
        // case 3: // G -> A
        r = 0;
        g = 255 - third;
        b = third;
      }
    }
  } else {
    // section 4-7
    // 1XX
    if (!(hue & 0x40)) {
      // 10X
      if (!(hue & 0x20)) {
        // 100
        //case 4: // A -> B
        r = 0;
        //uint8_t twothirds = (third << 1);
        uint8_t twothirds = SCALE8(offset8, ((256 * 2) / 3)); // max=170
        g = 171 - twothirds; //170?
        b = 85 + twothirds;
      } else {
        // 101
        //case 5: // B -> P
        r = third;
        g = 0;
        b = 255 - third;
      }
    } else {
      if (!(hue & 0x20)) {
        // 110
        //case 6: // P -- K
        r = 85 + third;
        g = 0;
        b = 171 - third;
      } else {
        // 111
        //case 7: // K -> R
        r = 170 + third;
        g = 0;
        b = 85 - third;
      }
    }
  }

  // This is one of the good places to scale the green down,
  // although the client can scale green down as well.
  if (G2) g = g >> 1;
  if (Gscale) g = SCALE8(g, Gscale);

  // Scale down colors if we're desaturated at all
  // and add the brightness_floor to r, g, and b.
  if (sat != 255) {
    if (sat == 0) {
      r = 255; b = 255; g = 255;
    } else {
      if (r) r = SCALE8(r, sat) + 1;
      if (g) g = SCALE8(g, sat) + 1;
      if (b) b = SCALE8(b, sat) + 1;

      uint8_t desat = 255 - sat;
      desat = SCALE8(desat, desat);

      uint8_t brightness_floor = desat;
      r += brightness_floor;
      g += brightness_floor;
      b += brightness_floor;
    }
  }

  // Now scale everything down if we're at value < 255.
  if (val != 255) {
    val = SCALE8(val, val);
    if (val == 0) {
      r = 0; g = 0; b = 0;
    } else {
      // nSCALE8x3_video( r, g, b, val);
      if (r) r = SCALE8(r, val) + 1;
      if (g) g = SCALE8(g, val) + 1;
      if (b) b = SCALE8(b, val) + 1;
    }
  }

  // Here we have the old AVR "missing std X+n" problem again
  // It turns out that fixing it winds up costing more than
  // not fixing it.
  // To paraphrase Dr Bronner, profile! profile! profile!
  col.red = r;
  col.green = g;
  col.blue = b;
  return col;
}
#endif

// generic hsv to rgb conversion nothing special
RGBColor hsv_to_rgb_generic(const HSVColor &rhs)
{
  unsigned char region, remainder, p, q, t;
  RGBColor col;

  if (rhs.sat == 0) {
    col.red = rhs.val;
    col.green = rhs.val;
    col.blue = rhs.val;
    return col;
  }

  // At 1MHz the AVR has ~8x fewer cycles per tick. The original division
  // (hue / 43) compiles to a ~200-cycle software divide on AVR. Replacing it
  // with a branchless compare tree + a LUT subtraction cuts this to ~10 cycles.
  static const uint8_t region_base[6] = { 0, 43, 86, 129, 172, 215 };
  region = (rhs.hue < 86)  ? ((rhs.hue < 43)  ? 0 : 1)
         : (rhs.hue < 172) ? ((rhs.hue < 129) ? 2 : 3)
         :                    ((rhs.hue < 215)  ? 4 : 5);
  remainder = ((rhs.hue - region_base[region]) * 6);

  // extraneous casts to uint16_t are to prevent overflow
  p = (uint8_t)(((uint16_t)(rhs.val) * (255 - rhs.sat)) >> 8);
  q = (uint8_t)(((uint16_t)(rhs.val) * (255 - (((uint16_t)(rhs.sat) * remainder) >> 8))) >> 8);
  t = (uint8_t)(((uint16_t)(rhs.val) * (255 - (((uint16_t)(rhs.sat) * (255 - remainder)) >> 8))) >> 8);

  switch (region) {
  case 0:
    col.red = rhs.val; col.green = t; col.blue = p;
    break;
  case 1:
    col.red = q; col.green = rhs.val; col.blue = p;
    break;
  case 2:
    col.red = p; col.green = rhs.val; col.blue = t;
    break;
  case 3:
    col.red = p; col.green = q; col.blue = rhs.val;
    break;
  case 4:
    col.red = t; col.green = p; col.blue = rhs.val;
    break;
  default:
    col.red = rhs.val; col.green = p; col.blue = q;
    break;
  }
  return col;
}

// Convert rgb to hsv with generic fast method
HSVColor rgb_to_hsv_generic(const RGBColor &rhs)
{
  unsigned char rgbMin, rgbMax;
  rgbMin = rhs.red < rhs.green ? (rhs.red < rhs.blue ? rhs.red : rhs.blue) : (rhs.green < rhs.blue ? rhs.green : rhs.blue);
  rgbMax = rhs.red > rhs.green ? (rhs.red > rhs.blue ? rhs.red : rhs.blue) : (rhs.green > rhs.blue ? rhs.green : rhs.blue);
  HSVColor hsv;

  hsv.val = rgbMax;
  if (hsv.val == 0) {
    hsv.hue = 0;
    hsv.sat = 0;
    return hsv;
  }

  hsv.sat = 255 * (long)(rgbMax - rgbMin) / hsv.val;
  if (hsv.sat == 0) {
    hsv.hue = 0;
    return hsv;
  }

  if (rgbMax == rhs.red) {
    hsv.hue = 0 + 43 * (rhs.green - rhs.blue) / (rgbMax - rgbMin);
  } else if (rgbMax == rhs.green) {
    hsv.hue = 85 + 43 * (rhs.blue - rhs.red) / (rgbMax - rgbMin);
  } else {
    hsv.hue = 171 + 43 * (rhs.red - rhs.green) / (rgbMax - rgbMin);
  }
  return hsv;
}
