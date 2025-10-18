#ifndef PATTERNS_H
#define PATTERNS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <inttypes.h>

// Forward declaration
typedef struct pattern_t pattern_t;

//
enum pattern_id {
  //
  PATTERN_NONE = -1,

  // first pattern of all
  PATTERN_FIRST = 0,
  // =====================================

  // Strobe
  PATTERN_STROBE = PATTERN_FIRST,
  PATTERN_HYPNOSTROBE,
  PATTERN_STROBIE,
  PATTERN_RAZOR,
  PATTERN_DASH_DOPS,

  // Meta pattern constants
  INTERNAL_PATTERNS_END,
  PATTERN_LAST = (INTERNAL_PATTERNS_END - 1),
  PATTERN_COUNT = (PATTERN_LAST - PATTERN_FIRST) + 1,  //
};

void patterns_make_default(uint8_t index, pattern_t *pat);
void patterns_make_pattern(enum pattern_id id, pattern_t *pat);

#ifdef __cplusplus
}
#endif

#endif
