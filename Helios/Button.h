#ifndef BUTTON_H
#define BUTTON_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

// Initialize button
uint8_t button_init(void);

// Directly poll the pin for whether it's pressed right now
uint8_t button_check(void);

// Poll the button pin and update the state of the button object
void button_update(void);

// Whether the button was pressed this tick
uint8_t button_on_press(void);

// Whether the button was released this tick
uint8_t button_on_release(void);

// Whether the button is currently pressed
uint8_t button_is_pressed(void);

// Whether the button was shortclicked this tick
uint8_t button_on_short_click(void);

// Whether the button was long clicked this tick
uint8_t button_on_long_click(void);

// Whether the button was hold clicked this tick
uint8_t button_on_hold_click(void);

// Detect if the button is being held past long click
uint8_t button_hold_pressing(void);

// When the button was last pressed
uint32_t button_press_time(void);

// When the button was last released
uint32_t button_release_time(void);

// How long the button is currently or was last held down (in ticks)
uint32_t button_hold_duration(void);

// How long the button is currently or was last released for (in ticks)
uint32_t button_release_duration(void);

// The number of releases
uint8_t button_release_count(void);

// Enable wake on press
void button_enable_wake(void);

#ifdef HELIOS_CLI
// These will 'inject' a short/long click without actually touching the
// button state, it's important that code uses 'button_on_short_click' or
// 'button_on_long_click' to capture this injected input event. Code that uses
// for example: 'button_hold_duration() >= threshold && button_on_release()'
// will never trigger because the injected input event doesn't actually
// press the button or change the button state it just sets the 'shortClick'
// or 'longClick' values accordingly
void button_do_short_click(void);
void button_do_long_click(void);
void button_do_hold_click(void);

// This will actually press down the button, it's your responsibility to wait
// for the appropriate number of ticks and then release the button
void button_do_press(void);
void button_do_release(void);
void button_do_toggle(void);

// Queue up an input event for the button
void button_queue_input(char input);
uint32_t button_input_queue_size(void);
#endif

#ifdef __cplusplus
}
#endif

#endif
