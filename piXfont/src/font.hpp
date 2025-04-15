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


#pragma once

#include <iostream>
#include <sstream>
#include <cstdint>

#include "graphics.hpp"

namespace font {
    
    typedef struct {
        uint16_t   bitmapOffset;    // Offset address into the bitmap data.
        uint8_t    width, height;   // Bitmap dimensions in pixels.
        uint8_t    xAdvance;        // Distance to advance cursor in the x-axis.
        int8_t     dX;              // Used to position the glyph within the cell in the horizontal direction.
        int8_t     dY;              // Distance from the baseline of the character to the top of the glyph.
    } TGlyph;

    typedef struct {
        uint8_t   *bitmap;          // Glyph bitmaps, concatenated.
        TGlyph    *glyph;           // Glyph array.
        uint8_t   first;           // The first UTF16 value of your first character.
        uint8_t   last;            // The last UTF16 value of your last character.
        uint8_t    yAdvance;        // Newline distance in the y-axis.
    } TFont;
    
    /*
     The UTF8 table stores the index of each glyph within the glyph table, indicating where the glyph entry
     for a particular UTF16 character resides. The table does not include entries for the first and last glyph
     indices, as they are unnecessary: the first glyph index always corresponds to the first glyph entry, and
     the last glyph in the font is always the last glyph entry.
     */
    
    typedef struct {
        std::vector<uint8_t> data;
        std::vector<TGlyph> glyphs;
        uint8_t first;
        uint8_t last;
        uint8_t yAdvance;
    } TAdafruitFont;
    
    /**
     @brief Draw a single character
     @param x Bottom left corner x coordinate
     @param y Bottom left corner y coordinate
     @param c The 8-bit font-indexed character (likely ascii)
     @param color Color to draw chraracter with
     @param sizeX Font magnification level in X-axis, 1 is 'original' size
     @param sizeY Font magnification level in Y-axis, 1 is 'original' size
     */
    uint8_t drawGlyph(int16_t x, int16_t y, unsigned char c, const uint8_t color, uint8_t sizeX, uint8_t sizeY, const TFont &font, const image::TImage &image);
    uint8_t drawGlyph(int16_t x, int16_t y, unsigned char c, const uint32_t color, uint8_t sizeX, uint8_t sizeY, const TFont &font, const image::TImage &image);

    int print(int16_t x, int16_t y, const char *s, const uint8_t color, const TFont &font, const image::TImage &image);
    int print(int16_t x, int16_t y, const char *s, const uint32_t color, const TFont &font, const image::TImage &image);
}

