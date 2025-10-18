// Main entry point for Helios on 8051 architecture (CA51F152XX)
#include "Helios.h"
#include "Led.h"

#if !defined(HELIOS_CLI) && !defined(HELIOS_ARDUINO)
// Main thread for 8051 embedded builds
int main(void)
{
  // Initialize Helios engine and all components
  helios_init();

  // Main loop - continuously call tick
  while (helios_keep_going()) {
    helios_tick();
  }

  return 0;
}
#endif
