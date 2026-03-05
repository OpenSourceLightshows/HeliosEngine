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

  bool init();
  void cleanup();

  // tick the clock forward to millis()
  void tickClock();

  // get the current tick, offset by any active simulation (simulation only exists in vortexlib)
  uint32_t getCurtime() const { return m_curTick; }

  // Current microseconds since startup, only use this for things like measuring rapid data transfer timings.
  // If you just need to perform regular time checks for a pattern or some logic then use getCurtime() and measure
  // time in ticks, use the SEC_TO_TICKS() or MS_TO_TICKS() macros to convert timings to measures of ticks for
  // purpose of comparing against getCurtime()
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

