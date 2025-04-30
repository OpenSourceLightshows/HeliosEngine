#!/bin/bash

# Directory containing test files
TEST_DIR="./tests"

# Output CSV file
OUTPUT_CSV="output.csv"

# Write header to the CSV file
echo "Filename,Input,Brief" > "$OUTPUT_CSV"

# Loop through each .test file in the directory
for file in "$TEST_DIR"/*.test; do
    # Extract the lines containing 'Input=' and 'Brief='
    input_line=$(grep "Input=" "$file")
    brief_line=$(grep "Brief=" "$file")

    # Remove the 'Input=' and 'Brief=' prefixes and any commas
    input=${input_line#Input=}
    input=${input//,/}
    brief=${brief_line#Brief=}
    brief=${brief//,/}

    # Write the filename, input, and brief to the CSV file
    echo "$(basename "$file"),$input,$brief" >> "$OUTPUT_CSV"
done