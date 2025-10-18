#include "Storage.h"

#include "Colorset.h"
#include "Pattern.h"

#ifdef HELIOS_EMBEDDED
#include <avr/io.h>
#endif

#ifdef HELIOS_CLI
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#endif

// Forward declarations for internal functions
static uint8_t storage_crc_pos(uint8_t pos);
static uint8_t storage_read_crc(uint8_t pos);
static uint8_t storage_check_crc(uint8_t pos);
static void storage_write_crc(uint8_t pos);
static void storage_write_byte(uint8_t address, uint8_t data);
static uint8_t storage_read_byte(uint8_t address);

#ifdef HELIOS_EMBEDDED
static inline uint8_t storage_internal_read(uint8_t address);
static inline void storage_internal_write(uint8_t address, uint8_t data);
#endif

#ifdef HELIOS_CLI
// whether storage is enabled, default enabled
static uint8_t m_enableStorage = 1;
#endif

uint8_t storage_init(void)
{
#ifdef HELIOS_CLI
  if (!m_enableStorage) {
    return 1;
  }
  // if the storage filename doesn't exist then create it
  if (access(STORAGE_FILENAME, O_RDWR) != 0 && errno == ENOENT) {
    // The file doesn't exist, so try creating it
    FILE *f = fopen(STORAGE_FILENAME, "w+b");
    if (!f) {
      perror("Error creating storage file for write");
      return 0;
    }
    // fill the storage with 0s
    uint32_t i;
    for (i = 0; i < STORAGE_SIZE; ++i){
      uint8_t b = 0x0;
      fwrite(&b, 1, sizeof(uint8_t), f);
    }
    fclose(f);
  }
#endif
  return 1;
}

uint8_t storage_read_pattern(uint8_t slot, pattern_t *pat)
{
  uint8_t pos = slot * SLOT_SIZE;
  if (!storage_check_crc(pos)) {
    return 0;
  }
  uint8_t i;
  for (i = 0; i < PATTERN_SIZE; ++i) {
    ((uint8_t *)pat)[i] = storage_read_byte(pos + i);
  }
  return 1;
}

void storage_write_pattern(uint8_t slot, const pattern_t *pat)
{
  uint8_t pos = slot * SLOT_SIZE;
  uint8_t i;
  for (i = 0; i < PATTERN_SIZE; ++i) {
    uint8_t val = ((uint8_t *)pat)[i];
    uint8_t target = pos + i;
    storage_write_byte(target, val);
  }
  storage_write_crc(pos);
}

void storage_copy_slot(uint8_t srcSlot, uint8_t dstSlot)
{
  uint8_t src = srcSlot * SLOT_SIZE;
  uint8_t dst = dstSlot * SLOT_SIZE;
  uint8_t i;
  for (i = 0; i < SLOT_SIZE; ++i) {
    storage_write_byte(dst + i, storage_read_byte(src + i));
  }
}

uint8_t storage_read_config(uint8_t index)
{
  return storage_read_byte(CONFIG_START_INDEX - index);
}

void storage_write_config(uint8_t index, uint8_t val)
{
  storage_write_byte(CONFIG_START_INDEX - index, val);
}

uint8_t storage_read_global_flags(void)
{
  return storage_read_config(STORAGE_GLOBAL_FLAG_INDEX);
}

void storage_write_global_flags(uint8_t global_flags)
{
  storage_write_config(STORAGE_GLOBAL_FLAG_INDEX, global_flags);
}

uint8_t storage_read_current_mode(void)
{
  return storage_read_config(STORAGE_CURRENT_MODE_INDEX);
}

void storage_write_current_mode(uint8_t current_mode)
{
  storage_write_config(STORAGE_CURRENT_MODE_INDEX, current_mode);
}

uint8_t storage_read_brightness(void)
{
  return storage_read_config(STORAGE_BRIGHTNESS_INDEX);
}

void storage_write_brightness(uint8_t brightness)
{
  storage_write_config(STORAGE_BRIGHTNESS_INDEX, brightness);
}

uint8_t storage_crc8(uint8_t pos, uint8_t size)
{
  uint8_t hash = 33;  // 
  uint8_t i;
  for (i = 0; i < size; ++i) {
    hash = ((hash << 5) + hash) + storage_read_byte(pos);
  }
  return hash;
}

static uint8_t storage_crc_pos(uint8_t pos)
{
  // crc the entire slot except last byte
  return storage_crc8(pos, PATTERN_SIZE);
}

static uint8_t storage_read_crc(uint8_t pos)
{
  // read the last byte of the slot
  return storage_read_byte(pos + PATTERN_SIZE);
}

static uint8_t storage_check_crc(uint8_t pos)
{
  // compare the last byte to the calculated crc
  return (storage_read_crc(pos) == storage_crc_pos(pos));
}

static void storage_write_crc(uint8_t pos)
{
  // compare the last byte to the calculated crc
  storage_write_byte(pos + PATTERN_SIZE, storage_crc_pos(pos));
}

static void storage_write_byte(uint8_t address, uint8_t data)
{
#ifdef HELIOS_EMBEDDED
  // 
  if (storage_read_byte(address) == data) {
    return;
  }
  storage_internal_write(address, data);
  // double check that shit
  if (storage_read_byte(address) != data) {
    // do it again because eeprom is stupid
    storage_internal_write(address, data);
    // god forbid it doesn't write again
  }
#else // 
  if (!m_enableStorage) {
    return;
  }
  FILE *f = fopen(STORAGE_FILENAME, "r+b");
  if (!f) {
    perror("Error opening storage file");
    return;
  }
  // Seek to the specified address
  if (fseek(f, address, SEEK_SET) != 0) {
    perror("Error opening storage file for write");
    fclose(f);
    return;
  }
  if (!fwrite((const void *)&data, sizeof(uint8_t), 1, f)) {
    fclose(f);
    return;
  }
  fclose(f); // 
#endif
}

static uint8_t storage_read_byte(uint8_t address)
{
#ifdef HELIOS_EMBEDDED
  // do a three way read because the attiny85 eeprom basically doesn't work
  uint8_t b1 = storage_internal_read(address);
  uint8_t b2 = storage_internal_read(address);
  if (b1 == b2) {
    return b2;
  }
  uint8_t b3 = storage_internal_read(address);
  if (b3 == b1) {
    return b1;
  }
  if (b3 == b2) {
    return b2;
  }
  return 0;
#else
  if (!m_enableStorage) {
    return 0;
  }
  uint8_t val = 0;
  if (access(STORAGE_FILENAME, O_RDONLY) != 0) {
    return val;
  }
  FILE *f = fopen(STORAGE_FILENAME, "rb"); // 
  if (!f) {
    // this error is ok, just means no storage
    // perror("Error opening file for read");
    return val;
  }
  // Seek to the specified address
  if (fseek(f, address, SEEK_SET) != 0) {
    // error
    perror("Failed to seek");
    fclose(f);
    return val;
  }
  // Read a byte of data
  if (!fread(&val, sizeof(uint8_t), 1, f)) {
    perror("Failed to read byte");
  }
  fclose(f); // 
  return val;
#endif
}

#ifdef HELIOS_EMBEDDED
static inline void storage_internal_write(uint8_t address, uint8_t data)
{
  while (EECR & (1<<EEPE)) {
    // Wait for completion of previous write
  }
  // Set Programming mode
  EECR = (0<<EEPM1)|(0<<EEPM0);
  // Set up address and data registers
  EEAR = address;
  EEDR = data;
  // Write logical one to EEMPE
  EECR |= (1<<EEMPE);
  // Start eeprom write by setting EEPE
  EECR |= (1<<EEPE);
}

static inline uint8_t storage_internal_read(uint8_t address)
{
  while (EECR & (1<<EEPE)) {
    // Wait for completion of previous write
  }
  // Set up address register
  EEAR = address;
  // Start eeprom read by writing EERE
  EECR |= (1<<EERE);
  // Return data from data register
  return EEDR;
}
#endif

#ifdef HELIOS_CLI
void storage_enable_storage(uint8_t enabled)
{
  m_enableStorage = enabled;
}
#endif

