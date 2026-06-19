#include <stdlib.h>

#include "Timer.h"

#include "TimeControl.h"

Timer::Timer() :
  m_alarm(0),
  m_startTime(0)
{
}

Timer::~Timer()
{
}

void Timer::init(uint8_t alarm)
{
  reset();
  m_alarm = alarm;
  start();
}

void Timer::start(uint32_t offset)
{
  // reset the start time
  m_startTime = Time::getCurtime() + offset;
}

void Timer::reset()
{
  m_alarm = 0;
  m_startTime = 0;
}

bool Timer::alarm()
{
  if (!m_alarm) {
    return false;
  }
  uint32_t now = Time::getCurtime();
  // time since start (forward or backwards)
  int32_t timeDiff = (int32_t)(int64_t)(now - m_startTime);
  if (timeDiff < 0) {
    return false;
  }
  // if no time passed it's first alarm that is starting
  if (timeDiff == 0) {
    return true;
  }
  // Recurring alarm: returns true once per m_alarm ticks.
  //
  // Small-slip branch (timeDiff in [m_alarm, 2*m_alarm)): re-anchor to now
  // so consecutive beats stay evenly spaced -- a 1-tick slip that would
  // produce a long-then-short pair instead advances the anchor to the actual
  // fire time, spreading the slip smoothly across future beats.
  // (This is the behavior Kurt confirmed "looks perfect" on hardware.)
  //
  // Big-gap branch (timeDiff >= 2*m_alarm): the timer was suspended for a
  // long menu hold or similar; realign to the period grid so post-menu
  // cadence matches the original schedule and menu-test timing stays intact.
  //
  // No 32-bit divide/modulo in the per-tick hot path (expensive on AVR).
  if (timeDiff < (int32_t)m_alarm) { return false; }
  if (timeDiff < (int32_t)(2 * m_alarm)) {
    m_startTime = now;
    return true;
  }
  int32_t rem = timeDiff;
  while (rem >= (int32_t)m_alarm) { rem -= (int32_t)m_alarm; }
  if (rem != 0) { return false; }
  m_startTime = now;
  return true;
}
