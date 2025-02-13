#!/bin/bash

# MIT License
# 
# Copyright (c) 2025 insoft
# 
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.


# Detect operating system
OS=$(uname -s)

# Detect hardware architecture
ARCH=$(uname -m)

echo "Operating System: $OS"
echo "Architecture: $ARCH"

# Example: Check platform
if [[ "$OS" == "Linux" && "$ARCH" == "x86_64" ]]; then
    echo "Running on a 64-bit Linux platform."
    exit
elif [[ "$OS" == "Darwin" ]]; then
    echo "Running on macOS."
    if [[ "$ARCH" == *"arm64"* ]]; then
        echo "AppleSilicon"
    else
        echo "Intel"
    fi
else
    echo "Platform: $OS $ARCH"
    exit
fi


mkdir -p ~/primeplus-toolchain
cd ~/primeplus-toolchain

if [ ! -d "p+" ]; then
    if [ ! -f "p+_3.1.2.zip" ]; then
        wget http://insoft.uk/downloads/src/hp/p+_3.1.2.zip
    fi
    unzip p+_3.1.2.zip -d p+
    rm p+_3.1.2.zip
fi

if [ ! -d "pplref" ]; then
    if [ ! -f "pplref_1.0.0.zip" ]; then
        wget http://insoft.uk/downloads/src/hp/pplref_1.0.0.zip
    fi
    unzip pplref_1.0.0.zip -d pplref
    rm pplref_1.0.0.zip
fi

if [ ! -d "pplmin" ]; then
    if [ ! -f "pplmin_1.0.1.zip" ]; then
        wget http://insoft.uk/downloads/src/hp/pplmin_1.0.1.zip
    fi
    unzip pplmin_1.0.1.zip -d pplmin
    rm pplmin_1.0.1.zip
fi

if [ ! -d "grob" ]; then
    if [ ! -f "grob_1.1.4.zip" ]; then
        wget http://insoft.uk/downloads/src/hp/grob_1.1.4.zip
    fi
    unzip grob_1.1.4.zip -d grob
    rm grob_1.1.4.zip
fi

cd ~/primeplus-toolchain/p+

make uninstall


cd ~/primeplus-toolchain/pplref

make uninstall


make -j$(sysctl -n hw.ncpu)
make uninstall

cd ~/primeplus-toolchain/grob

make uninstall

cd ../..
rmdir -p -r primeplus-toolchain
rmdir -p -r hpprgm ~/Documents/HP\ Prime/SDK/Libraries

read -p "Press Enter to continue..."


