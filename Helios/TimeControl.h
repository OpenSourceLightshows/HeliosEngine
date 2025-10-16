#ifndef TIME_CONTROL_H
#define TIME_CONTROL_H

#include <inttypes.h>

#include "HeliosConfig.h"

/* macros to convert milliseconds and seconds to measures of ticks */
#define MS_TO_TICKS(ms) (uint32_t)(((uint32_t)(ms) * TICKRATE) / 1000)
#define SEC_TO_TICKS(s) (uint32_t)((uint32_t)(s) * TICKRATE)

/* Initialize time system */
uint8_t Time_init(void);
void Time_cleanup(void);

/* Tick the clock forward to millis() */
void Time_tickClock(void);

/* Get the current tick, offset by any active simulation (simulation only exists in vortexlib)
 * Exposing this as inline or macro seems to save on space a non negligible amount, it is used a lot
 * and exposing in the header probably allows the compiler to optimize away repetitive calls */
uint32_t Time_getCurtime(void);

/* Current microseconds since startup, only use this for things like measuring rapid data transfer timings.
 * If you just need to perform regular time checks for a pattern or some logic then use Time_getCurtime() and measure
 * time in ticks, use the SEC_TO_TICKS() or MS_TO_TICKS() macros to convert timings to measures of ticks for
 * purpose of comparing against Time_getCurtime() */
uint32_t Time_microseconds(void);

/* Delay for some number of microseconds or milliseconds, these are bad */
void Time_delayMicroseconds(uint32_t us);
void Time_delayMilliseconds(uint32_t ms);

#ifdef HELIOS_CLI
/* Toggle timestep on/off */
void Time_enableTimestep(uint8_t enabled);
#endif

#endif
