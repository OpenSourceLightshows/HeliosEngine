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
#define STORAGE_FILENAME "Helios.storage"
#endif

#ifdef HELIOS_CLI
// whether storage is enabled, default enabled
bool Storage::m_enableStorage = true;
#endif

bool Storage::init()
{
#ifdef HELIOS_CLI
  // may want this for cli tool
  //unlink(STORAGE_FILENAME);
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
    // reads out the byte of the eeprom first to see if it's different
    // before writing out the byte -- this is faster than always writing
    if (val != read_byte(target)) {
      write_byte(target, val);
    }
  }
  write_crc(pos);
}

void Storage::swap_pattern(uint8_t slot1, uint8_t slot2)
{
  uint8_t pos1 = slot1 * SLOT_SIZE;
  uint8_t pos2 = slot2 * SLOT_SIZE;
  for (uint8_t i = 0; i < SLOT_SIZE; ++i) {
    uint8_t b1 = read_byte(pos1 + i);
    uint8_t b2 = read_byte(pos2 + i);
    write_byte(pos1 + i, b2);
    write_byte(pos2 + i, b1);
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
#ifdef HELIOS_EMBEDDED
  /* Wait for completion of previous write */
  while(EECR & (1<<EEPE))
    ;
  /* Set Programming mode */
  EECR = (0<<EEPM1)|(0<<EEPM0);
  /* Set up address and data registers */
  EEAR = address;
  EEDR = data;
  /* Write logical one to EEMPE */
  EECR |= (1<<EEMPE);
  /* Start eeprom write by setting EEPE */
  EECR |= (1<<EEPE);
#else // HELIOS_CLI
  if (!m_enableStorage) {
    return;
  }
  FILE *f = fopen(STORAGE_FILENAME, "r+b");
  if (!f) {
    if (errno != ENOENT) {
      perror("Error opening storage file");
      return;
    }
    // The file doesn't exist, so try creating it
    f = fopen(STORAGE_FILENAME, "w+b");
    if (!f) {
      perror("Error creating storage file for write");
      return;
    }
  }
  // Seek to the specified address
  if (fseek(f, address, SEEK_SET) != 0) {
    perror("Error opening storage file for write");
    fclose(f);
    return;
  }
  if (!fwrite(&data, sizeof(uint8_t), 1, f)) {
    return;
  }
  fclose(f); // Close the file
#endif
}

uint8_t Storage::read_byte(uint8_t address)
{
#ifdef HELIOS_EMBEDDED
  /* Wait for completion of previous write */
  while(EECR & (1<<EEPE))
    ;
  /* Set up address register */
  EEAR = address;
  /* Start eeprom read by writing EERE */
  EECR |= (1<<EERE);
  /* Return data from data register */
  return EEDR;
#else
  if (!m_enableStorage) {
    return 0;
  }
  uint8_t val = 0;
  if (!access(STORAGE_FILENAME, O_RDONLY)) {
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
