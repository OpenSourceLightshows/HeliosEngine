#ifndef STORAGE_H
#define STORAGE_H

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

class Pattern;

class Storage
{


  static bool init();

  static bool read_pattern(uint8_t slot, Pattern &pat);
  static void write_pattern(uint8_t slot, const Pattern &pat);

  static void copy_slot(uint8_t srcSlot, uint8_t dstSlot);

  static uint8_t read_config(uint8_t index);
  static void write_config(uint8_t index, uint8_t val);

  static uint8_t read_global_flags() { return read_config(STORAGE_GLOBAL_FLAG_INDEX); }
  static void write_global_flags(uint8_t global_flags) { write_config(STORAGE_GLOBAL_FLAG_INDEX, global_flags); }

  static uint8_t read_current_mode() { return read_config(STORAGE_CURRENT_MODE_INDEX); }
  static void write_current_mode(uint8_t current_mode) { write_config(STORAGE_CURRENT_MODE_INDEX, current_mode); }

  static uint8_t read_brightness() { return read_config(STORAGE_BRIGHTNESS_INDEX); }
  static void write_brightness(uint8_t brightness) { write_config(STORAGE_BRIGHTNESS_INDEX, brightness); }

  static uint8_t crc8(uint8_t pos, uint8_t size);

#ifdef HELIOS_CLI
  // toggle storage on/off
  static void enableStorage(bool enabled) { m_enableStorage = enabled; }
#endif

  static uint8_t crc_pos(uint8_t pos);
  static uint8_t read_crc(uint8_t pos);
  static bool check_crc(uint8_t pos);
  static void write_crc(uint8_t pos);

  static void write_byte(uint8_t address, uint8_t data);
  static uint8_t read_byte(uint8_t address);

#ifdef HELIOS_EMBEDDED
  static inline uint8_t internal_read(uint8_t address);
  static inline void internal_write(uint8_t address, uint8_t data);
#endif

#ifdef HELIOS_CLI
  // whether storage is enabled
  static bool m_enableStorage;
#endif
};

#endif
