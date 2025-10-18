#!/bin/bash

# A simple, hardcoded script to replace a word in a project.
# WARNING: This script is destructive. BACK UP YOUR DIRECTORY FIRST.
#
# Usage: ./simple_refactor.sh <path_to_project_dir>
# Example: ./simple_refactor.sh ./usax

# --- CONFIGURE YOUR WORDS HERE ---
OLD_WORD="usax"
NEW_WORD="usax"  # <-- CHANGE THIS TO YOUR DESIRED WORD

# --- The directory to modify is the first argument to the script ---
TARGET_DIR="$1"

# Basic check to ensure the directory exists
if [ ! -d "$TARGET_DIR" ]; then
    echo "Error: Directory '$TARGET_DIR' does not exist."
    echo "Usage: ./simple_refactor.sh <path_to_directory>"
    exit 1
fi

echo "--- STARTING REFACTOR ---"
echo "Replacing '${OLD_WORD}' with '${NEW_WORD}' in directory '${TARGET_DIR}'"
echo "Press Ctrl+C within 5 seconds to cancel..."
sleep 5

# --- Phase 1: Rename all directories and files ---
# The '-depth' option processes a directory's contents before the directory itself.
echo "Phase 1: Renaming files and directories..."
find "$TARGET_DIR" -depth -name "*$OLD_WORD*" | while read path; do
    # Create the new path by replacing the old word with the new word
    new_path=$(echo "$path" | sed -e "s/$OLD_WORD/$NEW_WORD/gi")

    # If the new path is different, move the file/directory
    if [ "$path" != "$new_path" ]; then
        mv "$path" "$new_path"
    fi
done
echo "Phase 1: Complete."


# --- Phase 2: Replace the content inside all files ---
echo "Phase 2: Replacing content within all files..."
# Find all files and use sed to replace the word case-insensitively.
find "$TARGET_DIR" -type f -print0 | xargs -0 sed -i "s/$OLD_WORD/$NEW_WORD/gi"
echo "Phase 2: Complete."

echo "--- REFACTOR FINISHED ---"
