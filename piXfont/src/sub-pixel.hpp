// The MIT License (MIT)
//
// Copyright (c) 2024-2025 Insoft. All rights reserved.
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

#ifndef sub_pixel_hpp
#define sub_pixel_hpp

#include <cstdint>
#include <vector>

//typedef struct {
//    uint8_t    width, height;   // Bitmap dimensions in pixels.
//    uint8_t    xAdvance;        // Distance (in sub-pixels) to advance after drawing this glyph.
//    int8_t     dX;              // Verical offset (in sub-pixels) relative to the origin.
//    int8_t     dY;              // Distance from the baseline of the character to the top of the glyph.
//    int8_t     type;            // 1: monochrome bitmap glyph, 2: RGB332 bitmap glyph, 3: sub-pixel
//} TCrystalGlyphData;
//
//typedef struct {
//    uint16_t utf16;
//    int16_t offset;
//} TCrystalGlyphIndexOffset;
//
//typedef struct {
//    uint8_t   *glyphData;       // Glyph data.
//    uint8_t    yAdvance;        // Newline distance in the y-axis.
//    uint16_t  *glyphOffset;
//    TCrystalGlyphIndexOffset *glyphIndexOffset;
//} TCrystalFont;
//
//typedef struct {
//    uint8_t   *glyphData;          // Glyph bitmaps, concatenated.
//    uint8_t    yAdvance;        // Newline distance in the y-axis.
//    std::vector<uint16_t> glyphOffset;
//    std::vector<TCrystalGlyphIndexOffset> glyphIndexOffset;
//} TCrystalType;
//
//uint8_t data[] = {
//    0
//};
//
//uint16_t glythOffsets[] = {
//    0, 0
//};
//
//TCrystalGlyphIndexOffset gio[] = {
//    { 0, 0}
//};
//
//TCrystalFont cf = {
//    0, 14, glythOffsets, gio
//};

void processSubPixels(uint8_t *input, uint32_t *output, const int width);

#endif /* sub_pixel_hpp */
