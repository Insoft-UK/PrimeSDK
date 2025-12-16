#!/bin/bash

# Exit immediately if a command fails
set -e

# Function to calculate KB size safely
calculate_size() {
    local path="$1"
    if [ -d "$path" ]; then
        du -sk "$path" 2>/dev/null | awk '{print $1}'
    elif [ -f "$path" ]; then
        du -sk "$path" 2>/dev/null | awk '{print $1}'
    else
        echo ""
    fi
}

PRIME_SIZE=$(calculate_size "primesdk.pkg")
HP_SIZE=$(calculate_size "hp.pkg")

# Check both were calculated
if [ -z "$PRIME_SIZE" ] || [ -z "$HP_SIZE" ] ]; then
    echo "❌ Error: Could not calculate one or more package sizes."
    echo "Make sure primesdk.pkg and hp.pkg (or their build folders) exist."
    exit 1
fi

# ---- Update distribution.dist ----
DIST_FILE="distribution.dist"

if [ ! -f "$DIST_FILE" ]; then
    echo "❌ Error: $DIST_FILE not found!"
    exit 1
fi

# Update installKBytes for both packages
sed -i '' "s|\(<pkg-ref id=\"uk\.insoft\.primesdk\"[^\>]*installKBytes=\)\"[0-9]*\"|\1\"$PRIME_SIZE\"|" "$DIST_FILE"
sed -i '' "s|\(<pkg-ref id=\"uk\.insoft\.hp\"[^\>]*installKBytes=\)\"[0-9]*\"|\1\"$HP_SIZE\"|" "$DIST_FILE"

echo "✅ Updated $DIST_FILE:"
echo "   - PrimeSDK installKBytes=\"$PRIME_SIZE\""
echo "   - HP   installKBytes=\"$HP_SIZE\""
