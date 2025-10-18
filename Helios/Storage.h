#ifndef STORAGE_H
#define STORAGE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <inttypes.h>
#include "HeliosConfig.h"

// the index of the first config byte, the config bytes start at the end
// then work their way backwards (so 'config index 0' is the last byte)
#define CONFIG_START_INDEX (STORAGE_SIZE - 2)
// the crc of the config bytes is the very last byte in storage
// TODO: implement the global config CRC again it got removed at some point
#define CONFIG_CRC_INDEX (STORAGE_SIZE - 1)

// Storage Config Indexes relative to the CONFIG_START_INDEX
#define STORAGE_GLOBAL_FLAG_INDEX 0
#define STORAGE_CURRENT_MODE_INDEX 1
#define STORAGE_BRIGHTNESS_INDEX 2

// Forward declaration (with include guard to prevent SDCC conflicts)
#ifndef PATTERN_T_FORWARD_DECLARED
#define PATTERN_T_FORWARD_DECLARED
typedef struct pattern_t pattern_t;
#endif

uint8_t storage_init(void);

uint8_t storage_read_pattern(uint8_t slot, pattern_t *pat);
void storage_write_pattern(uint8_t slot, const pattern_t *pat);

void storage_copy_slot(uint8_t srcSlot, uint8_t dstSlot);

uint8_t storage_read_config(uint8_t index);
void storage_write_config(uint8_t index, uint8_t val);

uint8_t storage_read_global_flags(void);
void storage_write_global_flags(uint8_t global_flags);

uint8_t storage_read_current_mode(void);
void storage_write_current_mode(uint8_t current_mode);

uint8_t storage_read_brightness(void);
void storage_write_brightness(uint8_t brightness);

uint8_t storage_crc8(uint8_t pos, uint8_t size);

#ifdef HELIOS_CLI
// toggle storage on/off
void storage_enable_storage(uint8_t enabled);
#endif

#ifdef __cplusplus
}
#endif

#endif
