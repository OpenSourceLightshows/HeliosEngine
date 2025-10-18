#ifndef HELIOS_H
#define HELIOS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "HeliosConfig.h"
#include "Colorset.h"
#include "Pattern.h"

// Forward declaration
typedef struct pattern_t pattern_t;
typedef struct colorset_t colorset_t;

uint8_t helios_init(void);
void helios_tick(void);

void helios_enter_sleep(void);
void helios_wakeup(void);

uint8_t helios_keep_going(void);
void helios_terminate(void);

void helios_load_next_mode(void);
void helios_load_cur_mode(void);
void helios_save_cur_mode(void);
void helios_load_global_flags(void);
void helios_save_global_flags(void);
void helios_set_mode_index(uint8_t mode_index);

#ifdef HELIOS_CLI
uint8_t helios_is_asleep(void);
pattern_t *helios_cur_pattern(void);
#endif

enum helios_flags {
  // No flags are set
  FLAG_NONE = 0,
  // The device is locked and must be unlocked to turn on
  FLAG_LOCKED = (1 << 0),
  // Conjure mode is enabled, one click will toggle power
  FLAG_CONJURE = (1 << 1),
  // Lock on mode is enabled, device stays on but locked
  FLAG_LOCK_ON = (1 << 2),
  // Autoplay is enabled, modes will automatically cycle
  FLAG_AUTOPLAY = (1 << 3),
  // Add new flags here, max 8 flags

  // ==============================================
  // Auto increment to count the number of flags
  INTERNAL_FLAGS_END,
  // 
  FLAGS_INVALID = (uint8_t)(~((1 << (INTERNAL_FLAGS_END - 1)) - 1))
};

// get/set global flags
void helios_set_flag(enum helios_flags flag);
uint8_t helios_has_flag(enum helios_flags flag);
uint8_t helios_has_any_flags(enum helios_flags flag);
void helios_clear_flag(enum helios_flags flag);
void helios_toggle_flag(enum helios_flags flag);

#ifdef __cplusplus
}
#endif

#endif
