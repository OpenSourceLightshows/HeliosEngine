#include "Patterns.h"

#include "Storage.h"
#include "Pattern.h"

// define arrays of colors, you can reuse these if you have multiple
// modes that use the same colorset -- these demonstrate the max amount
// of colors in each set but you can absolutely list a lesser amount
static const uint32_t color_codes0[] = {RGB_RED, RGB_ORANGE, RGB_YELLOW, RGB_TURQUOISE, RGB_BLUE, RGB_PINK};
static const uint32_t color_codes1[] = {RGB_RED, RGB_CORAL_ORANGE_SAT_MEDIUM, RGB_ORANGE, RGB_YELLOW_SAT_LOW};
static const uint32_t color_codes2[] = {RGB_PURPLE_BRI_LOWEST, RGB_MAGENTA, RGB_HOT_PINK_SAT_MEDIUM, RGB_PINK_SAT_LOWEST};
static const uint32_t color_codes3[] = {RGB_WHITE, RGB_BLUE_BRI_LOWEST, RGB_BLUE_BRI_LOWEST, RGB_BLUE_BRI_LOWEST, RGB_BLUE_BRI_LOWEST, RGB_BLUE_BRI_LOWEST};
static const uint32_t color_codes4[] = {RGB_MAGENTA_BRI_LOWEST, RGB_ROYAL_BLUE_BRI_LOW, RGB_TURQUOISE, RGB_ROYAL_BLUE_BRI_LOW, RGB_MAGENTA_BRI_LOWEST, RGB_OFF};
static const uint32_t color_codes5[] = {RGB_RED, RGB_HOT_PINK, RGB_ROYAL_BLUE, RGB_BLUE, RGB_GREEN, RGB_YELLOW};

// Define Colorset configurations for each slot
struct default_colorset {
  uint8_t num_cols;
  const uint32_t *cols;
};

// the array of colorset entries, make sure the number on the left reflects
// the number of colors in the array on the right
static const default_colorset default_colorsets[] = {
  { 6, color_codes0 },  // 0 Lightside
  { 4, color_codes1 },  // 1 Sauna
  { 4, color_codes2 },  // 2 Butterfly
  { 6, color_codes3 },  // 3 Freezer Burn
  { 6, color_codes4 },  // 4 Ice Blade
  { 6, color_codes5 },  // 5 Rainbow Glitter
};

void Patterns::make_default(uint8_t index, Pattern &pat)
{
  if (index >= NUM_MODE_SLOTS) {
    return;
  }
  PatternArgs args;
  switch (index) {
    case 0:  // Lightside
      args.on_dur = 2;
      args.gap_dur = 40;
      break;
    case 1:  // Sauna
      args.on_dur = 1;
      args.off_dur = 9;
      break;
    case 2:  // Butterfly
      args.on_dur = 1;
      args.off_dur = 9;
      args.gap_dur = 6;
      args.dash_dur = 15;
      break;
    case 3:  // Freezer Burn
      args.on_dur = 1;
      args.off_dur = 9;
      args.dash_dur = 5;
      break;
    case 4:  // Ice Blade
      args.on_dur = 3;
      args.off_dur = 1;
      args.gap_dur = 30;
      break;
    case 5:  // Rainbow Glitter
      args.on_dur = 1;
      args.off_dur = 50;
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

  case PATTERN_RIBBON:
    args.on_dur = 9; // 10 for flashing pattern circles
    break;

  case PATTERN_ULTRA_DOPS:
    args.on_dur = 1;
    args.off_dur = 3;
    break;

  case PATTERN_DOPS:
    args.on_dur = 1;
    args.off_dur = 9;
    break;

  case PATTERN_STROBE:
    args.on_dur = 5;
    args.off_dur = 8; // 10 for flashing pattern circles
    break;

  case PATTERN_HYPNOSTROBE:
    args.on_dur = 14;
    args.off_dur = 10;
    break;

  case PATTERN_STROBIE:
    args.on_dur = 3;
    args.off_dur = 23; // 21 for flashing pattern circles
    break;

  case PATTERN_RAZOR:
    args.on_dur = 3;
    args.off_dur = 1;
    args.gap_dur = 30; // 29 for flashing pattern circles
    break;

  case PATTERN_FLARE:
    args.on_dur = 2;
    args.off_dur = 30; // 28 for flashing pattern circles
    break;

  case PATTERN_BURST:
    args.on_dur = 3;
    args.off_dur = 40; // 37 for flashing pattern circles
    break;

  case PATTERN_GLOW:
    args.on_dur = 2;
    args.gap_dur = 40; // 39 for flashing pattern circles
    break;

  case PATTERN_FLICKER:
    args.on_dur = 1;
    args.off_dur = 50; // 44 for flashing pattern circles
    break;

  case PATTERN_FLASH:
    args.on_dur = 10;
    args.off_dur = 250; // 120 for flashing pattern circles
    break;

  case PATTERN_MORPH:
    args.on_dur = 9;
    args.blend_speed = 5; // 14 for flashing pattern circles
    break;

  case PATTERN_MORPH_STROBE:
    args.on_dur = 5;
    args.off_dur = 8;
    args.blend_speed = 10; // 19 for flashing pattern circles
    break;

  case PATTERN_MORPH_STROBIE:
    args.on_dur = 3;
    args.off_dur = 23;
    args.blend_speed = 10; // 35 for flashing pattern circles
    break;

  case PATTERN_MORPH_GLOW:
    args.on_dur = 1;
    args.off_dur = 3;
    args.gap_dur = 40; // 36 for flashing pattern circles
    args.blend_speed = 30;
    break;

  case PATTERN_DASH_DOPS:
    args.on_dur = 1;
    args.off_dur = 9;
    args.gap_dur = 6;
    args.dash_dur = 15; // 17 for flashing pattern circles
    break;

  case PATTERN_DASH_DOT:
    args.on_dur = 2;
    args.off_dur = 3;
    args.dash_dur = 24; // 22 for flashing pattern circles
    break;

  case PATTERN_WAVE_PARTICLE:
    args.on_dur = 1;
    args.off_dur = 9;
    args.dash_dur = 5; // 10 for flashing pattern circles
    break;

  case PATTERN_LIGHTSPEED:
    args.on_dur = 2;
    args.off_dur = 3;
    args.dash_dur = 24; // 23 for flashing pattern circles
    args.blend_speed = 10;
    break;
  }

  pat.setArgs(args);
}
