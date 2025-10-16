#ifndef COLOR_C_H
#define COLOR_C_H

#include <inttypes.h>

#include "HeliosConfig.h"
#include "ColorConstants.h"

#ifdef __cplusplus
extern "C" {
#endif

#if ALTERNATIVE_HSV_RGB == 1
typedef enum {
  HSV_TO_RGB_GENERIC = 0,
  HSV_TO_RGB_RAINBOW = 1
} hsv_to_rgb_algorithm;

// global hsv to rgb algorithm selector
extern hsv_to_rgb_algorithm g_hsv_rgb_alg;
#endif

// ============================================================================
// HSV Color Type
// ============================================================================

typedef struct {
  uint8_t hue;
  uint8_t sat;
  uint8_t val;
} HSVColor;

// HSV Color functions
static inline void HSVColor_init(HSVColor *hsv) {
  hsv->hue = 0;
  hsv->sat = 0;
  hsv->val = 0;
}

static inline void HSVColor_set(HSVColor *hsv, uint8_t h, uint8_t s, uint8_t v) {
  hsv->hue = h;
  hsv->sat = s;
  hsv->val = v;
}

static inline void HSVColor_fromU32(HSVColor *hsv, uint32_t dwVal) {
  hsv->hue = ((dwVal >> 16) & 0xFF);
  hsv->sat = ((dwVal >> 8) & 0xFF);
  hsv->val = (dwVal & 0xFF);
}

static inline uint32_t HSVColor_raw(const HSVColor *hsv) {
  return ((uint32_t)hsv->hue << 16) | ((uint32_t)hsv->sat << 8) | (uint32_t)hsv->val;
}

static inline int HSVColor_equals(const HSVColor *a, const HSVColor *b) {
  return HSVColor_raw(a) == HSVColor_raw(b);
}

static inline int HSVColor_empty(const HSVColor *hsv) {
  return !hsv->hue && !hsv->sat && !hsv->val;
}

static inline void HSVColor_clear(HSVColor *hsv) {
  hsv->hue = 0;
  hsv->sat = 0;
  hsv->val = 0;
}

// ============================================================================
// RGB Color Type
// ============================================================================

typedef struct {
  uint8_t red;
  uint8_t green;
  uint8_t blue;
} RGBColor;

// RGB Color functions
static inline void RGBColor_init(RGBColor *rgb) {
  rgb->red = 0;
  rgb->green = 0;
  rgb->blue = 0;
}

static inline void RGBColor_set(RGBColor *rgb, uint8_t r, uint8_t g, uint8_t b) {
  rgb->red = r;
  rgb->green = g;
  rgb->blue = b;
}

static inline void RGBColor_fromU32(RGBColor *rgb, uint32_t dwVal) {
  rgb->red = ((dwVal >> 16) & 0xFF);
  rgb->green = ((dwVal >> 8) & 0xFF);
  rgb->blue = (dwVal & 0xFF);
}

static inline uint32_t RGBColor_raw(const RGBColor *rgb) {
  return ((uint32_t)rgb->red << 16) | ((uint32_t)rgb->green << 8) | (uint32_t)rgb->blue;
}

static inline int RGBColor_equals(const RGBColor *a, const RGBColor *b) {
  return RGBColor_raw(a) == RGBColor_raw(b);
}

static inline int RGBColor_empty(const RGBColor *rgb) {
  return !rgb->red && !rgb->green && !rgb->blue;
}

static inline void RGBColor_clear(RGBColor *rgb) {
  rgb->red = 0;
  rgb->green = 0;
  rgb->blue = 0;
}

// Scale down the brightness of a color
static inline void RGBColor_adjustBrightness(RGBColor *rgb, uint8_t fadeBy) {
  rgb->red = (((int)rgb->red) * (int)(256 - fadeBy)) >> 8;
  rgb->green = (((int)rgb->green) * (int)(256 - fadeBy)) >> 8;
  rgb->blue = (((int)rgb->blue) * (int)(256 - fadeBy)) >> 8;
}

#ifdef HELIOS_CLI
// CLI-only functions with float support
void RGBColor_scaleBrightness(RGBColor *rgb, float scale);
void RGBColor_bringUpBrightness(RGBColor *rgb, uint8_t min_brightness);
#endif

// ============================================================================
// Color Conversion Functions
// ============================================================================

// Convert HSV to RGB
void HSVColor_toRGB(const HSVColor *hsv, RGBColor *rgb);
void RGBColor_toHSV(const RGBColor *rgb, HSVColor *hsv);

// Conversion functions
void hsv_to_rgb_rainbow(const HSVColor *hsv, RGBColor *rgb);
void hsv_to_rgb_generic(const HSVColor *hsv, RGBColor *rgb);
void rgb_to_hsv_generic(const RGBColor *rgb, HSVColor *hsv);

#ifdef __cplusplus
}
#endif

#endif // COLOR_C_H

