#include "Patterns.h"

#include "Storage.h"
#include "Pattern.h"

// define arrays of colors, you can reuse these if you have multiple
// modes that use the same colorset -- these demonstrate the max amount
// of colors in each set but you can absolutely list a lesser amount
static const uint32_t color_codes0[] = {RGB_RED, RGB_ORANGE, RGB_YELLOW, RGB_TURQUOISE, RGB_BLUE, RGB_PINK};
static const uint32_t color_codes1[] = {RGB_RED, RGB_CORAL_ORANGE_SAT_MEDIUM, RGB_ORANGE, RGB_YELLOW_SAT_LOW};
static const uint32_t color_codes2[] = {RGB_PURPLE_BRI_LOWEST, RGB_MAGENTA, RGB_HOT_PINK_SAT_MEDIUM, RGB_PINK_SAT_LOWEST};

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

  case PATTERN_GLOW:
    args.on_dur = 2;
    args.gap_dur = 40; // 39 for flashing pattern circles
    break;

  case PATTERN_FLICKER:
    args.on_dur = 1;
    args.off_dur = 50; // 44 for flashing pattern circles
    break;
  }

  pat.setArgs(args);
}
