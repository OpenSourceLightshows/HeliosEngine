#include "Patterns.h"

#include "Storage.h"
#include "Pattern.h"
#include "ColorConstants.h"

/* define arrays of colors, you can reuse these if you have multiple
 * modes that use the same colorset -- these demonstrate the max amount
 * of colors in each set but you can absolutely list a lesser amount */
static const uint32_t color_codes0[] = {RGB_RED, RGB_GREEN, RGB_BLUE}; /* Rainbow Flow */

// Define Colorset configurations for each slot
struct default_colorset_t {
  uint8_t num_cols;
  const uint32_t *cols;
};

/* the array of colorset entries, make sure the number on the left reflects
 * the number of colors in the array on the right */
static const struct default_colorset_t default_colorsets[] = {
  { 3, color_codes0 },  /* 0 Rainbow Flow */
};

void patterns_make_default(uint8_t index, pattern_t *pat)
{
  if (index >= NUM_MODE_SLOTS) {
    return;
  }
  pattern_args_t args;
  pattern_args_init(&args, 0, 0, 0, 0, 0, 0, 0);
  switch (index) {
    case 0:  /* Rainbow Flow */
      args.on_dur = 3;
      args.off_dur = 23;
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

  case PATTERN_RAZOR:
    args.on_dur = 3;
    args.off_dur = 1;
    args.gap_dur = 30; /* 29 for flashing pattern circles */
    break;

  case PATTERN_DASH_DOPS:
    args.on_dur = 1;
    args.off_dur = 9;
    args.gap_dur = 6;
    args.dash_dur = 15;
    break;

  }

  pattern_set_args(pat, &args);
}

