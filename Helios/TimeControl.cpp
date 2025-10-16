#include "TimeControl.h"

#include <math.h>

#include "Timings.h"

#include "Led.h"

#ifdef HELIOS_EMBEDDED
  #ifdef HELIOS_8051
    #include "CA51Hardware.h"
  #elif defined(HELIOS_AVR)
    #include <avr/sleep.h>
    #include <avr/interrupt.h>
    #ifdef HELIOS_ARDUINO
      #include <arduino.h>
    #endif
  #endif
#endif

#ifdef HELIOS_CLI
#include <unistd.h>
#include <time.h>
uint64_t start = 0;
// convert seconds and nanoseconds to microseconds
#define SEC_TO_US(sec) ((sec)*1000000)
#define NS_TO_US(ns) ((ns)/1000)
#endif

// static members
uint32_t Time::m_curTick = 0;
// the last frame timestamp
uint32_t Time::m_prevTime = 0;

#ifdef HELIOS_CLI
// whether timestep is enabled, default enabled
bool Time::m_enableTimestep = true;
#endif

bool Time::init()
{
  m_prevTime = microseconds();
  m_curTick = 0;
  return true;
}

void Time::cleanup()
{
}

void Time::tickClock()
{
  // tick clock forward
  m_curTick++;

#ifdef HELIOS_CLI
  if (!m_enableTimestep) {
    return;
  }
#endif

  // the rest of this only runs inside vortexlib because on the duo the tick runs in the
  // tcb timer callback instead of in a busy loop constantly checking microseconds()
  // perform timestep
  uint32_t elapsed_us;
  uint32_t us;
  do {
    us = microseconds();
    // detect rollover of microsecond counter
    if (us < m_prevTime) {
      // calculate wrapped around difference
      elapsed_us = (uint32_t)((UINT32_MAX - m_prevTime) + us);
    } else {
      // otherwise calculate regular difference
      elapsed_us = (uint32_t)(us - m_prevTime);
    }
    // if building anywhere except visual studio then we can run alternate sleep code
    // because in visual studio + windows it's better to just spin and check the high
    // resolution clock instead of trying to sleep for microseconds.
    // 1000us per ms, divided by tickrate gives
    // the number of microseconds per tick
  } while (elapsed_us < (1000000 / TICKRATE));

  // store current time
  m_prevTime = microseconds();
}

#ifdef HELIOS_EMBEDDED
volatile uint32_t timer0_overflow_count = 0;

  #ifdef HELIOS_8051
    // 8051 Timer 0 overflow ISR for microsecond tracking
    void timer0_overflow_isr(void) ISR_ATTR(1) {
      timer0_overflow_count++;  // Increment on each overflow
      TF0 = 0;  // Clear overflow flag
    }
  #elif defined(HELIOS_AVR)
    ISR(TIMER0_OVF_vect) {
      timer0_overflow_count++;  // Increment on each overflow
    }
  #endif
#endif

uint32_t Time::microseconds()
{
#ifdef HELIOS_CLI
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  uint64_t us = SEC_TO_US((uint64_t)ts.tv_sec) + NS_TO_US((uint64_t)ts.tv_nsec);
  return (unsigned long)us;
#else
  #ifdef HELIOS_8051
    // CA51F152XX: Calculate microseconds from timer overflows
    // Assuming 12MHz system clock and Timer0 in mode 0 (13-bit counter)
    // Timer increments every 12 clock cycles
    // Each overflow = 8192 timer counts = 8192 microseconds at 12MHz/12
    uint8_t oldIE;
    SAVE_INTERRUPTS(oldIE);
    DISABLE_INTERRUPTS();

    // Read timer value
    uint8_t tl = TL0;
    uint8_t th = TH0;
    uint32_t overflows = timer0_overflow_count;

    RESTORE_INTERRUPTS(oldIE);

    // Calculate total microseconds
    // Each overflow = 8192 us (for 13-bit mode at 12MHz/12)
    uint32_t micros = (overflows * 8192UL) + ((((uint16_t)th << 5) | (tl >> 3)));
    return micros;
  #elif defined(HELIOS_ARDUINO)
    return micros();
  #elif defined(HELIOS_AVR)
    // The only reason that micros() is actually necessary is if Helios::tick()
    // cannot be called in a 1Khz ISR. If Helios::tick() cannot be reliably called
    // by an interrupt then Time::tickClock() must perform manual timestep via micros().
    // If Helios::tick() is called by an interrupt then you don't need this function and
    // should always just rely on the current tick to perform operations
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
  #ifndef HELIOS_8051
    __attribute__((noinline))
  #endif
#endif
void
Time::delayMicroseconds(uint32_t us)
{
#ifdef HELIOS_EMBEDDED
  #ifdef HELIOS_8051
    // CA51F152XX: Delay using busy loop
    // At 12MHz, each instruction cycle is 1us (12 clock cycles)
    // This is a simple busy loop - accuracy depends on compiler optimization
    uint32_t newtime = microseconds() + us;
    while (microseconds() < newtime) {
      // busy loop
    }
  #elif defined(HELIOS_AVR)
    #if F_CPU >= 16000000L
      // For the ATtiny85 running at 16MHz

      // The loop takes 3 cycles per iteration
      us *= 2; // 0.5us per iteration

      // Subtract the overhead of the function call and loop setup
      // Assuming approximately 5 cycles overhead
      us -= 5; // Simplified subtraction

      // Assembly loop for delay
      __asm__ __volatile__(
          "1: sbiw %0, 1"
          "\n\t" // 2 cycles
          "nop"
          "\n\t"                         // 1 cycle
          "brne 1b" : "=w"(us) : "0"(us) // 2 cycles
      );

    #elif F_CPU >= 8000000L
      // For the ATtiny85 running at 8MHz

      // The loop takes 4 cycles per iteration
      us <<= 1; // 1us per iteration

      // Subtract the overhead of the function call and loop setup
      // Assuming approximately 6 cycles overhead
      us -= 6; // Simplified subtraction

      // Assembly loop for delay
      __asm__ __volatile__(
          "1: sbiw %0, 1"
          "\n\t" // 2 cycles
          "rjmp .+0"
          "\n\t"                         // 2 cycles
          "brne 1b" : "=w"(us) : "0"(us) // 2 cycles
      );
    #endif
  #endif
#else
  uint32_t newtime = microseconds() + us;
  while (microseconds() < newtime)
  {
    // busy loop
  }
#endif
}

void Time::delayMilliseconds(uint32_t ms)
{
#ifdef HELIOS_CLI
  usleep(ms * 1000);
#else
  // not very accurate
  for (uint16_t i = 0; i < ms; ++i) {
    delayMicroseconds(1000);
  }
#endif
}
