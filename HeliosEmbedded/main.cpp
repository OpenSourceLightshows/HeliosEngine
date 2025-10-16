#include "Helios.h"
#include "Led.h"

#ifdef HELIOS_AVR
#include <avr/sleep.h>
#endif

#if !defined(HELIOS_CLI) && !defined(HELIOS_ARDUINO)
// this is the main thread for non-arduino embedded builds
int main(int argc, char *argv[])
{
  (void)argc;  // Unused parameters
  (void)argv;

  Helios::init();
  // the main thread just initializes Helios then continuously calls tick
  while (Helios::keep_going()) {
    Helios::tick();
  }
  return 0;
}
#endif
