#!/bin/bash

MAP_FILE=$1

if [ "$MAP_FILE" == "" ]; then
  echo "Please specify a .map file: $0 <file.map>"
  exit 1
fi

if [ ! -f "$MAP_FILE" ]; then
  echo "File not found: $MAP_FILE"
  exit 1
fi

# STM8S001J3M3TR memory limits
FLASH_SIZE=8192
RAM_SIZE=1024
EEPROM_SIZE=128

# Parse a hex size from the SDCC linker map symbol table (l_<SECTION> = length)
get_size() {
  local sym=$1
  local val
  val=$(grep -m1 "[[:space:]]${sym}[[:space:]]" "$MAP_FILE" | awk '{print $1}')
  if [ -z "$val" ]; then
    echo "0"
  else
    printf "%d\n" "0x${val}"
  fi
}

# ROM sections (all live in flash)
HOME_SIZE=$(get_size l_HOME)
GSINIT_SIZE=$(get_size l_GSINIT)
GSFINAL_SIZE=$(get_size l_GSFINAL)
CONST_SIZE=$(get_size l_CONST)
INITIALIZER_SIZE=$(get_size l_INITIALIZER)
CODE_SIZE=$(get_size l_CODE)

# RAM sections
DATA_SIZE=$(get_size l_DATA)
INITIALIZED_SIZE=$(get_size l_INITIALIZED)

FLASH_USED=$((HOME_SIZE + GSINIT_SIZE + GSFINAL_SIZE + CONST_SIZE + INITIALIZER_SIZE + CODE_SIZE))
RAM_USED=$((DATA_SIZE + INITIALIZED_SIZE))

FLASH_PERCENT=$(awk -v used="$FLASH_USED" -v total="$FLASH_SIZE" 'BEGIN { printf("%.2f", used / total * 100) }')
RAM_PERCENT=$(awk  -v used="$RAM_USED"   -v total="$RAM_SIZE"   'BEGIN { printf("%.2f", used / total * 100) }')

echo "Flash uses $FLASH_USED bytes ($FLASH_PERCENT%) of program storage. Maximum is $FLASH_SIZE bytes. (Hex: $(printf '%x' $FLASH_USED)/$(printf '%x' $FLASH_SIZE))"
echo "RAM uses $RAM_USED bytes ($RAM_PERCENT%) of data memory, leaving $(($RAM_SIZE - $RAM_USED)) bytes available. Maximum is $RAM_SIZE bytes. (Hex: $(printf '%x' $RAM_USED)/$(printf '%x' $RAM_SIZE))"
