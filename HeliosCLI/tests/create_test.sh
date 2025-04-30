#!/bin/bash

HELIOS="../HeliosCLI/helios"
OUTPUT_FILE="recorded_input.txt"

# Color definitions
RED="$(tput setaf 1)"
GREEN="$(tput setaf 2)"
YELLOW="$(tput setaf 3)"
WHITE="$(tput setaf 7)"
NC="$(tput sgr0)" # No Color
INTERACTIVE=0
NOMAKE=0
TODO=

for arg in "$@"
do
  if [ "$arg" == "-i" ]; then
    echo "Interactive mode not implemented yet"
    exit 1
    #INTERACTIVE=1
  fi
  if [ "$arg" == "-n" ]; then
    NOMAKE=1
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

# a helper function to insert w10 and w100 between characters in the input string
function insert_w10_w100() {
  local input_string="$1"
  local output_string=""
  local length=${#input_string}
  local last_index=$((length - 1))

  for (( i=0; i<$length; i++ )); do
      char=${input_string:$i:1}

      if [[ $char =~ [0-9] ]]; then
          output_string+="$char"
      else
          if [ $i -ne 0 ]; then
              if [ $i -eq $last_index ]; then
                  output_string+="100w"
              else
                  output_string+="300w"
              fi
          fi
          output_string+="$char"
      fi
  done

  echo "300w${output_string}300wq"
}

TESTDIR=tests

mkdir -p $TESTDIR

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

# =====================================================================
#  repeat everything below here if they say yes to making another test
while true; do

  ARGS=""

  # print a helpful help message
  $HELIOS --help

  echo -en "${YELLOW}Enter the Args:${WHITE} "
  read -e ARGS

  if [ $INTERACTIVE -eq 1 ]; then
    # Run the Helios program
    # TODO: implement --record in helios to get interactive mode
    $HELIOS $ARGS --color --in-place --record

    # Check if the output file exists and read the result from it
    if [ ! -f "$OUTPUT_FILE" ]; then
      echo "Output file not found"
      exit
    fi

    RESULT=$(cat "$OUTPUT_FILE")
    echo -n "${YELLOW}Use Result [${WHITE}$RESULT${YELLOW}]? (Y/n): ${WHITE}"

    read -e CONFIRM
    if [[ $CONFIRM == [nN] || $CONFIRM == [nN][oO] ]]; then
      exit
    fi

    NEW_INPUT=$(insert_w10_w100 "$RESULT")

    echo -e "\n${WHITE}================================================================================${NC}"
    echo -e "Processed Input: ${WHITE}$NEW_INPUT${NC}"
  else
    echo -en "${YELLOW}Enter the Input:${WHITE} "
    read -e RESULT
    NEW_INPUT=$(insert_w10_w100 "$RESULT")
    echo -n "${YELLOW}Use injected waits [${WHITE}$NEW_INPUT${YELLOW}]? (y/N): ${WHITE}"
    read -e CONFIRM
    if [[ $CONFIRM != [yY] && $CONFIRM != [yY][eE][sS] ]]; then
      NEW_INPUT="$RESULT"
      echo "Using provided input: $RESULT"
    fi
  fi

  # Prompt for test name description and args
  echo -en "${YELLOW}Enter the name of the test:${WHITE} "
  read -e TEST_NAME
  echo -en "${YELLOW}Enter the description:${WHITE} "
  read -e DESCRIPTION

  echo "========================================="
  echo "Name: $TEST_NAME"
  echo "Desc: $DESCRIPTION"
  echo "Input: $NEW_INPUT"
  echo "Args: $ARGS"

  # replace spaces with underscores
  TEST_NAME="${TEST_NAME// /_}"

  # cd to test dir
  cd $TESTDIR

  # Create the test file with an incremented prefix number
  PREFIX=0
  MAX_PREFIX=0
  for file in [0-9][0-9][0-9][0-9]*.test; do
    PREFIX_NUM="${file%%_*}"
    if [[ "$PREFIX_NUM" =~ ^[0-9]+$ ]]; then
      if ((10#$PREFIX_NUM > 10#$MAX_PREFIX)); then
        MAX_PREFIX=$PREFIX_NUM
      fi
    fi
  done
  NEXT_PREFIX=$((10#$MAX_PREFIX + 1))
  if [ "$TODO" != "" ]; then
    NEXT_PREFIX=$TODO
  fi
  TEST_FILE="$(printf "%04d" $NEXT_PREFIX)_${TEST_NAME}.test"

  # Write the test information to the test file
  echo "Input=${NEW_INPUT}" > "$TEST_FILE"
  echo "Brief=${DESCRIPTION}" >> "$TEST_FILE"
  echo "Args=${ARGS}" >> "$TEST_FILE"
  echo "--------------------------------------------------------------------------------" >> "$TEST_FILE"

  # generate the history for the test and append it to the test file
  echo "${NEW_INPUT}" | ../$HELIOS $ARGS --hex --no-timestep >> "$TEST_FILE"

  # strip any \r in case this was run on windows
  sed -i 's/\r//g' $TEST_FILE

  # cd back
  cd -

  # done
  echo "Test file created: ${TEST_FILE}"

  # add to git?
  #git add $TEST_FILE

  # run again?
  if [ $INTERACTIVE -ne 1 ]; then
    echo -n "${YELLOW}Create another test? (y/N): ${WHITE}"
    read -e CONFIRM
    if [[ $CONFIRM != [yY] && $CONFIRM != [yY][eE][sS] ]]; then
      exit
    fi
  fi

# end while true
done
