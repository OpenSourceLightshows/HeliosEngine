#ifndef PATTERNS_H
#define PATTERNS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <inttypes.h>
#include "HeliosTypes.h"

// Forward declaration
// typedef struct pattern_t pattern_t;

// 
enum pattern_id {
  // 
  PATTERN_NONE = -1,

  // first pattern of all
  PATTERN_FIRST = 0,
  // =====================================

  // Strobe
  PATTERN_DOPS = PATTERN_FIRST,
  PATTERN_STROBE,
  PATTERN_HYPNOSTROBE,
  PATTERN_STROBIE,
  PATTERN_FLARE,

  // Morph
  PATTERN_MORPH_STROBIE,

  // Dash
  PATTERN_DASH_DOPS,

  // Fade
  PATTERN_FADE,
  PATTERN_MORPH_FADE,
  PATTERN_GLITCH_FADE,

  // Meta pattern constants
  INTERNAL_PATTERNS_END,
  PATTERN_LAST = (INTERNAL_PATTERNS_END - 1),
  PATTERN_COUNT = (PATTERN_LAST - PATTERN_FIRST) + 1  // 
};

// Pattern creation functions
void patterns_make_default(uint8_t index, pattern_t *pat);
void patterns_make_pattern(enum pattern_id id, pattern_t *pat);

#ifdef __cplusplus
}
#endif

#endif
