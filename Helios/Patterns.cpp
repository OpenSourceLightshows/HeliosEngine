#include "Patterns.h"

#include "Storage.h"
#include "Pattern.h"

// define arrays of colors, you can reuse these if you have multiple
// modes that use the same colorset -- these demonstrate the max amount
// of colors in each set but you can absolutely list a lesser amount
static const uint32_t color_codes0[] = {RGB_RED, RGB_ORANGE, RGB_WHITE}; // Rainbow Flow
static const uint32_t color_codes1[] = {RGB_SEAFOAM, RGB_BLUE, RGB_WHITE}; // Ghostly
static const uint32_t color_codes2[] = {RGB_PURPLE, RGB_MAGENTA, RGB_WHITE}; // Photo Copy

// Define Colorset configurations for each slot
struct default_colorset {
  uint8_t num_cols;
  const uint32_t *cols;
};

// the array of colorset entries, make sure the number on the left reflects
// the number of colors in the array on the right
static const default_colorset default_colorsets[] = {
  { 3, color_codes0 },  // 0 Rainbow Flow
  { 3, color_codes1 },  // 1 Ghostly
  { 3, color_codes2 },  // 2 Photo Copy
};

void Patterns::make_default(uint8_t index, Pattern &pat)
{
  if (index >= NUM_MODE_SLOTS) {
    return;
  }
  PatternArgs args;
  switch (index) {
    case 0:  // Rainbow Flow
      args.on_dur = 1;
      args.off_dur = 9;
      break;
    case 1:  // Ghostly
      args.on_dur = 3;
      args.off_dur = 23;
      args.blend_speed = 10;
      break;
    case 2:  // Photo Copy
      args.on_dur = 1;
      args.off_dur = 10;
      args.gap_dur = 30;
      args.fade_dur = 5;

      break;
  }
  // assign default args
  pat.setArgs(args);
  // build the set out of the defaults
  Colorset set(default_colorsets[index].num_cols, default_colorsets[index].cols);
  // assign default colorset
  pat.setColorset(set);
}

void Patterns::make_pattern(PatternID id, Pattern &pat)
{
  PatternArgs args;
  switch (id)
  {
  default:

  case PATTERN_DOPS:
    args.on_dur = 1;
    args.off_dur = 9;
    break;

  case PATTERN_STROBE:
    args.on_dur = 5;
    args.off_dur = 8;
    break;

  case PATTERN_HYPNOSTROBE:
    args.on_dur = 14;
    args.off_dur = 10;
    break;

  case PATTERN_STROBIE:
    args.on_dur = 3;
    args.off_dur = 23;
    break;

  case PATTERN_FLARE:
    args.on_dur = 2;
    args.off_dur = 30;
    break;

  case PATTERN_MORPH_STROBIE:
    args.on_dur = 3;
    args.off_dur = 23;
    args.blend_speed = 10;
    break;

  case PATTERN_DASH_DOPS:
    args.on_dur = 1;
    args.off_dur = 9;
    args.gap_dur = 6;
    args.dash_dur = 15;
    break;

  case PATTERN_FADE:
    args.on_dur = 1;
    args.off_dur = 15;
    args.fade_dur = 25;
    break;

  case PATTERN_MORPH_FADE:
    args.on_dur = 1;
    args.off_dur = 15;
    args.fade_dur = 15;
    args.blend_speed = 2;
    break;

  case PATTERN_GLITCH_FADE:
    args.on_dur = 1;
    args.off_dur = 10;
    args.gap_dur = 30;
    args.fade_dur = 5;
    break;
  }

  pat.setArgs(args);
}
