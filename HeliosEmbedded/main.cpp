#include "Helios.h"
#include "Led.h"

#include <avr/sleep.h>

#if !defined(HELIOS_CLI) && !defined(HELIOS_ARDUINO)
// this is the main thread for non-arduino embedded builds
int main(int argc, char *argv[])
{
  Helios helios;
  helios.init();
  // the main thread just initializes Helios then continuously calls tick
  while (helios.keep_going()) {
    helios.tick();
  }
  return 0;
}
#endif
