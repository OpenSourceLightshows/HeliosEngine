#ifndef TIME_CONTROL_H
#define TIME_CONTROL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <inttypes.h>

#include "HeliosConfig.h"

// macros to convert milliseconds and seconds to measures of ticks
#define MS_TO_TICKS(ms) (uint32_t)(((uint32_t)(ms) * TICKRATE) / 1000)
#define SEC_TO_TICKS(s) (uint32_t)((uint32_t)(s) * TICKRATE)

// Initialize time system
uint8_t time_init(void);
void time_cleanup(void);

// Tick the clock forward to millis()
void time_tick_clock(void);

// Get the current engine tick number (1 tick per millisecond)
uint32_t time_get_current_time(void);

// Current microseconds since startup *DO NOT USE THIS API!*
//
// If you just need to perform regular time checks for a pattern or some
// logic then use time_get_current_time() and measure time in ticks. Use the
// macros SEC_TO_TICKS() or MS_TO_TICKS() to convert timings to measures of
// ticks then compare against time_get_current_time(). The engine thinks in
// ticks, only the timestep system sees microseconds, purely to maintain a
// stable tickrate.
uint32_t time_microseconds(void);

// Delay for some number of microseconds or milliseconds, these are bad
void time_delay_microseconds(uint32_t us);
void time_delay_milliseconds(uint32_t ms);

#ifdef HELIOS_CLI
// Toggle timestep on/off
void time_enable_timestep(uint8_t enabled);
#endif

#ifdef __cplusplus
}
#endif

#endif
