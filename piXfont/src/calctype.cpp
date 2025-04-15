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

#include "calctype.hpp"

#include "common.hpp"



#include <regex>
using namespace cmn;

static std::string load(const std::string &filename)
{
    std::ifstream infile;
    std::string str;
    
    // Open the file in text mode
    infile.open(filename, std::ios::in);
    
    // Check if the file is successfully opened
    if (!infile.is_open()) {
        return str;
    }
    
    if (is_utf16le(infile)) {
        char c;
        while (!infile.eof()) {
            infile.get(c);
            str += c;
            infile.peek();
        }
        
        uint16_t *utf16_str = (uint16_t *)str.c_str();
        str = utf16_to_utf8(utf16_str, str.size() / 2);
    } else {
        // Use a stringstream to read the file's content into the string
        std::stringstream buffer;
        buffer << infile.rdbuf();
        str = buffer.str();
    }

    infile.close();
    
    return str;
}

bool calctype::decodeFont(const std::string &filename, TCalcTypeFont &font)
{
    std::string utf8;
    utf8 = load(filename);
    
    std::string str;
    std::string s;
    std::regex re;
    
    // Check if the file is successfully opened
    if (utf8.empty()) {
        std::cerr << "Error no UTF8 data.\n";
        return false;
    }
    
    // Remove any and all comments befor decoding.
    s = std::regex_replace(utf8, std::regex(R"( *\/\/.*)"), "");
    
    std::smatch match;
    re = R"(\bconst unsigned char _[A-Z]\w*\[\]  *= *\{\s*([^}]*)\s*\};\s*const *TCalcTypeFont *[A-Z]\w* *= *\{\s*.height *= *(\d+),\s*\.base *= *(-\d+),\s*\.space *= *(\d+),\s*\.glyphData *= *_[A-Z]\w*,\s*\.glyphOffset *= *\{\s*([^\}]*)\}\s*\};)";
    
    if (std::regex_search(s, match, re) == false) return false;
    
    re = R"(\d+)";
    str = match.str(1);
    for (std::sregex_iterator it(str.begin(), str.end(), re), end; it != end; ++it) {
        uint8_t number = (uint8_t)std::stoi(it->str());
        font.glyphData.push_back(number);
    }
    
    font.height = (uint8_t)std::stoi(match.str(2));
    font.base = (int8_t)std::stoi(match.str(3));
    font.space = (uint8_t)std::stoi(match.str(4));

    str = match.str(5);
    str = std::regex_replace(str, std::regex(R"(\s)"), "");
    int i = 0;
    for (std::sregex_iterator it(str.begin(), str.end(), re), end; it != end; ++it) {
        uint16_t number = (uint16_t)std::stoi(it->str());
        font.glyphOffset[i++] = number;
    }
    
    return true;
}

void calctype::drawGlyph(calctype::TCalcTypeGlyphData *glyph, int x, int y, uint32_t color, const image::TImage &image)
{
    static const uint32_t redMapping[8] =
    {
        0x00000000, // 0
        0x00000018, // 24
        0x00000041, // 65
        0x00000062, // 98
        0x0000008B, // 139
        0x000000B4, // 180
        0x000000DE, // 222
        0x000000FF  // 255
    };
    
    static const uint32_t greenMapping[8] =
    {
        0x00000000, // 0
        0x00001800, // 24
        0x00004100, // 65
        0x00006200, // 98
        0x00008B00, // 139
        0x0000B400, // 180
        0x0000DE00, // 222
        0x0000FF00  // 255
    };

    static const uint32_t blueMapping[4] =
    {
        0x00000000, // 0
        0x00310000, // 49
        0x008B0000, // 139
        0x00FF0000  // 255
    };
    
    uint32_t rgb888;
    uint32_t invRgb888;
    
    uint32_t *dest = ((uint32_t *)image.bytes.data());
    dest += x + y * image.width;
    
    unsigned char *data = (unsigned char *)glyph + sizeof(calctype::TCalcTypeGlyphData);

    for (y = 0; y < glyph->height; y++) {
        for (x = 0; x < glyph->width; x++) {
            uint32_t currentPixel = *dest;
            uint8_t rgb = *data++;
            
            rgb888 = redMapping[R_CHANNEL(rgb)] | greenMapping[G_CHANNEL(rgb)] | blueMapping[B_CHANNEL(rgb)];
            
            /*
             In order for fast interpolation to work, the inverse of the mask value
             must be the bit inverse to prevent color overlap
             */
            rgb = ~rgb;
            invRgb888 = redMapping[R_CHANNEL(rgb)] | greenMapping[G_CHANNEL(rgb)] | blueMapping[B_CHANNEL(rgb)];
         
            *dest++ = (color & rgb888) | (currentPixel & invRgb888) | 0xFF000000;
        }
        dest += image.width - glyph->width;
    }
}

void calctype::drawRawGlyph(calctype::TCalcTypeGlyphData *glyph, int x, int y, const image::TImage &image)
{
    static const uint32_t redMapping[8] =
    {
        0x00000000, // 0
        0x00000024, // 36
        0x00000049, // 73
        0x0000006D, // 109
        0x00000092, // 146
        0x000000B6, // 182
        0x000000DB, // 219
        0x000000FF  // 255
    };
    
    static const uint32_t greenMapping[8] =
    {
        0x00000000, // 0
        0x00002400, // 36
        0x00004900, // 73
        0x00006D00, // 109
        0x00009200, // 146
        0x0000B600, // 182
        0x0000DB00, // 219
        0x0000FF00  // 255
    };

    static const uint32_t blueMapping[4] =
    {
        0x00000000, // 0
        0x006D0000, // 109
        0x00B60000, // 182
        0x00FF0000  // 255
    };
    
    uint32_t *dest = ((uint32_t *)image.bytes.data());
    dest += x + y * image.width;
    
    unsigned char *data = (unsigned char *)glyph + sizeof(calctype::TCalcTypeGlyphData);

    for (y = 0; y < glyph->height; y++) {
        for (x = 0; x < glyph->width; x++) {
            uint8_t rgb = *data++;
            
            *dest++ = redMapping[R_CHANNEL(rgb)] | 0xFF000000;
            *dest++ = greenMapping[G_CHANNEL(rgb)] | 0xFF000000;
            *dest++ = blueMapping[B_CHANNEL(rgb)] | 0xFF000000;
        }
        dest += image.width - glyph->width * 3;
    }
}


unsigned int calctype::glyphWidth(const TCalcTypeFont &font, const char charactor)
{
    unsigned int subPixelWidth = 0;
    unsigned short dataOffset;
                
    dataOffset = font.glyphOffset[(int)charactor - 32];
    if (dataOffset == 0xFFFF) return font.space;
    
    subPixelWidth = ((TCalcTypeGlyphData *)font.glyphData.data() + dataOffset)->xAdvance;
             
    return subPixelWidth / 3;
}

void calctype::drawString(const TCalcTypeFont &font, const char *str, int x, int y, uint32_t color, const image::TImage &image)
{
    int subX = x * 3;
    unsigned short glyphOffset;
    
    y += font.base;
    
    while (*str && subX < image.width * 3) {
        char charactor = *str++;
 
        switch (charactor) {
            case '\n':
                subX = x * 3;
                y += font.height;
                break;
                
            case ' ':
                subX += font.space;
                break;
                
            default:
                if (charactor < ' ') continue;
                
                glyphOffset = font.glyphOffset[charactor - 32];
                if (glyphOffset == 0xFFFF) continue;
                
                TCalcTypeGlyphData *glyph = (TCalcTypeGlyphData *)font.glyphData.data() + glyphOffset;
                drawGlyph(glyph, (subX + glyph->xOffset) / 3, y + glyph->yOffset, color, image);
                subX += glyph->xAdvance;
                if (subX >= image.width * 3) {
                    subX = x * 3;
                    y += font.height;
                }
                break;
        }
    }
}

bool calctype::isCalcType(const std::string &filename)
{
    std::smatch match;
    std::regex re;
    std::string str;
    
    str = load(filename);

    re = R"(\bconst unsigned char _[A-Z]\w*\[\]  *= *\{\s*([^}]*)\s*\};\s*const *TCalcTypeFont *[A-Z]\w* *= *\{\s*.height *= *(\d+),\s*\.base *= *(-\d+),\s*\.space *= *(\d+),\s*\.glyphData *= *_[A-Z]\w*,\s*\.glyphOffset *= *\{\s*([^\}]*)\}\s*\};)";
    bool result = std::regex_search(str, match, re);
    
    return result;
}

