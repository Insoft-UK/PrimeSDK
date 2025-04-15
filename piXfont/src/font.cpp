//
//  font.cpp
//  piXfont
//
//  Created by Richie on 03/03/2025.
//  Copyright © 2025 Insoft. All rights reserved.
//

#include "font.hpp"

uint8_t font::drawGlyph(int16_t x, int16_t y, unsigned char c, const uint8_t color, uint8_t sizeX, uint8_t sizeY, const TFont &font, const image::TImage &image)
{
    const TGlyph *glyph = &font.glyph[(int)c - font.first];
    
    int h = glyph->height;
    int w = glyph->width;
    
    x += glyph->dX;
    y += glyph->dY * sizeY;
    
    uint8_t *bitmap = font.bitmap + glyph->bitmapOffset;
    int bitPosition = 0;
    uint8_t bits = 0;
    
    while (h--) {
        for (int xx = 0; xx < w; xx++) {
            if (!(bitPosition++ & 7)) {
                bits = *bitmap++;
            }
            if (bits & 0x80) {
                if (sizeX == 1 && sizeY == 1) {
                    graphics::setPixel(x + xx, y, color, image);
                } else {
                    graphics::drawFillRect(x + xx * sizeX, y, sizeX, sizeY, color, image);
                }
            }
            bits <<= 1;
        }
        y+=sizeY;
    }
    
    return glyph->xAdvance;
}

uint8_t font::drawGlyph(int16_t x, int16_t y, unsigned char c, const uint32_t color, uint8_t sizeX, uint8_t sizeY, const TFont &font, const image::TImage &image)
{
    const TGlyph *glyph = &font.glyph[(int)c - font.first];
    
    int h = glyph->height;
    int w = glyph->width;
    
    x += glyph->dX;
    y += glyph->dY * sizeY;
    
    uint8_t *bitmap = font.bitmap + glyph->bitmapOffset;
    int bitPosition = 0;
    uint8_t bits = 0;
    
    while (h--) {
        for (int xx = 0; xx < w; xx++) {
            if (!(bitPosition++ & 7)) {
                bits = *bitmap++;
            }
            if (bits & 0x80) {
                if (sizeX == 1 && sizeY == 1) {
                    graphics::setPixel(x + xx, y, color, image);
                } else {
                    graphics::drawFillRect(x + xx * sizeX, y, sizeX, sizeY, color, image);
                }
            }
            bits <<= 1;
        }
        y+=sizeY;
    }
    
    return glyph->xAdvance;
}

int font::print(int16_t x, int16_t y, const char *s, const uint8_t color, const TFont &font, const image::TImage &image)
{
    uint8_t *c = (uint8_t *)s;
    uint8_t asciiCode;
    while (*c) {
        asciiCode = *c++;
        if (asciiCode < font.first || asciiCode > font.last) {
            switch (asciiCode) {
                case '\n':
                    x = 0;
                    y += font.yAdvance;
                    break;
            }
            continue;
        }
        x += (int16_t)drawGlyph(x, y, asciiCode, color, 1, 1, font, image);
    }
    return x;
}

int font::print(int16_t x, int16_t y, const char *s, const uint32_t color, const TFont &font, const image::TImage &image)
{
    uint8_t *c = (uint8_t *)s;
    uint8_t asciiCode;
    while (*c) {
        asciiCode = *c++;
        if (asciiCode < font.first || asciiCode > font.last) {
            switch (asciiCode) {
                case '\n':
                    x = 0;
                    y += font.yAdvance;
                    break;
            }
            continue;
        }
        x += (int16_t)drawGlyph(x, y, asciiCode, color, 1, 1, font, image);
    }
    return x;
}
