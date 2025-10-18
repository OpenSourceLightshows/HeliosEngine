#ifndef LED_CONTROL_H
#define LED_CONTROL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <inttypes.h>

#include "Colortypes.h"

// opting for static functions here because there should only ever be one
// Led control object and I don't like singletons

uint8_t led_init(void);
void led_cleanup(void);

// control individual LED, these are appropriate to use in internal pattern logic
void led_set_rgb(const rgb_color_t *col);
void led_set_rgb3(uint8_t r, uint8_t g, uint8_t b);

// Turn off individual LEDs, these are appropriate to use in internal pattern logic
void led_clear(void);

// Dim individual LEDs, these are appropriate to use in internal pattern logic
void led_adjust_brightness(uint8_t fadeBy);

// strobe between two colors with a simple on/off timing
void led_strobe(uint16_t on_time, uint16_t off_time, const rgb_color_t *col1, const rgb_color_t *col2);

// breath the hue on an index
// warning: these use hsv to rgb in realtime!
void led_breath(uint8_t hue, uint32_t duration, uint8_t magnitude, uint8_t sat, uint8_t val);

// a very specialized api to hold all leds on a color for 250ms
void led_hold(const rgb_color_t *col);

// get the RGBColor of an Led index
rgb_color_t led_get(void);

// global brightness
uint8_t led_get_brightness(void);
void led_set_brightness(uint8_t brightness);

// actually update the LEDs and show the changes
void led_update(void);

#ifdef __cplusplus
}
#endif

#endif
