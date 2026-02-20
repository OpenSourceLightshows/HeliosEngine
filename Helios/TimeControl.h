#ifndef TIME_CONTROL_H
#define TIME_CONTROL_H

#include <inttypes.h>

#include "HeliosConfig.h"

// macros to convert milliseconds and seconds to measures of ticks
#define MS_TO_TICKS(ms) (uint32_t)(((uint32_t)(ms) * TICKRATE) / 1000)
#define SEC_TO_TICKS(s) (uint32_t)((uint32_t)(s) * TICKRATE)

class Time
{
  // private unimplemented constructor
  Time();

public:
  // opting for static class here because there should only ever be one
  // Settings control object and I don't like singletons
  static bool init();
  static void cleanup();

  // tick the clock forward to millis()
  static void tickClock();

  // get the current tick, offset by any active simulation (simulation only exists in vortexlib)
  // Exposing this in the header seems to save on space a non negligible amount, it is used a lot
  // and exposing in the header probably allows the compiler to optimize away repititive calls
  static uint32_t getCurtime() { return m_curTick; }

  // Current microseconds since startup, only use this for things like measuring rapid data transfer timings.
  // If you just need to perform regular time checks for a pattern or some logic then use getCurtime() and measure
  // time in ticks, use the SEC_TO_TICKS() or MS_TO_TICKS() macros to convert timings to measures of ticks for
  // purpose of comparing against getCurtime()
  static uint32_t microseconds();

  // delay for some number of microseconds or milliseconds, these are bad
  static void delayMicroseconds(uint32_t us);
  static void delayMilliseconds(uint32_t ms);

#ifdef HELIOS_CLI
  // toggle timestep on/off
  static void enableTimestep(bool enabled) { m_enableTimestep = enabled; }
#endif

private:
  // global tick counter
  static uint32_t m_curTick;
  // the last frame timestamp
  static uint32_t m_prevTime;

#ifdef HELIOS_CLI
  // whether timestep is enabled
  static bool m_enableTimestep;
#endif
};

#endif

