#ifndef HELIOS_TYPES_H
#define HELIOS_TYPES_H

// Centralized forward type declarations to avoid SDCC duplicate typedef errors
// This file should be included in headers that use these types

typedef struct pattern_t pattern_t;
typedef struct pattern_args_t pattern_args_t;
typedef struct colorset_t colorset_t;
typedef struct rgb_color_t rgb_color_t;
typedef struct hsv_color_t hsv_color_t;
typedef struct random_t random_t;
typedef struct timer_t timer_t;

#endif
