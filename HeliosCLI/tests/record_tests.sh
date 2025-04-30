#!/bin/bash

VALGRIND="valgrind --quiet --leak-check=full --show-leak-kinds=all"
HELIOS="../HeliosCLI/helios"

VALIDATE=0
QUIET=0
NUMFILES=0
TODO=

for arg in "$@"; do
  # -q for quiet
  if [ "$arg" == "-q" ]; then
    QUIET=1
  fi
  # -v for validate
  if [ "$arg" == "-v" ]; then
    VALIDATE=1
  fi
  # -t=test num
  if [[ $arg =~ ^-t=([0-9]*)$ ]]; then
    TODO="${BASH_REMATCH[1]}"
  fi
done

# Function to display colored text
colored() {
  printf "%s%s%s" "${!1}" "${2}" "${NC}"
}

echo -e -n "\e[33mBuilding Helios...\e[0m"
make -C ../HeliosCLI clean &> /dev/null
make -j -C ../HeliosCLI &> /dev/null
if [ $? -ne 0 ]; then
  echo -e "\e[31mFailed to build Helios!\e[0m"
  exit
fi
if [ ! -x "$HELIOS" ]; then
  echo -e "\e[31mCould not find Helios!\e[0m"
  exit
fi
echo -e "\e[32mSuccess\e[0m"

function record_tests() {
  # Directory where this script is located
  SCRIPT_DIR="$(dirname "$0")"

  # Full path to the tests directory
  PROJECT="$SCRIPT_DIR/tests"

  FILES=()

  rm -rf "$SCRIPT_DIR"/tmp/$PROJECT
  mkdir -p "$SCRIPT_DIR"/tmp/$PROJECT


  if [ "$TODO" != "" ]; then
    FILES+=($(find "$PROJECT" -name "$(printf "%04d" $TODO)*.test"))
    if [ ${#FILES[@]} -eq 0 ]; then
      echo "Could not find test $TODO"
      exit
    fi
  else
    FILES=($(find "$PROJECT" -name "*.test"))
    if [ ${#FILES[@]} -eq 0 ]; then
      echo -e "\e[31mNo tests found in $PROJECT folder\e[0m"
      exit
    fi
  fi

  NUMFILES=${#FILES[@]}
  echo -e "\e[33m== [\e[31mRECORDING \e[97m$NUMFILES INTEGRATION TESTS\e[33m] ==\e[0m"

  TESTCOUNT=0

  for FILE in "${FILES[@]}"; do
    # Call the record_test.sh using its relative path
    "$SCRIPT_DIR"/record_test.sh "$FILE" $VALIDATE $TESTCOUNT "$NUMFILES" $QUIET
    TESTCOUNT=$((TESTCOUNT + 1))
  done


  # Wait for all background jobs to finish
  wait

  if [ "$QUIET" -eq 1 ]; then
    echo ". Complete"
  fi
  echo "All tests recorded successfully!"
  #rm -rf tmp/$PROJECT
}

record_tests

