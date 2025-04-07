#!/bin/bash

# Directory containing the test files
TEST_DIR="tests/tests"

# Check if dry run mode is enabled
DRY_RUN=false
if [ "$1" == "--dry-run" ]; then
  DRY_RUN=true
  echo "Running in dry-run mode. No files will be changed."
fi

# Check if the directory exists
if [ ! -d "$TEST_DIR" ]; then
  echo "Error: Directory $TEST_DIR does not exist"
  exit 1
fi

# Get all test files and just their descriptive parts for unique naming
cd "$TEST_DIR"
files=($(ls *.test | sort))

echo "Found ${#files[@]} test files. Renumbering..."

# Extract unique descriptive parts and assign new sequential numbers
unique_descs=($(for f in "${files[@]}"; do echo "$f" | cut -d'_' -f2-; done | sort -u))
echo "Found ${#unique_descs[@]} unique test types."

# Create a temporary directory for the renaming operations
if [ "$DRY_RUN" = false ]; then
  TEMP_DIR=$(mktemp -d)
fi

# Process file renames
counter=1
for desc in "${unique_descs[@]}"; do
  # Find all files matching this descriptive part
  matches=($(ls [0-9]*_"$desc" 2>/dev/null))

  # Only process the first file that matches each description
  # (the cleanup should have already removed duplicates)
  if [ ${#matches[@]} -gt 0 ]; then
    file="${matches[0]}"

    # Create new sequential number with padding
    new_num=$(printf "%04d" $counter)
    new_filename="${new_num}_${desc}"

    # Skip if the filename wouldn't change
    if [ "$file" = "$new_filename" ]; then
      echo "No change needed for: $file"
    else
      if [ "$DRY_RUN" = true ]; then
        echo "Would rename: $file -> $new_filename"
      else
        # Move to temp dir to avoid conflicts
        mv "$file" "$TEMP_DIR/$new_filename"
        echo "Renamed: $file -> $new_filename"
      fi
    fi

    # Increment counter for next file
    ((counter++))
  fi
done

if [ "$DRY_RUN" = false ]; then
  # Move all renamed files back to the original directory
  mv "$TEMP_DIR"/* . 2>/dev/null

  # Clean up the temporary directory
  rmdir "$TEMP_DIR"
  echo "Renumbering complete!"
else
  echo "Dry run complete. Run without --dry-run to apply changes."
fi