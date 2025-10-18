#ifndef COLORSET_H
#define COLORSET_H

#ifdef __cplusplus
extern "C" {
#endif

#include "Colortypes.h"

#include "HeliosConfig.h"

// Forward declarations (with include guards to prevent SDCC conflicts)
#ifndef RANDOM_T_FORWARD_DECLARED
#define RANDOM_T_FORWARD_DECLARED
typedef struct random_t random_t;
#endif

#ifndef COLORSET_T_FORWARD_DECLARED
#define COLORSET_T_FORWARD_DECLARED
typedef struct colorset_t colorset_t;
#endif

enum colorset_value_style
{
  // Random values
  VAL_STYLE_RANDOM = 0,
  // First color low value, the rest are random
  VAL_STYLE_LOW_FIRST_COLOR,
  // First color high value, the rest are low
  VAL_STYLE_HIGH_FIRST_COLOR,
  // Alternate between high and low value
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

enum colorset_color_mode
{
  // randomize with color theory
  COLOR_MODE_COLOR_THEORY,
  // randomize a monochromatic set
  COLOR_MODE_MONOCHROMATIC,
  // randomize an evenly spaced hue set
  COLOR_MODE_EVENLY_SPACED,

  // total different randomize modes above
  COLOR_MODE_COUNT,

  // EXTRA OPTION: randomly pick one of the other 3 options
  COLOR_MODE_RANDOMLY_PICK = COLOR_MODE_COUNT,
};

struct colorset_t
{
  // palette of colors
  rgb_color_t m_palette[NUM_COLOR_SLOTS];
  // the actual number of colors in the set
  uint8_t m_numColors;
  // the current index, starts at 255 so that
  // the very first call to colorset_getNext will iterate to 0
  uint8_t m_curIndex;
};

// Empty colorset
void colorset_init(colorset_t *set);

// Initialize with up to 8 colors
void colorset_init_multi(colorset_t *set, rgb_color_t c1, rgb_color_t c2, rgb_color_t c3,
    rgb_color_t c4, rgb_color_t c5, rgb_color_t c6, rgb_color_t c7, rgb_color_t c8);

// Initialize from array of colors
void colorset_init_array(colorset_t *set, uint8_t numCols, const uint32_t *cols);

// Copy colorset
void colorset_copy(colorset_t *dest, const colorset_t *src);

// Equality operators
uint8_t colorset_equals(const colorset_t *a, const colorset_t *b);

// Clear the colorset
void colorset_clear(colorset_t *set);

// CRC the colorset
uint32_t colorset_crc32(const colorset_t *set);

// Index operator to access color index
rgb_color_t colorset_get_at_index(const colorset_t *set, int index);

// Add a single color
uint8_t colorset_add_color(colorset_t *set, rgb_color_t col);
uint8_t colorset_add_color_hsv(colorset_t *set, uint8_t hue, uint8_t sat, uint8_t val);
void colorset_add_color_with_value_style(colorset_t *set, random_t *ctx, uint8_t hue, uint8_t sat,
    enum colorset_value_style valStyle, uint8_t numColors, uint8_t colorPos);
void colorset_remove_color(colorset_t *set, uint8_t index);

// Function to randomize the colors with various different modes of randomization
void colorset_randomize_colors(colorset_t *set, random_t *ctx, uint8_t numColors, enum colorset_color_mode color_mode);

// Fade all of the colors in the set
void colorset_adjust_brightness(colorset_t *set, uint8_t fadeby);

// Get a color from the colorset
rgb_color_t colorset_get(const colorset_t *set, uint8_t index);

// Set an rgb color in a slot, or add a new color if you specify
// a slot higher than the number of colors in the colorset
void colorset_set(colorset_t *set, uint8_t index, rgb_color_t col);

// Skip some amount of colors
void colorset_skip(colorset_t *set, int32_t amount);

// Get current color in cycle
rgb_color_t colorset_cur(const colorset_t *set);

// Set the current index of the colorset
void colorset_set_cur_index(colorset_t *set, uint8_t index);
void colorset_reset_index(colorset_t *set);

// The current index
uint8_t colorset_cur_index(const colorset_t *set);

// Get the prev color in cycle
rgb_color_t colorset_get_prev(colorset_t *set);

// Get the next color in cycle
rgb_color_t colorset_get_next(colorset_t *set);

// Peek at the color indexes from current but don't iterate
rgb_color_t colorset_peek(const colorset_t *set, int32_t offset);

// Better wording for peek 1 ahead
rgb_color_t colorset_peek_next(const colorset_t *set);

// The number of colors in the palette
uint8_t colorset_num_colors(const colorset_t *set);

// Whether the colorset is currently on the first color or last color
uint8_t colorset_on_start(const colorset_t *set);
uint8_t colorset_on_end(const colorset_t *set);

#ifdef __cplusplus
}
#endif

#endif
