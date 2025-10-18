#include "Helios.h"
#include "Led.h"

#include <avr/sleep.h>

#if !defined(HELIOS_CLI) && !defined(HELIOS_ARDUINO)
// this is the main thread for non-arduino embedded builds
int main(int argc, char *argv[])
{
  (void)argc;
  (void)argv;

  helios_init();
  // the main thread just initializes Helios then continuously calls tick
  while (helios_keep_going()) {
    helios_tick();
  }
  return 0;
}
#endif

