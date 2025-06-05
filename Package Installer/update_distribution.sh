#!/bin/bash

# Calculate total size in KB of package-root, scripts, and Resources
PKG_SIZE=$(du -sk package-root/ scripts/ resources/ 2>/dev/null | awk '{total += $1} END {print total}')

# Check if PKG_SIZE was successfully calculated
if [ -z "$PKG_SIZE" ]; then
    echo "Error: Could not calculate package size. Make sure the directories exist."
    exit 1
fi

# Update installKBytes in distribution.xml
sed -i '' "s/installKBytes=\"[0-9]*\"/installKBytes=\"$PKG_SIZE\"/" distribution.xml

echo "Updated distribution.xml with installKBytes=\"$PKG_SIZE\""
