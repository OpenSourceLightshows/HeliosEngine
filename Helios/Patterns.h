#ifndef PATTERNS_H
#define PATTERNS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <inttypes.h>

// Forward declaration (avoid duplicate if Pattern.h is included)
#ifndef PATTERN_T_FORWARD_DECLARED
#define PATTERN_T_FORWARD_DECLARED
typedef struct pattern_t pattern_t;
#endif

// List of patterns that can be built, both single and multi-led patterns are found in this list.
// Within both single and multi LED pattern lists there are 'core' patterns which are associated
// with a class, and there are 'shell' patterns which are simply wrappers around another pattern
// with different parameters passed to the constructor.  There is no way to know which patterns
// are 'core' patterns, except by looking at PatternBuilder::generate to see which classes exist
enum pattern_id {
  // no pattern at all, use this sparingly and default to
  // PATTERN_FIRST when possible
  PATTERN_NONE = -1,

  // first pattern of all
  PATTERN_FIRST = 0,
  // =====================================

  // Strobe
  PATTERN_RIBBON = PATTERN_FIRST,
  PATTERN_ULTRA_DOPS,
  PATTERN_DOPS,
  PATTERN_STROBE,
  PATTERN_HYPNOSTROBE,
  PATTERN_STROBIE,
  PATTERN_RAZOR,
  PATTERN_FLARE,
  PATTERN_BURST,
  PATTERN_GLOW,
  PATTERN_FLICKER,
  PATTERN_FLASH,
  // Morph
  PATTERN_MORPH,
  PATTERN_MORPH_STROBE,
  PATTERN_MORPH_STROBIE,
  PATTERN_MORPH_GLOW,
  // Dash
  PATTERN_DASH_DOPS,
  PATTERN_DASH_DOT,
  PATTERN_WAVE_PARTICLE,
  PATTERN_LIGHTSPEED,

  // Meta pattern constants
  INTERNAL_PATTERNS_END,
  PATTERN_LAST = (INTERNAL_PATTERNS_END - 1),
  PATTERN_COUNT = (PATTERN_LAST - PATTERN_FIRST) + 1  // total number of patterns
};

// Pattern creation functions
void patterns_make_default(uint8_t index, pattern_t *pat);
void patterns_make_pattern(enum pattern_id id, pattern_t *pat);

#ifdef __cplusplus
}
#endif

#endif
