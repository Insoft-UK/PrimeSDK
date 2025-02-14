#!/bin/bash

# Get the package size in KB
PKG_SIZE=$(du -sk package-root/ | awk '{print $1}')
# Replace the installKBytes value dynamically

sed -i '' "s/installKBytes=\"[0-9]*\"/installKBytes=\"$PKG_SIZE\"/" distribution.xml

echo "Updated distribution.xml with installKBytes=\"$PKG_SIZE\""
