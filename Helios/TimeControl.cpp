// Enable POSIX features for clock_gettime, usleep, etc.
#ifdef HELIOS_CLI
#define _POSIX_C_SOURCE 200112L
#endif

#include "TimeControl.h"

#include <math.h>

#include "Timings.h"

#include "Led.h"

#ifdef HELIOS_EMBEDDED
#include <avr/sleep.h>
#include <avr/interrupt.h>
#ifdef HELIOS_ARDUINO
#include <arduino.h>
#endif
#endif

#ifdef HELIOS_CLI
#include <unistd.h>
#include <time.h>
// convert seconds and nanoseconds to microseconds
#define SEC_TO_US(sec) ((sec)*1000000)
#define NS_TO_US(ns) ((ns)/1000)
#endif

// static members
static uint32_t m_curTick = 0;
// the last frame timestamp
static uint32_t m_prevTime = 0;

#ifdef HELIOS_CLI
// whether timestep is enabled, default enabled
static uint8_t m_enableTimestep = 1;
#endif

uint8_t time_init(void)
{
  m_prevTime = time_microseconds();
  m_curTick = 0;
  return 1;
}

void time_cleanup(void)
{
}

void time_tick_clock(void)
{
  // tick clock forward
  m_curTick++;

#ifdef HELIOS_CLI
  if (!m_enableTimestep) {
    return;
  }
#endif

  /* the rest of this only runs inside vortexlib because on the duo the tick runs in the
   * tcb timer callback instead of in a busy loop constantly checking microseconds()
   * perform timestep */
  uint32_t elapsed_us;
  uint32_t us;
  do {
    us = time_microseconds();
    // detect rollover of microsecond counter
    if (us < m_prevTime) {
      // calculate wrapped around difference
      elapsed_us = (uint32_t)((UINT32_MAX - m_prevTime) + us);
    } else {
      // otherwise calculate regular difference
      elapsed_us = (uint32_t)(us - m_prevTime);
    }
    /* if building anywhere except visual studio then we can run alternate sleep code
     * because in visual studio + windows it's better to just spin and check the high
     * resolution clock instead of trying to sleep for microseconds.
     * 1000us per ms, divided by tickrate gives
     * the number of microseconds per tick */
  } while (elapsed_us < (1000000 / TICKRATE));

  // store current time
  m_prevTime = time_microseconds();
}

uint32_t time_get_current_time(void)
{
  return m_curTick;
}

#ifdef HELIOS_EMBEDDED
volatile uint32_t timer0_overflow_count = 0;
ISR(TIMER0_OVF_vect) {
  timer0_overflow_count++;  /* Increment on each overflow */
}
#endif

uint32_t time_microseconds(void)
{
#ifdef HELIOS_CLI
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  uint64_t us = SEC_TO_US((uint64_t)ts.tv_sec) + NS_TO_US((uint64_t)ts.tv_nsec);
  return (unsigned long)us;
#else
#ifdef HELIOS_ARDUINO
  return micros();
#else
  /* The only reason that micros() is actually necessary is if Helios::tick()
   * cannot be called in a 1Khz ISR. If Helios::tick() cannot be reliably called
   * by an interrupt then Time::tickClock() must perform manual timestep via micros().
   * If Helios::tick() is called by an interrupt then you don't need this function and
   * should always just rely on the current tick to perform operations */
  uint8_t oldSREG = SREG;
  cli();
  // multiply by 8 early to avoid floating point math or division
  uint32_t micros = (timer0_overflow_count * (256 * 8)) + (TCNT0 * 8);
  SREG = oldSREG;
  // then shift right to counteract the multiplication by 8
  return micros >> 6;
#endif
#endif
}

#ifdef HELIOS_EMBEDDED
__attribute__((noinline))
#endif
void
time_delay_microseconds(uint32_t us)
{
#ifdef HELIOS_EMBEDDED
#if F_CPU >= 16000000L
  // For the ATtiny85 running at 16MHz

  // The loop takes 3 cycles per iteration
  us *= 2; /* 0.5us per iteration */

  /* Subtract the overhead of the function call and loop setup
   * Assuming approximately 5 cycles overhead */
  us -= 5; /* Simplified subtraction */

  // Assembly loop for delay
  __asm__ __volatile__(
      "1: sbiw %0, 1"
      "\n\t" /* 2 cycles */
      "nop"
      "\n\t"                         /* 1 cycle */
      "brne 1b" : "=w"(us) : "0"(us) /* 2 cycles */
  );

#elif F_CPU >= 8000000L
  // For the ATtiny85 running at 8MHz

  // The loop takes 4 cycles per iteration
  us <<= 1; /* 1us per iteration */

  /* Subtract the overhead of the function call and loop setup
   * Assuming approximately 6 cycles overhead */
  us -= 6; /* Simplified subtraction */

  // Assembly loop for delay
  __asm__ __volatile__(
      "1: sbiw %0, 1"
      "\n\t" /* 2 cycles */
      "rjmp .+0"
      "\n\t"                         /* 2 cycles */
      "brne 1b" : "=w"(us) : "0"(us) /* 2 cycles */
  );
#endif

#else
  uint32_t newtime = time_microseconds() + us;
  while (time_microseconds() < newtime)
  {
    // busy loop
  }
#endif
}

void time_delay_milliseconds(uint32_t ms)
{
#ifdef HELIOS_CLI
  usleep(ms * 1000);
#else
  // not very accurate
  uint16_t i;
  for (i = 0; i < ms; ++i) {
    time_delay_microseconds(1000);
  }
#endif
}

#ifdef HELIOS_CLI
void time_enable_timestep(uint8_t enabled)
{
  m_enableTimestep = enabled;
}
#endif

