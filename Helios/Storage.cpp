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

Storage::Storage()
#ifdef HELIOS_CLI
  : m_enableStorage(true),
    m_callbacks(nullptr)
#else
  : m_callbacks(nullptr)
#endif
{
}

bool Storage::init()
{
#ifdef HELIOS_CLI
  if (!m_enableStorage) {
    return true;
  }
  // if the storage filename doesn't exist then create it
  if (access(STORAGE_FILENAME, O_RDWR) != 0 && errno == ENOENT) {
    // The file doesn't exist, so try creating it
    FILE *f = fopen(STORAGE_FILENAME, "w+b");
    if (!f) {
      perror("Error creating storage file for write");
      return false;
    }
    // fill the storage with 0s
    for (uint32_t i = 0; i < STORAGE_SIZE; ++i){
      uint8_t b = 0x0;
      fwrite(&b, 1, sizeof(uint8_t), f);
    }
    fclose(f);
  }
#endif
  return true;
}

bool Storage::read_pattern(uint8_t slot, Pattern &pat)
{
  uint8_t pos = slot * SLOT_SIZE;
  if (!check_crc(pos)) {
    return false;
  }
  for (uint8_t i = 0; i < PATTERN_SIZE; ++i) {
    ((uint8_t *)&pat)[i] = read_byte(pos + i);
  }
  return true;
}

void Storage::write_pattern(uint8_t slot, const Pattern &pat)
{
  uint8_t pos = slot * SLOT_SIZE;
  for (uint8_t i = 0; i < PATTERN_SIZE; ++i) {
    uint8_t val = ((uint8_t *)&pat)[i];
    uint8_t target = pos + i;
    write_byte(target, val);
  }
  write_crc(pos);
}

void Storage::copy_slot(uint8_t srcSlot, uint8_t dstSlot)
{
  uint8_t src = srcSlot * SLOT_SIZE;
  uint8_t dst = dstSlot * SLOT_SIZE;
  for (uint8_t i = 0; i < SLOT_SIZE; ++i) {
    write_byte(dst + i, read_byte(src + i));
  }
}

uint8_t Storage::read_config(uint8_t index)
{
  return read_byte(CONFIG_START_INDEX - index);
}

void Storage::write_config(uint8_t index, uint8_t val)
{
  write_byte(CONFIG_START_INDEX - index, val);
}

uint8_t Storage::crc8(uint8_t pos, uint8_t size)
{
  uint8_t hash = 33;  // A non-zero initial value
  for (uint8_t i = 0; i < size; ++i) {
    hash = ((hash << 5) + hash) + read_byte(pos);
  }
  return hash;
}

uint8_t Storage::crc_pos(uint8_t pos)
{
  // crc the entire slot except last byte
  return crc8(pos, PATTERN_SIZE);
}

uint8_t Storage::read_crc(uint8_t pos)
{
  // read the last byte of the slot
  return read_byte(pos + PATTERN_SIZE);
}

bool Storage::check_crc(uint8_t pos)
{
  // compare the last byte to the calculated crc
  return (read_crc(pos) == crc_pos(pos));
}

void Storage::write_crc(uint8_t pos)
{
  // compare the last byte to the calculated crc
  write_byte(pos + PATTERN_SIZE, crc_pos(pos));
}

void Storage::write_byte(uint8_t address, uint8_t data)
{
  if (m_callbacks && m_callbacks->storageWrite(address, data)) {
    return;
  }
#ifdef HELIOS_EMBEDDED
  // reads out the byte of the eeprom first to see if it's different
  // before writing out the byte -- this is faster than always writing
  if (read_byte(address) == data) {
    return;
  }
  internal_write(address, data);
  // double check that shit
  if (read_byte(address) != data) {
    // do it again because eeprom is stupid
    internal_write(address, data);
    // god forbid it doesn't write again
  }
#else // HELIOS_CLI
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
    return;
  }
  fclose(f); // Close the file
#endif
}

uint8_t Storage::read_byte(uint8_t address)
{
  uint8_t callbackValue = 0;
  if (m_callbacks && m_callbacks->storageRead(address, callbackValue)) {
    return callbackValue;
  }
#ifdef HELIOS_EMBEDDED
  // do a three way read because the attiny85 eeprom basically doesn't work
  uint8_t b1 = internal_read(address);
  uint8_t b2 = internal_read(address);
  if (b1 == b2) {
    return b2;
  }
  uint8_t b3 = internal_read(address);
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
  FILE *f = fopen(STORAGE_FILENAME, "rb"); // Open file for reading in binary mode
  if (!f) {
		// this error is ok, just means no storage
    //perror("Error opening file for read");
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
  fclose(f); // Close the file
  return val;
#endif
}

#ifdef HELIOS_EMBEDDED
inline void Storage::internal_write(uint8_t address, uint8_t data)
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

inline uint8_t Storage::internal_read(uint8_t address)
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
