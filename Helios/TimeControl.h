#ifndef TIME_CONTROL_H
#define TIME_CONTROL_H

#include <inttypes.h>

#include "HeliosConfig.h"

class Helios;

// macros to convert milliseconds and seconds to measures of ticks
#define MS_TO_TICKS(ms) (uint32_t)(((uint32_t)(ms) * TICKRATE) / 1000)
#define SEC_TO_TICKS(s) (uint32_t)((uint32_t)(s) * TICKRATE)

class Time
{
public:
  Time(Helios &helios);

  // initialization and cleanup of time system
  bool init();
  void cleanup();

  // tick the clock forward to millis()
  void tickClock();

  // get the current engine tick number (1 tick per millisecond)
  uint32_t getCurtime() { return m_curTick; }

  // Current microseconds since startup *DO NOT USE USING THIS API!*
  //
  // If you just need to perform regular time checks for a pattern or some
  // logic then use getCurtime() and measure time in ticks. Use the macros
  // SEC_TO_TICKS() or MS_TO_TICKS() to convert timings to measures of ticks
  // then compare against getCurtime(). The engine thinks in ticks, only the
  // timestep system sees microseconds, purely to maintain a stable tickrate.
  uint32_t microseconds();

  // delay for some number of microseconds or milliseconds, these are bad
  void delayMicroseconds(uint32_t us);
  void delayMilliseconds(uint32_t ms);

#ifdef HELIOS_CLI
  // toggle timestep on/off
  void enableTimestep(bool enabled) { m_enableTimestep = enabled; }
#endif

private:
  // reference to helios
  Helios &m_helios;
  // tick counter
  uint32_t m_curTick;
  // the last frame timestamp
  uint32_t m_prevTime;

#ifdef HELIOS_CLI
  // whether timestep is enabled
  bool m_enableTimestep;
#endif
};

#endif

