#!/bin/bash

VALGRIND=
HELIOS="../HeliosCLI/helios"
DIFF="diff --strip-trailing-cr"

# select the target repo to create a test for
TARGETREPO=
VERBOSE=0
AUDIT=0
NOMAKE=0
TODO=

for arg in "$@"
do
  if [ "$arg" == "-v" ]; then
    VALGRIND=
    VERBOSE=1
  fi
  if [ "$arg" == "-n" ]; then
    NOMAKE=1
  fi
  if [ "$arg" == "-f" ]; then
    VALGRIND="valgrind --quiet --leak-check=full --show-leak-kinds=all"
  fi
  if [ "$arg" == "-a" ]; then
    AUDIT=1
    VERBOSE=1
    VALGRIND=
  fi
  if [[ $arg =~ ^-t=([0-9]*)$ ]]; then
    TODO="${BASH_REMATCH[1]}"
  fi
done

function run_tests() {
  PROJECT="tests"

  ALLSUCCES=1

  # Initialize a counter
  NUMFILES=0
  FILES=

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

  echo -e "\e[33m== [\e[97mRunning $NUMFILES Helios Integration Tests\e[33m] ==\e[0m"

  # clear tmp folder
  rm -rf tmp/$PROJECT
  mkdir -p tmp/$PROJECT

  TESTCOUNT=0

  for FILE in $FILES; do
    INPUT="$(grep "Input=" $FILE | cut -d= -f2 | tr -d '\n' | tr -d '\r')"
    BRIEF="$(grep "Brief=" $FILE | cut -d= -f2 | tr -d '\n' | tr -d '\r')"
    ARGS="$(grep "Args=" $FILE | cut -d= -f2 | tr -d '\n' | tr -d '\r')"
    TESTNUM="$(echo $FILE | cut -d/ -f2 | cut -d_ -f1 | cut -d/ -f2)"
    TESTNUM=$((10#$TESTNUM))
    TESTCOUNT=$((TESTCOUNT + 1))
    echo -e -n "\e[33mRunning test ($TESTCOUNT/$NUMFILES) [\e[97m$BRIEF\e[33m] "
    if [ "$ARGS" != "" ]; then
      echo -e -n "[\e[97m$ARGS\e[33m] "
    fi
    echo -e -n "... \e[0m"
    DIVIDER=$(grep -n -- "--------------------------------------------------------------------------------" $FILE | cut -f1 -d:)
    EXPECTED="tmp/${FILE}.expected"
    OUTPUT="tmp/${FILE}.output"
    DIFFOUT="tmp/${FILE}.diff"
    tail -n +$(($DIVIDER + 1)) "$FILE" &> $EXPECTED
    # run again?
    if [ $AUDIT -eq 1 ]; then
      echo -n "${YELLOW}Begin test? (Y/n): ${WHITE}"
      read -e CONFIRM
      if [[ $CONFIRM == [nN] || $CONFIRM == [nN][oO] ]]; then
        break
      fi
      echo ""
      echo "-----------------------------"
      echo "Input: $INPUT"
      echo "Brief: $BRIEF"
      echo "Args: $ARGS"
      echo "Test: $TESTNUM"
      echo "-----------------------------"
    fi
    # ensure there is no leftover storage file
    rm -f Helios.storage
    # now run the test
    $VALGRIND $HELIOS $ARGS --no-timestep --hex <<< $INPUT &> $OUTPUT
    # and diff the result
    $DIFF --brief $EXPECTED $OUTPUT &> $DIFFOUT
    RESULT=$?
    if [ $VERBOSE -eq 1 ]; then
      $HELIOS $ARGS --no-timestep --color <<< $INPUT
    fi
    if [ $RESULT -eq 0 ]; then
      echo -e "\e[32mSUCCESS\e[0m"
    else
      echo -e "\e[31mFAILURE\e[0m"
      ALLSUCCES=0
      if [ "$VERBOSE" -eq 1 ]; then
        break
      fi
    fi
  done

  # check if all test succeeded
  if [ $ALLSUCCES -eq 1 ]; then
    echo -e "\e[33m== [\e[32mSUCCESS ALL TESTS PASSED\e[33m] ==\e[0m"
    # if so clear the tmp folder
    rm -rf tmp/$PROJECT
  else
    if [ "$VERBOSE" -eq 1 ]; then
      # otherwise cat the last diff
      $DIFF $EXPECTED $OUTPUT
    else
      echo -e "\e[31m== FAILURE ==\e[0m"
      exit 1
    fi
  fi
}

if [ $NOMAKE -eq 0 ]; then
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
fi

# select repo and run tests with it
run_tests
