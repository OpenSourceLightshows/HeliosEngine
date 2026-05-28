#ifndef PATTERN_H
#define PATTERN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "Colorset.h"

#include "Timer.h"
#include "Patterns.h"

// Forward declarations
// typedef struct pattern_args_t pattern_args_t;
// typedef struct pattern_t pattern_t;

// for specifying things like default args
struct pattern_args_t {
  uint8_t on_dur;
  uint8_t off_dur;
  uint8_t gap_dur;
  uint8_t dash_dur;
  uint8_t group_size;
  uint8_t blend_speed;
  uint8_t fade_dur;
};

// Initialize pattern args with all parameters
void pattern_args_init(pattern_args_t *args, uint8_t on, uint8_t off, uint8_t gap,
                       uint8_t dash, uint8_t group, uint8_t blend, uint8_t fade);

// The various different blinking states the pattern can be in
enum pattern_state
{
  // the led is disabled (there is no on or dash)
  STATE_DISABLED,

  // the pattern is blinking on the next color in the set
  STATE_BLINK_ON,
  STATE_ON,

  // the pattern is blinking off
  STATE_BLINK_OFF,
  STATE_OFF,

  // the pattern is starting a gap after a colorset
  STATE_BEGIN_GAP,
  STATE_IN_GAP,

  // the pattern is beginning a dash after a colorset or gap
  STATE_BEGIN_DASH,
  STATE_IN_DASH,

  // the pattern is starting a gap after a dash
  STATE_BEGIN_GAP2,
  STATE_IN_GAP2,
};

struct pattern_t
{
  // ==================================
  //  Pattern Parameters
  pattern_args_t m_args;

  // ==================================
  //  Pattern Members

  // any flags the pattern has
  uint8_t m_patternFlags;
  // a copy of the colorset that this pattern is initialized with
  colorset_t m_colorset;

  // ==================================
  //  Blink Members
  uint8_t m_groupCounter;

  // the state of the current pattern
  enum pattern_state m_state;

  // the blink timer used to measure blink timings
  helios_timer_t m_blinkTimer;

  // ==================================
  //  Blend Members

  // current color and target blend color
  rgb_color_t m_cur;
  rgb_color_t m_next;

  // ==================================
  //  Fade Members

  // shifting value to represent current fade
  uint8_t m_fadeValue;

  // Add a member variable to store when the pattern was last initialized
  uint32_t m_fadeStartTime;
};

// try to not set on duration to 0
void pattern_init(pattern_t *pat, uint8_t onDur, uint8_t offDur, uint8_t gap,
                 uint8_t dash, uint8_t group, uint8_t blend, uint8_t fade);
void pattern_init_with_args(pattern_t *pat, const pattern_args_t *args);

// init the pattern to initial state
void pattern_init_state(pattern_t *pat);

// play the pattern
void pattern_play(pattern_t *pat);

// set/get args
void pattern_set_args(pattern_t *pat, const pattern_args_t *args);
pattern_args_t pattern_get_args(const pattern_t *pat);
pattern_args_t *pattern_args_ptr(pattern_t *pat);

// change the colorset
colorset_t pattern_get_colorset(const pattern_t *pat);
colorset_t *pattern_colorset_ptr(pattern_t *pat);
void pattern_set_colorset(pattern_t *pat, const colorset_t *set);
void pattern_clear_colorset(pattern_t *pat);

// comparison to other pattern
uint8_t pattern_equals(const pattern_t *pat, const pattern_t *other);

// set a color in the colorset and re-initialize
void pattern_update_color(pattern_t *pat, uint8_t index, const rgb_color_t *col);

// calculate crc of the colorset + pattern
#ifndef HELIOS_STM8
uint32_t pattern_crc32(const pattern_t *pat);
#endif

// get the pattern flags
#ifndef HELIOS_STM8
uint32_t pattern_get_flags(const pattern_t *pat);
#endif
#ifndef HELIOS_STM8
uint8_t pattern_has_flags(const pattern_t *pat, uint32_t flags);
#endif

// whether blend speed is non 0
uint8_t pattern_is_blend(const pattern_t *pat);

// whether fade speed is non 0
uint8_t pattern_is_fade(const pattern_t *pat);

#ifdef __cplusplus
}
#endif

#endif
