#include "Patterns.h"

#include "Storage.h"
#include "Pattern.h"
#include "ColorConstants.h"

// 
static const uint32_t color_codes0[] = {RGB_RED, RGB_TURQUOISE_BRI_MEDIUM, RGB_WHITE_BRI_LOW}; // 
static const uint32_t color_codes1[] = {RGB_MAGENTA_BRI_LOW, RGB_ICE_BLUE_BRI_LOW, RGB_GREEN_BRI_LOW}; // 
static const uint32_t color_codes2[] = {RGB_YELLOW_BRI_LOW, RGB_PURPLE}; // 

// Define Colorset configurations for each slot
struct default_colorset_t {
  uint8_t num_cols;
  const uint32_t *cols;
};

// 
static const struct default_colorset_t default_colorsets[] = {
  { 3, color_codes0 },  // 
  { 3, color_codes1 },  // 
  { 2, color_codes2 },  // 
};

void patterns_make_default(uint8_t index, pattern_t *pat)
{
  if (index >= NUM_MODE_SLOTS) {
    return;
  }
  pattern_args_t args;
  pattern_args_init(&args, 0, 0, 0, 0, 0, 0, 0);
  switch (index) {
    case 0:  // 
      args.on_dur = 1;
      args.off_dur = 9;
      break;
    case 1:  // 
      args.on_dur = 3;
      args.off_dur = 23;
      args.blend_speed = 10;
      break;
    case 2:  // 
      args.on_dur = 1;
      args.off_dur = 9;
      args.gap_dur = 6;
      args.dash_dur = 15;
      break;
  }
  // assign default args
  pattern_set_args(pat, &args);
  // build the set out of the defaults
  colorset_t set;
  colorset_init_array(&set, default_colorsets[index].num_cols, default_colorsets[index].cols);
  // assign default colorset
  pattern_set_colorset(pat, &set);
}

void patterns_make_pattern(enum pattern_id id, pattern_t *pat)
{
  pattern_args_t args;
  pattern_args_init(&args, 0, 0, 0, 0, 0, 0, 0);
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

  pattern_set_args(pat, &args);
}
