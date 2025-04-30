#!/bin/bash
parse_csv() {
  tail -n +2 "$1" | while IFS=, read -r input brief file_name
do
    echo "Creating test for $file_name"
    create_test "$input" "$brief" "$file_name"
done
}

# Function to create test file
create_test() {
    local input="$1"
    local brief="$2"
    local file_name="$3"

    # Determine the next prefix number
    local next_prefix=$(get_next_prefix)

    # Format the file name with prefix and replace spaces with underscores
    local test_file="$(printf "%04d" $next_prefix)_${file_name// /_}.test"

    # Create test file
    cd $TESTDIR

    echo "Input=${input}" > "$test_file"
    echo "Brief=${brief}" >> "$test_file"
    echo "Args="
    echo "--------------------------------------------------------------------------------" >> "$test_file"

    # Append history to the test file
    echo "$input" | $HELIOS --hex --no-timestep >> "$test_file"

    # Strip any \r in case this was run on windows
    sed -i 's/\r//g' $test_file

    # Return to previous directory
    cd -

    echo "Test file created: ${test_file}"
}

# Function to get the next available prefix number
get_next_prefix() {
    local max_prefix=0
    for file in $TESTDIR/[0-9][0-9][0-9][0-9]*.test; do
        local prefix_num="${file%%_*}"
        prefix_num="${prefix_num##*/}"
        prefix_num="${prefix_num%%.*}"
        if [[ "$prefix_num" =~ ^[0-9]+$ ]]; then
            if ((10#$prefix_num > 10#$max_prefix)); then
                max_prefix=$prefix_num
            fi
        fi
    done
    echo $((10#$max_prefix + 1))
}

# Check for CSV file argument
if [ "$#" -ne 1 ]; then
    echo "Usage: $0 <path_to_csv_file>"
    exit 1
fi

CSV_FILE="$1"

# Set the directory for test files
TESTDIR="tests"
mkdir -p $TESTDIR

# Helios command path
HELIOS="../HeliosCLI/helios"

# Check if the CSV file exists
if [ ! -f "$CSV_FILE" ]; then
    echo "CSV file not found: $CSV_FILE"
    exit 1
fi

# Parse CSV and create tests
parse_csv "$CSV_FILE"
