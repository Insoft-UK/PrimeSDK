// The MIT License (MIT)
//
// Copyright (c) 2023-2025 Insoft. All rights reserved.
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

#include "utf.hpp"


std::string utf::to_utf8(const std::wstring &wstr) {
    std::string utf8;
    uint16_t utf16 = 0;

    for (size_t i = 0; i < wstr.size(); i++) {
        utf16 = static_cast<uint16_t>(wstr[i]);

        if (utf16 <= 0x007F) {
            // 1-byte UTF-8: 0xxxxxxx
            utf8 += static_cast<char>(utf16 & 0x7F);
        } else if (utf16 <= 0x07FF) {
            // 2-byte UTF-8: 110xxxxx 10xxxxxx
            utf8 += static_cast<char>(0b11000000 | ((utf16 >> 6) & 0b00011111));
            utf8 += static_cast<char>(0b10000000 | (utf16 & 0b00111111));
        } else {
            // 3-byte UTF-8: 1110xxxx 10xxxxxx 10xxxxxx
            utf8 += static_cast<char>(0b11100000 | ((utf16 >> 12) & 0b00001111));
            utf8 += static_cast<char>(0b10000000 | ((utf16 >> 6) & 0b00111111));
            utf8 += static_cast<char>(0b10000000 | (utf16 & 0b00111111));
        }
    }

    return utf8;
}

std::wstring utf::to_utf16(const std::string &str) {
    std::wstring utf16;
    size_t i = 0;

    while (i < str.size()) {
        uint8_t byte1 = static_cast<uint8_t>(str[i]);

        if ((byte1 & 0b10000000) == 0) {
            // 1-byte UTF-8: 0xxxxxxx
            utf16 += static_cast<wchar_t>(byte1);
            i += 1;
        } else if ((byte1 & 0b11100000) == 0b11000000) {
            // 2-byte UTF-8: 110xxxxx 10xxxxxx
            if (i + 1 >= str.size()) break;
            uint8_t byte2 = static_cast<uint8_t>(str[i + 1]);

            uint16_t ch = ((byte1 & 0b00011111) << 6) |
                          (byte2 & 0b00111111);
            utf16 += static_cast<wchar_t>(ch);
            i += 2;
        } else if ((byte1 & 0b11110000) == 0b11100000) {
            // 3-byte UTF-8: 1110xxxx 10xxxxxx 10xxxxxx
            if (i + 2 >= str.size()) break;
            uint8_t byte2 = static_cast<uint8_t>(str[i + 1]);
            uint8_t byte3 = static_cast<uint8_t>(str[i + 2]);

            uint16_t ch = ((byte1 & 0b00001111) << 12) |
                          ((byte2 & 0b00111111) << 6) |
                          (byte3 & 0b00111111);
            utf16 += static_cast<wchar_t>(ch);
            i += 3;
        } else {
            // Invalid or unsupported UTF-8 sequence
            i += 1; // Skip it
        }
    }

    return utf16;
}

uint16_t utf::utf16(const char *str) {
    uint8_t *utf8 = (uint8_t *)str;
    uint16_t utf16 = *utf8;
    
    if ((utf8[0] & 0b11110000) == 0b11100000) {
        utf16 = utf8[0] & 0b11111;
        utf16 <<= 6;
        utf16 |= utf8[1] & 0b111111;
        utf16 <<= 6;
        utf16 |= utf8[2] & 0b111111;
        return utf16;
    }
    
    // 110xxxxx 10xxxxxx
    if ((utf8[0] & 0b11100000) == 0b11000000) {
        utf16 = utf8[0] & 0b11111;
        utf16 <<= 6;
        utf16 |= utf8[1] & 0b111111;
        return utf16;
    }
    
    return utf16;
}

void utf::write(const std::string &str, std::ofstream &outfile) {
    if (str.empty()) return;
    
    for ( int n = 0; n < str.length(); n++) {
        uint8_t *ascii = (uint8_t *)&str.at(n);
        if (str.at(n) == '\r') continue;
        
        // Output as UTF-16LE
        if (*ascii >= 0x80) {
            uint16_t utf16 = utf::utf16(&str.at(n));
            
#ifndef __LITTLE_ENDIAN__
            utf16 = utf16 >> 8 | utf16 << 8;
#endif
            outfile.write((const char *)&utf16, 2);
            if ((*ascii & 0b11100000) == 0b11000000) n++;
            if ((*ascii & 0b11110000) == 0b11100000) n+=2;
            if ((*ascii & 0b11111000) == 0b11110000) n+=3;
        } else {
            outfile.put(str.at(n));
            outfile.put('\0');
        }
    }
}
