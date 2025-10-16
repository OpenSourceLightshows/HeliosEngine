#include "Pattern.h"

#include "TimeControl.h"
#include "Colorset.h"

#include "HeliosConfig.h"
#include "Led.h"

#include <string.h> /* for memcpy */

/* Forward declarations for internal functions */
static void pattern_on_blink_on(pattern_t *pat);
static void pattern_on_blink_off(pattern_t *pat);
static void pattern_begin_gap(pattern_t *pat);
static void pattern_begin_dash(pattern_t *pat);
static void pattern_next_state(pattern_t *pat, uint8_t timing);
static void pattern_blend_blink_on(pattern_t *pat);
static void pattern_interpolate(uint8_t *current, const uint8_t next, uint8_t blend_speed);
static void pattern_tick_fade(pattern_t *pat);

/* ==================================
 *  Pattern Args Functions */

void pattern_args_init(pattern_args_t *args, uint8_t on, uint8_t off, uint8_t gap,
                      uint8_t dash, uint8_t group, uint8_t blend, uint8_t fade)
{
  args->on_dur = on;
  args->off_dur = off;
  args->gap_dur = gap;
  args->dash_dur = dash;
  args->group_size = group;
  args->blend_speed = blend;
  args->fade_dur = fade;
}

/* ==================================
 *  Pattern Functions */

void pattern_init(pattern_t *pat, uint8_t onDur, uint8_t offDur, uint8_t gap,
                 uint8_t dash, uint8_t group, uint8_t blend, uint8_t fade)
{
  pattern_args_init(&pat->m_args, onDur, offDur, gap, dash, group, blend, fade);
  pat->m_patternFlags = 0;
  colorset_init(&pat->m_colorset);
  pat->m_groupCounter = 0;
  pat->m_state = STATE_BLINK_ON;
  timer_init_default(&pat->m_blinkTimer);
  rgb_init(&pat->m_cur);
  rgb_init(&pat->m_next);
  pat->m_fadeValue = 0;
  pat->m_fadeStartTime = 0;
}

void pattern_init_with_args(pattern_t *pat, const pattern_args_t *args)
{
  pattern_init(pat, args->on_dur, args->off_dur, args->gap_dur,
              args->dash_dur, args->group_size, args->blend_speed, args->fade_dur);
}

void pattern_init_state(pattern_t *pat)
{
  colorset_reset_index(&pat->m_colorset);

  /* Reset the fade start time to the current time */
  pat->m_fadeStartTime = time_get_current_time();

  /* the default state to begin with */
  pat->m_state = STATE_BLINK_ON;
  /* if a dash is present then always start with the dash because
   * it consumes the first color in the colorset */
  if (pat->m_args.dash_dur > 0) {
    pat->m_state = STATE_BEGIN_DASH;
  }
  /* if there's no on duration or dash duration the led is just disabled */
  if ((!pat->m_args.on_dur && !pat->m_args.dash_dur) || !colorset_num_colors(&pat->m_colorset)) {
    pat->m_state = STATE_DISABLED;
  }
  pat->m_groupCounter = pat->m_args.group_size ? pat->m_args.group_size : (colorset_num_colors(&pat->m_colorset) - (pat->m_args.dash_dur != 0));

  if (pat->m_args.blend_speed > 0) {
    /* convert current/next colors to HSV but only if we are doing a blend */
    pat->m_cur = colorset_get_next(&pat->m_colorset);
    pat->m_next = colorset_get_next(&pat->m_colorset);
  } else if (pat->m_args.fade_dur) {
    /* if there is a fade dur and no blend need to iterate colorset */
    colorset_get_next(&pat->m_colorset);
  }

  /* Initialize the fluctuating fade value */
  pat->m_fadeValue = 0;
}

static void pattern_tick_fade(pattern_t *pat)
{
  uint32_t now = time_get_current_time();
  /* Calculate relative time since pattern was initialized */
  uint32_t relativeTime = now - pat->m_fadeStartTime;
  uint32_t duration = pat->m_args.fade_dur * 10;

  /* only tick forward every fade_dur ticks */
  if (!relativeTime || (relativeTime % duration) != 0) {
    return;
  }

  /* count the number of steps based on relative time */
  uint32_t steps = relativeTime / duration;
  uint32_t range = pat->m_args.off_dur;

  /* make sure the range is non-zero */
  if (range == 0) {
    pat->m_fadeValue = 0;
    return;
  }

  uint32_t double_range = range * 2;
  uint32_t step = steps % double_range;

  /* Triangle wave: up from 0 to range, then down to 0 */
  pat->m_fadeValue = (step < range) ? step : (double_range - step - 1);

  /* iterate color when at lowest point */
  if (step == 0) {
    colorset_get_next(&pat->m_colorset);
  }
}

void pattern_play(pattern_t *pat)
{
  /* tick forward the fade logic each tick */
  if (pattern_is_fade(pat)) {
    pattern_tick_fade(pat);
  }

  /* Sometimes the pattern needs to cycle multiple states in a single frame so
   * instead of using a loop or recursion I have just used a simple goto */
replay:

  /* its kinda evolving as i go */
  switch (pat->m_state) {
  case STATE_DISABLED:
    return;
  case STATE_BLINK_ON:
    if (pat->m_args.on_dur > 0) {
      pattern_on_blink_on(pat);
      --pat->m_groupCounter;
      /* When in ON state, use current fading on-time */
      pattern_next_state(pat, pat->m_args.on_dur + pat->m_fadeValue);
      return;
    }
    pat->m_state = STATE_BLINK_OFF;
  case STATE_BLINK_OFF:
    /* the whole 'should blink off' situation is tricky because we might need
     * to go back to blinking on if our colorset isn't at the end yet */
    if (pat->m_groupCounter > 0 || (!pat->m_args.gap_dur && !pat->m_args.dash_dur)) {
      if (pat->m_args.off_dur > 0) {
        pattern_on_blink_off(pat);
        pattern_next_state(pat, pat->m_args.off_dur - pat->m_fadeValue);
        return;
      }
      if (pat->m_groupCounter > 0 && pat->m_args.on_dur > 0) {
        pat->m_state = STATE_BLINK_ON;
        goto replay;
      }
    }
    pat->m_state = STATE_BEGIN_GAP;
  case STATE_BEGIN_GAP:
    pat->m_groupCounter = pat->m_args.group_size ? pat->m_args.group_size : (colorset_num_colors(&pat->m_colorset) - (pat->m_args.dash_dur != 0));
    if (pat->m_args.gap_dur > 0) {
      pattern_begin_gap(pat);
      pattern_next_state(pat, pat->m_args.gap_dur);
      return;
    }
    pat->m_state = STATE_BEGIN_DASH;
  case STATE_BEGIN_DASH:
    if (pat->m_args.dash_dur > 0) {
      pattern_begin_dash(pat);
      pattern_next_state(pat, pat->m_args.dash_dur);
      return;
    }
    pat->m_state = STATE_BEGIN_GAP2;
  case STATE_BEGIN_GAP2:
    if (pat->m_args.dash_dur > 0 && pat->m_args.gap_dur > 0) {
      pattern_begin_gap(pat);
      pattern_next_state(pat, pat->m_args.gap_dur);
      return;
    }
    pat->m_state = STATE_BLINK_ON;
    goto replay;
  default:
    break;
  }

  if (!timer_alarm(&pat->m_blinkTimer)) {
    /* no alarm triggered just stay in current state, return and don't transition states */
    return;
  }

  /* this just transitions the state into the next state, with some edge conditions for
   * transitioning to different states under certain circumstances. Honestly this is
   * a nightmare to read now and idk how to fix it */
  if (pat->m_state == STATE_IN_GAP2 || (pat->m_state == STATE_OFF && pat->m_groupCounter > 0)) {
    /* this is an edge condition for when in the second gap or off in the non-last off blink
     * then the state actually needs to jump backwards rather than iterate */
    pat->m_state = pat->m_args.on_dur ? STATE_BLINK_ON : (pat->m_args.dash_dur ? STATE_BEGIN_DASH : STATE_BEGIN_GAP);
  } else if (pat->m_state == STATE_OFF && (!pat->m_groupCounter || colorset_num_colors(&pat->m_colorset) == 1)) {
    /* this is an edge condition when the state is off but this is the last off blink in the
     * group or there's literally only one color in the group then if there is more blinks
     * left in the group we need to cycle back to blink on instead of to the next state */
    pat->m_state = (pat->m_groupCounter > 0) ? STATE_BLINK_ON : STATE_BEGIN_GAP;
  } else {
    /* this is the standard case, iterate to the next state */
    pat->m_state = (enum pattern_state)(pat->m_state + 1);
  }
  /* poor-mans recurse with the new state change (this transitions to a new state within the same tick) */
  goto replay;
}

void pattern_set_args(pattern_t *pat, const pattern_args_t *args)
{
  memcpy(&pat->m_args, args, sizeof(pattern_args_t));
}

pattern_args_t pattern_get_args(const pattern_t *pat)
{
  return pat->m_args;
}

pattern_args_t *pattern_args_ptr(pattern_t *pat)
{
  return &pat->m_args;
}

static void pattern_on_blink_on(pattern_t *pat)
{
  if (pattern_is_blend(pat)) {
    pattern_blend_blink_on(pat);
    return;
  }

  /* Check if this is a fading duration pattern */
  if (pattern_is_fade(pat)) {
    rgb_color_t cur_col = colorset_cur(&pat->m_colorset);
    led_set_rgb(&cur_col);
    return;
  }

  rgb_color_t next_col = colorset_get_next(&pat->m_colorset);
  led_set_rgb(&next_col);
}

static void pattern_on_blink_off(pattern_t *pat)
{
  (void)pat; /* unused */
  led_clear();
}

static void pattern_begin_gap(pattern_t *pat)
{
  (void)pat; /* unused */
  led_clear();
}

static void pattern_begin_dash(pattern_t *pat)
{
  rgb_color_t next_col = colorset_get_next(&pat->m_colorset);
  led_set_rgb(&next_col);
}

static void pattern_next_state(pattern_t *pat, uint8_t timing)
{
  timer_init(&pat->m_blinkTimer, timing);
  pat->m_state = (enum pattern_state)(pat->m_state + 1);
}

colorset_t pattern_get_colorset(const pattern_t *pat)
{
  return pat->m_colorset;
}

colorset_t *pattern_colorset_ptr(pattern_t *pat)
{
  return &pat->m_colorset;
}

void pattern_set_colorset(pattern_t *pat, const colorset_t *set)
{
  colorset_copy(&pat->m_colorset, set);
  pattern_init_state(pat);
}

void pattern_clear_colorset(pattern_t *pat)
{
  colorset_clear(&pat->m_colorset);
}

uint8_t pattern_equals(const pattern_t *pat, const pattern_t *other)
{
  if (!other) {
    return 0;
  }
  /* compare the colorset */
  if (!colorset_equals(&pat->m_colorset, &other->m_colorset)) {
    return 0;
  }
  /* compare the args of each pattern for equality */
  if (memcmp(&pat->m_args, &other->m_args, sizeof(pattern_args_t)) != 0) {
    return 0;
  }
  /* if those match then it's effectively the same
   * pattern even if anything else is different */
  return 1;
}

void pattern_update_color(pattern_t *pat, uint8_t index, const rgb_color_t *col)
{
  colorset_set(&pat->m_colorset, index, *col);
  pattern_init_state(pat);
}

uint32_t pattern_crc32(const pattern_t *pat)
{
  uint32_t hash = 5381;
  uint8_t i;
  for (i = 0; i < PATTERN_SIZE; ++i) {
    hash = ((hash << 5) + hash) + ((uint8_t *)pat)[i];
  }
  return hash;
}

uint32_t pattern_get_flags(const pattern_t *pat)
{
  return pat->m_patternFlags;
}

uint8_t pattern_has_flags(const pattern_t *pat, uint32_t flags)
{
  return (pat->m_patternFlags & flags) != 0;
}

uint8_t pattern_is_blend(const pattern_t *pat)
{
  return pat->m_args.blend_speed > 0;
}

uint8_t pattern_is_fade(const pattern_t *pat)
{
  return pat->m_args.fade_dur > 0;
}

static void pattern_blend_blink_on(pattern_t *pat)
{
  /* if we reached the next color, then cycle the colorset
   * like normal and begin playing the next color */
  if (rgb_equals(&pat->m_cur, &pat->m_next)) {
    pat->m_next = colorset_get_next(&pat->m_colorset);
  }
  /* interpolate to the next color */
  pattern_interpolate(&pat->m_cur.red, pat->m_next.red, pat->m_args.blend_speed);
  pattern_interpolate(&pat->m_cur.green, pat->m_next.green, pat->m_args.blend_speed);
  pattern_interpolate(&pat->m_cur.blue, pat->m_next.blue, pat->m_args.blend_speed);
  /* set the color */
  led_set_rgb(&pat->m_cur);
}

static void pattern_interpolate(uint8_t *current, const uint8_t next, uint8_t blend_speed)
{
  if (*current < next) {
    uint8_t step = (next - *current) > blend_speed ? blend_speed : (next - *current);
    *current += step;
  } else if (*current > next) {
    uint8_t step = (*current - next) > blend_speed ? blend_speed : (*current - next);
    *current -= step;
  }
}

