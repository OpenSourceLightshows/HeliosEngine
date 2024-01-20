#!/bin/bash

VALGRIND="valgrind --quiet --leak-check=full --show-leak-kinds=all"
HELIOS="../HeliosCLI/helios"

VALIDATE=0
QUIET=0
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

echo -e -n "\e[33mBuilding Vortex...\e[0m"
make -C ../HeliosCLI clean &> /dev/null
make -j -C ../HeliosCLI &> /dev/null
if [ $? -ne 0 ]; then
  echo -e "\e[31mFailed to build Vortex!\e[0m"
  exit
fi
if [ ! -x "$HELIOS" ]; then
  echo -e "\e[31mCould not find Vortex!\e[0m"
  exit
fi
echo -e "\e[32mSuccess\e[0m"

function record_tests() {
  PROJECT="tests"

  FILES=

  rm -rf tmp/$PROJECT
  mkdir -p tmp/$PROJECT

  if [ "$TODO" != "" ]; then
    FILES=$(find $PROJECT -name $(printf "%04d" $TODO)*.test)
    if [ "$FILES" == "" ]; then
      echo "Could not find test $TODO"
      exit
    fi
    NUMFILES=1
  else
    # Iterate through the test files
    for file in "$PROJECT"/*.test; do
      # Check if the file exists
      if [ -e "$file" ]; then
        NUMFILES=$((NUMFILES + 1))
        FILES="${FILES} $file"
      fi
    done
    if [ $NUMFILES -eq 0 ]; then
      echo -e "\e[31mNo tests found in $PROJECT folder\e[0m"
      exit
    fi
  fi

  NUMFILES="$(echo $FILES | wc -w)"

  echo -e "\e[33m== [\e[31mRECORDING \e[97m$NUMFILES INTEGRATION TESTS\e[33m] ==\e[0m"

  TESTCOUNT=0

  for FILE in $FILES; do
    ./record_test.sh $FILE $VALIDATE $TESTCOUNT $NUMFILES $QUIET &
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

