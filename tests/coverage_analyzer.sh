#!/bin/bash

# Coverage Analyzer
# This script helps identify which tests cover specific lines of code

# Colors for terminal output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
WHITE='\033[0;97m'
NC='\033[0m' # No Color

# Get the directory of the script
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
CLI_DIR="${SCRIPT_DIR}/../HeliosCLI"
COV_DIR="${SCRIPT_DIR}/coverage"
TEMP_DIR="${SCRIPT_DIR}/tmp/coverage_temp"

# Define source file paths
SOURCE_FILE=""
LINE_NUMBER=""
LIST_UNCOVERED=0
ALL_FILES=0

# Parse command line arguments
while [[ $# -gt 0 ]]; do
  case $1 in
    -f|--file)
      SOURCE_FILE="$2"
      shift 2
      ;;
    -l|--line)
      LINE_NUMBER="$2"
      shift 2
      ;;
    -u|--uncovered)
      LIST_UNCOVERED=1
      shift
      ;;
    -a|--all-files)
      ALL_FILES=1
      shift
      ;;
    -h|--help)
      echo "Usage: $0 [options]"
      echo "Options:"
      echo "  -f, --file FILE       Source file to analyze"
      echo "  -l, --line NUMBER     Line number to check for coverage"
      echo "  -u, --uncovered       List all uncovered lines in the file"
      echo "  -a, --all-files       List all files with coverage information"
      echo "  -h, --help            Show this help message"
      exit 0
      ;;
    *)
      echo "Unknown option: $1"
      exit 1
      ;;
  esac
done

# Check if LCOV is installed
if ! command -v lcov &> /dev/null; then
    echo -e "${RED}Error: lcov is not installed. Please install it first.${NC}"
    echo "  macOS: brew install lcov"
    echo "  Ubuntu/Debian: apt-get install lcov"
    exit 1
fi

# Create temporary directory
mkdir -p "${TEMP_DIR}"

# Function to find tests that cover a specific line
function find_tests_for_line() {
  local source_file=$1
  local line_number=$2
  local found_tests=()

  # Print header
  echo -e "${YELLOW}== [${WHITE}Tests covering ${source_file}:${line_number}${YELLOW}] ==${NC}"

  # Iterate through each test file
  for test_file in $(find "${SCRIPT_DIR}/tests" -name "*.test" | sort); do
    test_number=$(basename "$test_file" | cut -d_ -f1)
    test_name=$(basename "$test_file" .test | cut -d_ -f2- | tr '_' ' ')

    # Run the specific test with coverage
    echo -e "${YELLOW}Testing: ${WHITE}${test_name} (${test_number})${NC}"

    # Clean previous coverage data for this test
    find "${CLI_DIR}" -name "*.gcda" -delete
    find "${SCRIPT_DIR}/../Helios" -name "*.gcda" -delete

    # Run just this test
    "${SCRIPT_DIR}/run_tests_with_coverage.sh" -n -t=${test_number} > /dev/null 2>&1

    # Check if the specific line was covered
    lcov --extract "${COV_DIR}/coverage.info" "*${source_file}" --output-file "${TEMP_DIR}/single_test.info" --quiet

    # Check if the line is covered
    if grep -q "DA:${line_number}," "${TEMP_DIR}/single_test.info"; then
      coverage_count=$(grep "DA:${line_number}," "${TEMP_DIR}/single_test.info" | cut -d, -f2)
      found_tests+=("${test_number} - ${test_name} (executed ${coverage_count} times)")
    fi
  done

  # Print results
  if [ ${#found_tests[@]} -eq 0 ]; then
    echo -e "${RED}No tests cover this line${NC}"
  else
    echo -e "${GREEN}Found ${#found_tests[@]} tests that cover this line:${NC}"
    for test in "${found_tests[@]}"; do
      echo -e "  ${GREEN}✓${NC} ${test}"
    done
  fi
}

# Function to list all uncovered lines in a file
function list_uncovered_lines() {
  local source_file=$1

  # Check if coverage data exists
  if [ ! -f "${COV_DIR}/coverage.info" ]; then
    echo -e "${RED}No coverage data found. Run run_tests_with_coverage.sh first.${NC}"
    exit 1
  fi

  # Extract coverage info for this file
  lcov --extract "${COV_DIR}/coverage.info" "*${source_file}" --output-file "${TEMP_DIR}/file_coverage.info" --quiet

  # Check if file exists in coverage data
  if [ ! -s "${TEMP_DIR}/file_coverage.info" ]; then
    echo -e "${RED}No coverage information for ${source_file}${NC}"
    exit 1
  fi

  # Find the actual file path
  actual_file_path=$(grep -m 1 "SF:" "${TEMP_DIR}/file_coverage.info" | cut -d: -f2)

  # Get all lines from the file
  total_lines=$(wc -l < "$actual_file_path")

  # Get covered lines
  covered_lines=$(grep "DA:" "${TEMP_DIR}/file_coverage.info" | cut -d: -f2 | cut -d, -f1)

  # Print header
  echo -e "${YELLOW}== [${WHITE}Uncovered lines in ${source_file}${YELLOW}] ==${NC}"

  # Calculate coverage percentage
  covered_count=$(echo "$covered_lines" | wc -l)
  coverage_percent=$(echo "scale=2; ($covered_count/$total_lines)*100" | bc)

  echo -e "File: ${WHITE}${actual_file_path}${NC}"
  echo -e "Total lines: ${WHITE}${total_lines}${NC}"
  echo -e "Covered lines: ${GREEN}${covered_count}${NC}"
  echo -e "Coverage: ${GREEN}${coverage_percent}%${NC}"
  echo ""

  # Create a temporary file with all line numbers
  seq 1 $total_lines > "${TEMP_DIR}/all_lines.txt"
  echo "$covered_lines" | sort -n > "${TEMP_DIR}/covered_lines.txt"

  # Find uncovered lines
  uncovered_lines=$(comm -23 "${TEMP_DIR}/all_lines.txt" "${TEMP_DIR}/covered_lines.txt")

  # Output uncovered lines
  echo -e "${YELLOW}Uncovered lines:${NC}"

  # Group consecutive uncovered lines
  prev_line=0
  range_start=0

  for line in $uncovered_lines; do
    # Check if this is a continuation of a range
    if [ $prev_line -eq $((line - 1)) ] && [ $range_start -ne 0 ]; then
      prev_line=$line
    else
      # Print the previous range if it exists
      if [ $range_start -ne 0 ]; then
        if [ $range_start -eq $prev_line ]; then
          echo -e "  ${RED}Line ${range_start}${NC}"
        else
          echo -e "  ${RED}Lines ${range_start}-${prev_line}${NC}"
        fi
      fi
      # Start a new range
      range_start=$line
      prev_line=$line
    fi
  done

  # Print the last range
  if [ $range_start -ne 0 ]; then
    if [ $range_start -eq $prev_line ]; then
      echo -e "  ${RED}Line ${range_start}${NC}"
    else
      echo -e "  ${RED}Lines ${range_start}-${prev_line}${NC}"
    fi
  fi
}

# Function to list all files with coverage information
function list_all_files() {
  # Check if coverage data exists
  if [ ! -f "${COV_DIR}/coverage.info" ]; then
    echo -e "${RED}No coverage data found. Run run_tests_with_coverage.sh first.${NC}"
    exit 1
  fi

  # Print header
  echo -e "${YELLOW}== [${WHITE}Files with coverage information${YELLOW}] ==${NC}"

  # Extract list of files and their coverage
  lcov --list "${COV_DIR}/coverage.info" > "${TEMP_DIR}/coverage_list.txt"

  # Format and display the results
  sed -n '/^File/,$p' "${TEMP_DIR}/coverage_list.txt" | tail -n +2 | sed '/^Total/d' | while read -r line; do
    file_path=$(echo "$line" | awk '{print $1}')
    line_coverage=$(echo "$line" | awk '{print $2}')

    # Color the coverage percentage based on value
    coverage_num=$(echo "$line_coverage" | tr -d '%')
    if (( $(echo "$coverage_num < 50" | bc -l) )); then
      color="${RED}"
    elif (( $(echo "$coverage_num < 80" | bc -l) )); then
      color="${YELLOW}"
    else
      color="${GREEN}"
    fi

    echo -e "${WHITE}${file_path}${NC}: ${color}${line_coverage}${NC} line coverage"
  done
}

# Main execution logic
if [ $ALL_FILES -eq 1 ]; then
  # List all files with coverage information
  list_all_files
elif [ -n "$SOURCE_FILE" ] && [ $LIST_UNCOVERED -eq 1 ]; then
  # List uncovered lines in the specified file
  list_uncovered_lines "$SOURCE_FILE"
elif [ -n "$SOURCE_FILE" ] && [ -n "$LINE_NUMBER" ]; then
  # Find tests that cover a specific line
  find_tests_for_line "$SOURCE_FILE" "$LINE_NUMBER"
else
  echo -e "${RED}Error: Invalid options${NC}"
  echo "Run '$0 --help' for usage information"
  exit 1
fi

# Clean up
rm -rf "${TEMP_DIR}"