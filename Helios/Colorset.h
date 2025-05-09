#ifndef COLORSET_H
#define COLORSET_H

#include "Colortypes.h"

#include "HeliosConfig.h"

class Random;

class Colorset
{
public:
  // empty colorset
  Colorset();
  // constructor for 1-8 color slots
  Colorset(RGBColor c1, RGBColor c2 = RGB_OFF, RGBColor c3 = RGB_OFF,
    RGBColor c4 = RGB_OFF, RGBColor c5 = RGB_OFF, RGBColor c6 = RGB_OFF,
    RGBColor c7 = RGB_OFF, RGBColor c8 = RGB_OFF);
  Colorset(uint8_t numCols, const uint32_t *cols);
  ~Colorset();

  // copy and assignment operators
  Colorset(const Colorset &other);

  // equality operators
  bool operator==(const Colorset &other) const;
  bool operator!=(const Colorset &other) const;

  // initialize the colorset
  void init(RGBColor c1 = RGB_OFF, RGBColor c2 = RGB_OFF, RGBColor c3 = RGB_OFF,
    RGBColor c4 = RGB_OFF, RGBColor c5 = RGB_OFF, RGBColor c6 = RGB_OFF,
    RGBColor c7 = RGB_OFF, RGBColor c8 = RGB_OFF);

  // clear the colorset
  void clear();

  // pointer comparison
  bool equals(const Colorset &set) const;
  bool equals(const Colorset *set) const;

  // crc the colorset
  uint32_t crc32() const;

  // index operator to access color index
  RGBColor operator[](int index) const;

  enum ValueStyle : uint8_t
  {
    // Random values
    VAL_STYLE_RANDOM = 0,
    // First color low value, the rest are random
    VAL_STYLE_LOW_FIRST_COLOR,
    // First color high value, the rest are low
    VAL_STYLE_HIGH_FIRST_COLOR,
    // Alternat between high and low value
    VAL_STYLE_ALTERNATING,
    // Ascending values from low to high
    VAL_STYLE_ASCENDING,
    // Descending values from high to low
    VAL_STYLE_DESCENDING,
    // Constant value
    VAL_STYLE_CONSTANT,
    // Total number of value styles
    VAL_STYLE_COUNT
  };

  // add a single color
  bool addColor(RGBColor col);
  bool addColorHSV(uint8_t hue, uint8_t sat, uint8_t val);
  void addColorWithValueStyle(Random &ctx, uint8_t hue, uint8_t sat,
    ValueStyle valStyle, uint8_t numColors, uint8_t colorPos);
  void removeColor(uint8_t index);

  // various modes of randomization types to use with randomizeColors
  enum ColorMode {
    // randomize with color theory
    COLOR_MODE_COLOR_THEORY,
    // randomize a nonochromatic set
    COLOR_MODE_MONOCHROMATIC,
    // randomize an evenly spaced hue set
    COLOR_MODE_EVENLY_SPACED,

    // total different randomize modes above
    COLOR_MODE_COUNT,

    // EXTRA OPTION: randomly pick one of the other 3 options
    COLOR_MODE_RANDOMLY_PICK = COLOR_MODE_COUNT,
  };
  // function to randomize the colors with various different modes of randomization
  void randomizeColors(Random &ctx, uint8_t numColors, ColorMode color_mode);

  // fade all of the colors in the set
  void adjustBrightness(uint8_t fadeby);

  // get a color from the colorset
  RGBColor get(uint8_t index = 0) const;

  // set an rgb color in a slot, or add a new color if you specify
  // a slot higher than the number of colors in the colorset
  void set(uint8_t index, RGBColor col);

  // skip some amount of colors
  void skip(int32_t amount = 1);

  // get current color in cycle
  RGBColor cur();

  // set the current index of the colorset
  void setCurIndex(uint8_t index);
  void resetIndex();

  // the current index
  uint8_t curIndex() const { return m_curIndex; }

  // get the prev color in cycle
  RGBColor getPrev();

  // get the next color in cycle
  RGBColor getNext();

  // peek at the color indexes from current but don't iterate
  RGBColor peek(int32_t offset) const;

  // better wording for peek 1 ahead
  RGBColor peekNext() const { return peek(1); }

  // the number of colors in the palette
  uint8_t numColors() const { return m_numColors; }

  // whether the colorset is currently on the first color or last color
  bool onStart() const;
  bool onEnd() const;
private:
  // palette of colors
  RGBColor m_palette[NUM_COLOR_SLOTS];
  // the actual number of colors in the set
  uint8_t m_numColors;
  // the current index, starts at UINT8_MAX so that
  // the very first call to getNext will iterate to 0
  uint8_t m_curIndex;
};

#endif
