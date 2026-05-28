#include "Colorset.h"

#include "Random.h"

#include <string.h>

// when no color is selected in the colorset the index is this
// then when you call colorset_getNext() for the first time it returns
// the 0th color in the colorset and after the index will be 0
#define INDEX_INVALID 255

void colorset_init(colorset_t *set)
{
  memset((void *)set->m_palette, 0, sizeof(set->m_palette));
  set->m_numColors = 0;
  set->m_curIndex = INDEX_INVALID;
}

#ifndef HELIOS_STM8
void colorset_init_multi(colorset_t *set, rgb_color_t c1, rgb_color_t c2, rgb_color_t c3,
    rgb_color_t c4, rgb_color_t c5, rgb_color_t c6, rgb_color_t c7, rgb_color_t c8)
{
  colorset_init(set);
  // would be nice if we could do this another way
  if (!rgb_empty(&c1)) colorset_add_color(set, c1);
  if (!rgb_empty(&c2)) colorset_add_color(set, c2);
  if (!rgb_empty(&c3)) colorset_add_color(set, c3);
  if (!rgb_empty(&c4)) colorset_add_color(set, c4);
  if (!rgb_empty(&c5)) colorset_add_color(set, c5);
  if (!rgb_empty(&c6)) colorset_add_color(set, c6);
  if (!rgb_empty(&c7)) colorset_add_color(set, c7);
  if (!rgb_empty(&c8)) colorset_add_color(set, c8);
}
#endif

void colorset_init_array(colorset_t *set, uint8_t numCols, const uint32_t *cols)
{
  colorset_init(set);
  if (numCols > NUM_COLOR_SLOTS) {
    numCols = NUM_COLOR_SLOTS;
  }
  uint8_t i;
  for (i = 0; i < numCols; ++i) {
    rgb_color_t col;
    rgb_init_from_raw(&col, cols[i]);
    colorset_add_color(set, col);
  }
}

void colorset_copy(colorset_t *dest, const colorset_t *src)
{
  memcpy(dest->m_palette, src->m_palette, sizeof(dest->m_palette));
  dest->m_numColors = src->m_numColors;
  dest->m_curIndex = src->m_curIndex;
}

uint8_t colorset_equals(const colorset_t *a, const colorset_t *b)
{
  // only compare the palettes for equality
  return (a->m_numColors == b->m_numColors) &&
         (memcmp(a->m_palette, b->m_palette, a->m_numColors * sizeof(rgb_color_t)) == 0);
}

void colorset_clear(colorset_t *set)
{
  memset((void *)set->m_palette, 0, sizeof(set->m_palette));
  set->m_numColors = 0;
  colorset_reset_index(set);
}

#ifndef HELIOS_STM8
uint32_t colorset_crc32(const colorset_t *set)
{
  uint32_t hash = 5381;
  uint8_t i;
  for (i = 0; i < set->m_numColors; ++i) {
    hash = ((hash << 5) + hash) + rgb_raw(&set->m_palette[i]);
  }
  return hash;
}
#endif

#ifndef HELIOS_STM8
rgb_color_t colorset_get_at_index(const colorset_t *set, int index)
{
  return colorset_get(set, index);
}
#endif

uint8_t colorset_add_color(colorset_t *set, rgb_color_t col)
{
  if (set->m_numColors >= NUM_COLOR_SLOTS) {
    return 0;
  }
  // insert new color and increment number of colors
  set->m_palette[set->m_numColors] = col;
  set->m_numColors++;
  return 1;
}

uint8_t colorset_add_color_hsv(colorset_t *set, uint8_t hue, uint8_t sat, uint8_t val)
{
  hsv_color_t hsv;
  rgb_color_t rgb;
  hsv_init3(&hsv, hue, sat, val);
  rgb_init_from_hsv(&rgb, &hsv);
  return colorset_add_color(set, rgb);
}

#ifndef HELIOS_STM8
void colorset_add_color_with_value_style(colorset_t *set, random_t *ctx, uint8_t hue, uint8_t sat,
    enum colorset_value_style valStyle, uint8_t numColors, uint8_t colorPos)
{
  if (numColors == 1) {
    colorset_add_color_hsv(set, hue, sat, random_next8(ctx, 16, 255));
    return;
  }
  switch (valStyle) {
  default:
  case VAL_STYLE_RANDOM:
    colorset_add_color_hsv(set, hue, sat, 85 * random_next8(ctx, 1, 4));
    break;
  case VAL_STYLE_LOW_FIRST_COLOR:
    if (set->m_numColors == 0) {
      colorset_add_color_hsv(set, hue, sat, random_next8(ctx, 0, 86));
    } else {
      colorset_add_color_hsv(set, hue, sat, 85 * random_next8(ctx, 1, 4));
    }
    break;
  case VAL_STYLE_HIGH_FIRST_COLOR:
    if (set->m_numColors == 0) {
      colorset_add_color_hsv(set, hue, sat, 255);
    } else {
      colorset_add_color_hsv(set, hue, sat, random_next8(ctx, 0, 86));
    }
    break;
  case VAL_STYLE_ALTERNATING:
    if (set->m_numColors % 2 == 0) {
      colorset_add_color_hsv(set, hue, sat, 255);
    } else {
      colorset_add_color_hsv(set, hue, sat, 85);
    }
    break;
  case VAL_STYLE_ASCENDING:
    colorset_add_color_hsv(set, hue, sat, (colorPos + 1) * (255 / numColors));
    break;
  case VAL_STYLE_DESCENDING:
    colorset_add_color_hsv(set, hue, sat, 255 - (colorPos * (255 / numColors)));
    break;
  case VAL_STYLE_CONSTANT:
    colorset_add_color_hsv(set, hue, sat, 255);
  }
}
#endif

#ifndef HELIOS_STM8
void colorset_remove_color(colorset_t *set, uint8_t index)
{
  if (index >= set->m_numColors) {
    return;
  }
  uint8_t i;
  for (i = index; i < (set->m_numColors - 1); ++i) {
    set->m_palette[i] = set->m_palette[i + 1];
  }
  rgb_clear(&set->m_palette[--set->m_numColors]);
}
#endif

#ifndef HELIOS_STM8
void colorset_randomize_colors(colorset_t *set, random_t *ctx, uint8_t numColors, enum colorset_color_mode mode)
{
  // if they specify randomly pick the color mode then roll it
  if (mode >= COLOR_MODE_RANDOMLY_PICK) {
    mode = (enum colorset_color_mode)(random_next8(ctx, 0, 255) % COLOR_MODE_COUNT);
  }
  colorset_clear(set);
  if (!numColors) {
    numColors = random_next8(ctx, mode == COLOR_MODE_MONOCHROMATIC ? 2 : 1, 9);
  }
  uint8_t randomizedHue = random_next8(ctx, 0, 255);
  uint8_t colorGap = 0;
  if (mode == COLOR_MODE_COLOR_THEORY && numColors > 1) {
    colorGap = random_next8(ctx, 16, 256 / (numColors - 1));
  }
  enum colorset_value_style valStyle = (enum colorset_value_style)random_next8(ctx, 0, VAL_STYLE_COUNT);
  // the doubleStyle decides if some colors are added to the set twice
  uint8_t doubleStyle = 0;
  if (numColors <= 7) {
    doubleStyle = random_next8(ctx, 0, 1);
  }
  if (numColors <= 4) {
    doubleStyle = random_next8(ctx, 0, 2);
  }
  uint8_t i;
  for (i = 0; i < numColors; i++) {
    uint8_t hueToUse;
    uint8_t valueToUse = 255;
    if (mode == COLOR_MODE_COLOR_THEORY) {
      hueToUse = (randomizedHue + (i * colorGap));
    } else if (mode == COLOR_MODE_MONOCHROMATIC) {
      hueToUse = randomizedHue;
      valueToUse = 255 - (i * (256 / numColors));
    } else { // EVENLY_SPACED
      hueToUse = (randomizedHue + (256 / numColors) * i);
    }
    colorset_add_color_with_value_style(set, ctx, hueToUse, valueToUse, valStyle, numColors, i);
    // double all colors or only first color
    if (doubleStyle == 2 || (doubleStyle == 1 && !i)) {
      colorset_add_color_with_value_style(set, ctx, hueToUse, valueToUse, valStyle, numColors, i);
    }
  }
}
#endif

#ifndef HELIOS_STM8
void colorset_adjust_brightness(colorset_t *set, uint8_t fadeby)
{
  uint8_t i;
  for (i = 0; i < set->m_numColors; ++i) {
    rgb_adjust_brightness(&set->m_palette[i], fadeby);
  }
}
#endif

// get a color from the colorset
#ifndef HELIOS_STM8
rgb_color_t colorset_get(const colorset_t *set, uint8_t index)
{
  rgb_color_t result;
  if (index >= set->m_numColors) {
    rgb_init3(&result, 0, 0, 0);
    return result;
  }
  return set->m_palette[index];
}
#endif

#ifndef HELIOS_STM8
void colorset_set(colorset_t *set, uint8_t index, rgb_color_t col)
{
  // special case for 'setting' a color at the edge of the palette,
  // ie adding a new color when you set an index higher than the max
  if (index >= set->m_numColors) {
    if (!colorset_add_color(set, col)) {
      // ERROR_LOGF("Failed to add new color at index %u", index);
    }
    return;
  }
  set->m_palette[index] = col;
}
#endif

#ifndef HELIOS_STM8
void colorset_skip(colorset_t *set, int32_t amount)
{
  if (!set->m_numColors) {
    return;
  }
  // if the colorset hasn't started yet
  if (set->m_curIndex == INDEX_INVALID) {
    set->m_curIndex = 0;
  }

  // first modulate the amount to skip to be within +/- the number of colors
  amount %= (int32_t)set->m_numColors;

  // max = 3
  // m_curIndex = 2
  // amount = -10
  set->m_curIndex = ((int32_t)set->m_curIndex + (int32_t)amount) % (int32_t)set->m_numColors;
  if (set->m_curIndex > set->m_numColors) { // must have wrapped
    // simply wrap it back
    set->m_curIndex += set->m_numColors;
  }
}
#endif

#ifndef HELIOS_STM8
rgb_color_t colorset_cur(const colorset_t *set)
{
  rgb_color_t result;
  if (set->m_curIndex >= set->m_numColors) {
    rgb_init3(&result, 0, 0, 0);
    return result;
  }
  return set->m_palette[set->m_curIndex];
}
#endif

#ifndef HELIOS_STM8
void colorset_set_cur_index(colorset_t *set, uint8_t index)
{
  if (!set->m_numColors) {
    return;
  }
  if (index > (set->m_numColors - 1)) {
    return;
  }
  set->m_curIndex = index;
}
#endif

void colorset_reset_index(colorset_t *set)
{
  set->m_curIndex = INDEX_INVALID;
}

#ifndef HELIOS_STM8
uint8_t colorset_cur_index(const colorset_t *set)
{
  return set->m_curIndex;
}
#endif

#ifndef HELIOS_STM8
rgb_color_t colorset_get_prev(colorset_t *set)
{
  rgb_color_t result;
  if (!set->m_numColors) {
    rgb_init_from_raw(&result, RGB_OFF);
    return result;
  }
  // handle wrapping at 0
  if (set->m_curIndex == 0 || set->m_curIndex == INDEX_INVALID) {
    set->m_curIndex = colorset_num_colors(set) - 1;
  } else {
    set->m_curIndex--;
  }
  // return the color
  return set->m_palette[set->m_curIndex];
}

rgb_color_t colorset_get_next(colorset_t *set)
{
  rgb_color_t result;
  if (!set->m_numColors) {
    rgb_init_from_raw(&result, RGB_OFF);
    return result;
  }
  // iterate current index, let it wrap at max uint8
  set->m_curIndex++;
  // then modulate the result within max colors
  set->m_curIndex %= colorset_num_colors(set);
  // return the color
  return set->m_palette[set->m_curIndex];
}

#endif

#ifndef HELIOS_STM8
rgb_color_t colorset_peek(const colorset_t *set, int32_t offset)
{
  rgb_color_t result;
  if (!set->m_numColors) {
    rgb_init_from_raw(&result, RGB_OFF);
    return result;
  }
  uint8_t nextIndex = 0;
  // get index of the next color
  if (offset >= 0) {
    nextIndex = (set->m_curIndex + offset) % colorset_num_colors(set);
  } else {
    if (offset < -1 * (int32_t)(colorset_num_colors(set))) {
      rgb_init_from_raw(&result, RGB_OFF);
      return result;
    }
    nextIndex = ((set->m_curIndex + colorset_num_colors(set)) + (int)offset) % colorset_num_colors(set);
  }
  // return the color
  return set->m_palette[nextIndex];
}

rgb_color_t colorset_peek_next(const colorset_t *set)
{
  return colorset_peek(set, 1);
}
#endif

uint8_t colorset_num_colors(const colorset_t *set)
{
  return set->m_numColors;
}

uint8_t colorset_on_start(const colorset_t *set)
{
  return (set->m_curIndex == 0);
}

#ifndef HELIOS_STM8
uint8_t colorset_on_end(const colorset_t *set)
{
  if (!set->m_numColors) {
    return 0;
  }
  return (set->m_curIndex == set->m_numColors - 1);
}
#endif

