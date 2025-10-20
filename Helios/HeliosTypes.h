#ifndef HELIOS_TYPES_H
#define HELIOS_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

// Forward declarations of all Helios types
// This header provides a single location for type declarations
// to avoid duplicate typedef issues with SDCC compiler

// Core pattern types
typedef struct pattern_t pattern_t;
typedef struct pattern_args_t pattern_args_t;

// Color types
typedef struct colorset_t colorset_t;
typedef struct rgb_color_t rgb_color_t;
typedef struct hsv_color_t hsv_color_t;

// Random number generator
typedef struct random_t random_t;

// Timer type
typedef struct timer_t timer_t;

#ifdef __cplusplus
}
#endif

#endif // HELIOS_TYPES_H
