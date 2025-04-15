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

#ifndef calctype_hpp
#define calctype_hpp

#include "image.hpp"

#include <vector>
#include <cstdint>

#define R_CHANNEL(rgb) (rgb >> 5)
#define G_CHANNEL(rgb) ((rgb & 0b00011100) >> 2)
#define B_CHANNEL(rgb) (rgb & 0b00000011)

namespace calctype {
    typedef struct CalcTypeGlyphData {
        int8_t   xOffset;                // Horizontal offset (in subpixels) of the glyph relative to the origin.
        int8_t   yOffset;                // Vertical offset (in pixels) of the glyph relative to the baseline.
        uint8_t  xAdvance;               // Horizontal distance (in subpixels) to advance after drawing this glyph.
        uint8_t  width;                  // Width of the glyph in real pixels (subpixel encoded).
        uint8_t  height;                 // Height of the glyph in real pixels.
        // data...
    } TCalcTypeGlyphData;
    
    typedef struct {
        uint8_t height;
        int8_t  base;
        uint8_t space;
        std::vector<uint8_t> glyphData;
        uint16_t glyphOffset[224];
    } TCalcTypeFont;
    
    bool decodeFont(const std::string &filename, TCalcTypeFont &font);
    unsigned int glyphWidth(const TCalcTypeFont &font, const char charactor);
    void drawGlyph(calctype::TCalcTypeGlyphData *glyph, int x, int y, uint32_t color, const image::TImage &image);
    void drawRawGlyph(calctype::TCalcTypeGlyphData *glyph, int x, int y, const image::TImage &image);
    void drawString(const TCalcTypeFont &font, const char *str, int x, int y, uint32_t color, const image::TImage &image);
    bool isCalcType(const std::string &filename);
}

#endif /* calctype_hpp */
