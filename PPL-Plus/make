#!/bin/bash
DIR=$(dirname "$0")
cd $DIR
clear

make -j$(sysctl -n hw.ncpu) all

read -p "Press Enter to exit!"

# Close the Terminal window
osascript -e 'tell application "Terminal" to close window 1' & exit
