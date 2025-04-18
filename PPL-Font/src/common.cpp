// The MIT License (MIT)
//
// Copyright (c) 2024-2025 Insoft. All rights reserved.
// Originally created in 2025
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "common.hpp"
#include <regex>


std::ifstream::pos_type cmn::filesize(const char* filename)
{
    std::ifstream in(filename, std::ifstream::ate | std::ifstream::binary);
    std::ifstream::pos_type pos = in.tellg();
    in.close();
    return pos;
}


uint8_t cmn::mirror_byte(uint8_t b) {
    b = ((b & 0xF0) >> 4) | ((b & 0x0F) << 4);  // Swap upper and lower 4 bits
    b = ((b & 0xCC) >> 2) | ((b & 0x33) << 2);  // Swap pairs of bits
    b = ((b & 0xAA) >> 1) | ((b & 0x55) << 1);  // Swap individual bits
    return b;
}

int cmn::parse_number(const std::string &str)
{
    std::regex hexPattern("^0x[\\da-fA-F]+$");
    std::regex decPattern("^[+-]?\\d+$");
    std::regex octPattern("^0[0-8]+$");
    std::regex binPattern("^0b[01]+$");
    
    if (std::regex_match(str, hexPattern)) return std::stoi(str, nullptr, 16);
    if (std::regex_match(str, decPattern)) return std::stoi(str, nullptr, 10);
    if (std::regex_match(str, octPattern)) return std::stoi(str, nullptr, 8);
    if (std::regex_match(str, binPattern)) return std::stoi(str, nullptr, 2);
    
    return 0;
}

