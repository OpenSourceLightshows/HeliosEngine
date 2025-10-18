#ifndef COLOR_H
#define COLOR_H

#include <inttypes.h>

#include "HeliosConfig.h"
#include "ColorConstants.h"

#ifdef __cplusplus
extern "C" {
#endif

#if ALTERNATIVE_HSV_RGB == 1
enum hsv_to_rgb_algorithm
{
  HSV_TO_RGB_GENERIC,
  HSV_TO_RGB_RAINBOW
};

/* global hsv to rgb algorithm selector, switch this to control
 * all hsv to rgb conversions */
extern enum hsv_to_rgb_algorithm g_hsv_rgb_alg;
#endif

// Forward declarations
typedef struct hsv_color_t hsv_color_t;
typedef struct rgb_color_t rgb_color_t;

struct hsv_color_t
{
  uint8_t hue;
  uint8_t sat;
  uint8_t val;
};

struct rgb_color_t
{
  uint8_t red;
  uint8_t green;
  uint8_t blue;
};

// HSVColor functions
void hsv_init(hsv_color_t *hsv);
void hsv_init3(hsv_color_t *hsv, uint8_t hue, uint8_t sat, uint8_t val);
void hsv_init_from_raw(hsv_color_t *hsv, uint32_t dwVal);
void hsv_init_from_rgb(hsv_color_t *hsv, const rgb_color_t *rgb);
void hsv_copy(hsv_color_t *dest, const hsv_color_t *src);
void hsv_assign_from_raw(hsv_color_t *hsv, uint32_t rhs);
void hsv_assign_from_rgb(hsv_color_t *hsv, const rgb_color_t *rhs);
uint8_t hsv_equals(const hsv_color_t *a, const hsv_color_t *b);
uint8_t hsv_empty(const hsv_color_t *hsv);
void hsv_clear(hsv_color_t *hsv);
uint32_t hsv_raw(const hsv_color_t *hsv);

// RGBColor functions
void rgb_init(rgb_color_t *rgb);
void rgb_init3(rgb_color_t *rgb, uint8_t red, uint8_t green, uint8_t blue);
void rgb_init_from_raw(rgb_color_t *rgb, uint32_t dwVal);
void rgb_init_from_hsv(rgb_color_t *rgb, const hsv_color_t *hsv);
void rgb_copy(rgb_color_t *dest, const rgb_color_t *src);
void rgb_assign_from_raw(rgb_color_t *rgb, uint32_t rhs);
void rgb_assign_from_hsv(rgb_color_t *rgb, const hsv_color_t *rhs);
uint8_t rgb_equals(const rgb_color_t *a, const rgb_color_t *b);
uint8_t rgb_empty(const rgb_color_t *rgb);
void rgb_clear(rgb_color_t *rgb);
void rgb_adjust_brightness(rgb_color_t *rgb, uint8_t fadeBy);
uint32_t rgb_raw(const rgb_color_t *rgb);

#ifdef HELIOS_CLI
/* Return a scaled brightness version of the current color
 * ex: 0.0 = black, 0.5 = half brightness, 1.0 = no change,
 *     1.5 = 50% brighter, 2.0 = twice as bright, 255.0 = white */
void rgb_scale_brightness(rgb_color_t *rgb, float scale);
// Bring up the brightness of a color to a minimum level
void rgb_bring_up_brightness(rgb_color_t *rgb, uint8_t min_brightness);
#endif

// Conversion functions
/* Stolen from FastLED hsv to rgb full rainbow where all colours
 * are given equal weight, this makes for-example yellow larger
 * best to use this function as it is the legacy choice */
rgb_color_t hsv_to_rgb_rainbow(const hsv_color_t *rhs);
// Generic hsv to rgb conversion nothing special
rgb_color_t hsv_to_rgb_generic(const hsv_color_t *rhs);
// Convert rgb to hsv with generic fast method
hsv_color_t rgb_to_hsv_generic(const rgb_color_t *rhs);

#ifdef __cplusplus
}
#endif

#endif
