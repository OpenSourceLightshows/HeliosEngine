#include "Helios.h"
#include "Led.h"
#include "stm8_init.h"

#if !defined(HELIOS_CLI) && !defined(HELIOS_ARDUINO)
// Main entry point for STM8S001J3M3TR
int main(void)
{
  // Initialize STM8 hardware
  stm8_init_clock();
  stm8_init_gpio();
  stm8_init_timers();
  stm8_init_interrupts();

  // Initialize Helios engine
  helios_init();

  // Main loop - continuously call tick
  while (helios_keep_going()) {
    helios_tick();
  }

  return 0;
}
#endif
