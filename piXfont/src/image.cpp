// The MIT License (MIT)
//
// Copyright (c) 2024-2025 Insoft. All rights reserved.
// Originally created in 2023
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

#include "image.hpp"

#include <fstream>


#include "pbm.hpp"
#include "bmp.hpp"
#include "png.hpp"


image::TImage image::loadImage(const char *filename)
{
    TImage image;
    
    auto bmp = bmp::load(filename);
    if (!bmp.bytes.empty()) {
        image.width = bmp.width;
        image.height = bmp.height;
        image.bpp = bmp.bpp;
        image.palette = bmp.palette;
        image.bytes = bmp.bytes;
        return image;
    }
    
    auto pbm = pbm::load(filename);
    if (!pbm.bytes.empty()) {
        image.width = pbm.width;
        image.height = pbm.height;
        image.bpp = 1;
        image.bytes = pbm.bytes;
        return image;
    }

    auto png = png::load(filename);
    if (!png.bytes.empty()) {
        image.width = pbm.width;
        image.height = pbm.height;
        image.bpp = bmp.bpp;
        image.bytes = bmp.bytes;
        return image;
    }

    return image;
}

bool image::saveImage(const char *filename, const image::TImage &image)
{
    std::filesystem::path filePath(filename);
    std::string extension = filePath.extension();
    
    if (extension == ".bmp") {
        bmp::TImage newImage = {
            .width = image.width,
            .height = image.height,
            .bpp = image.bpp,
            .palette = image.palette,
            .bytes = image.bytes
        };
        return bmp::save(filename, newImage);
    }
    
    if (extension == ".png") {
        TImage RGBAImage = convert256ColorImageToRGBAImage(image);
        
        png::TImage pngImage = {
            .width = image.width,
            .height = image.height,
            .bpp = 32,
            .bytes = RGBAImage.bytes
        };
        return png::save(filename, pngImage);
    }
    
    return false;
}

void image::setPaletteToRGB332(image::TImage &image)
{
    image.palette.resize(0);
    
    static const uint32_t R[] = {
        0, 36, 73, 109, 146, 182, 219, 255
    };
    
    static const uint32_t G[] = {
        0, 36, 73, 109, 146, 182, 219, 255
    };
    
    static const uint32_t B[] = {
        0, 109, 182, 255
    };
    
    for (int i = 0; i < 256; i++) {
        uint32_t color;
        color = 0xFF000000 | B[i & 3] << 16 | G[i >> 3 & 7] << 8 | R[i >> 5];
        image.palette.push_back(color);
    }
}

image::TImage image::createImage(int w, int h, uint8_t bpp)
{
    TImage image = {
        .width = static_cast<uint16_t>(w),
        .height = static_cast<uint16_t>(h),
        .bpp = bpp
    };
    
    size_t length;
    
    length = w * h;
    if (bpp == 1) length /= 8;
    if (bpp == 16) length *= 2;
    if (bpp == 32) length *= sizeof(uint32_t);
    
    
    image.bytes.reserve(length);
    image.bytes.resize(length);
    
    image.palette.reserve(2);
    image.palette.resize(2);
    
    image.palette.at(1) = 0xFF000000;
    image.palette.at(0) = 0xFFFFFFFF;

    return image;
}

void image::copyImage(const TImage &dst, int dx, int dy, const TImage &src, int x, int y, uint16_t w, uint16_t h)
{
    uint8_t *d = (uint8_t *)dst.bytes.data();
    uint8_t *s = (uint8_t *)src.bytes.data();
    
    d += dx + dy * dst.width;
    s += x + y * src.width;
    while (h--) {
        for (int i=0; i<w; i++) {
            d[i] = s[i];
        }
        d += dst.width;
        s += src.width;
    }
}

image::TImage image::convertMonochromeToGrayScale(const TImage monochrome)
{
    TImage image;
    
    if (monochrome.bpp != 1) return image;

    image.bpp = 8;
    image.width = monochrome.width;
    image.height = monochrome.height;
    size_t length = (size_t)monochrome.width * (size_t)monochrome.height;
    image.bytes.reserve(length);
    image.bytes.resize(length);
    
   
    
    for (uint32_t color = 0xFF000000; image.palette.size() != 256; color += 0x010101) {
        image.palette.push_back(color);
    }
    
    uint8_t *src = (uint8_t *)monochrome.bytes.data();
    uint8_t *dest = (uint8_t *)image.bytes.data();
    uint8_t bitPosition = 1 << 7;
    
    int x, y;
    for (y=0; y<monochrome.height; y++) {
        bitPosition = 1 << 7;
        for (x=0; x<monochrome.width; x++) {
            *dest++ = (*src & bitPosition ? 0 : 255);
            if (bitPosition == 1) {
                src++;
                bitPosition = 1 << 7;
            } else {
                bitPosition >>= 1;
            }
        }
        if (x & 7) src++;
    }
    
    return image;
}

image::TImage image::convertMonochromeToIndex(const TImage monochrome)
{
    TImage image;

    if (monochrome.bpp != 1) return image;

    image.bpp = 8;
    image.width = monochrome.width;
    image.height = monochrome.height;
    size_t length = (size_t)monochrome.width * (size_t)monochrome.height;
    image.bytes.reserve(length);
    image.bytes.resize(length);
    
    image.palette.push_back(0x0);
    image.palette.push_back(0xFFFFFFFF);

    uint8_t *src = (uint8_t *)monochrome.bytes.data();
    uint8_t *dest = (uint8_t *)image.bytes.data();
    uint8_t bitPosition = 1 << 7;

    int x, y;
    for (y=0; y<monochrome.height; y++) {
        bitPosition = 1 << 7;
        for (x=0; x<monochrome.width; x++) {
            *dest++ = (*src & bitPosition ? 1 : 0);
            if (bitPosition == 1) {
                src++;
                bitPosition = 1 << 7;
            } else {
                bitPosition >>= 1;
            }
        }
        if (x & 7) src++;
    }

    return image;
}

image::TImage image::convert16ColorTo256Color(const TImage &image)
{
    TImage newImage;
    
    newImage = createImage(image.width, image.height, Index256Colors);
    
    for (auto it = image.bytes.begin(); it < image.bytes.end(); it++) {
        newImage.bytes.push_back(*it >> 4);
        newImage.bytes.push_back(*it & 15);
    }
    
    newImage.bpp = 8;
    newImage.palette = image.palette;
    
    return newImage;
}

image::TImage image::convert256ColorImageToRGBAImage(const TImage &image)
{
    TImage newImage;
    
    newImage = createImage(image.width, image.height, TrueColor);
    if (image.bytes.empty()) return newImage;
    
    uint32_t *dst = (uint32_t *)newImage.bytes.data();
    for (auto it = image.bytes.begin(); it < image.bytes.end(); it++) {
        *dst++ = image.palette.at(*it);
    }
    
    newImage.bpp = 32;
    
    return newImage;
}


bool image::containsImage(const TImage &image, uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
    if (image.bytes.empty()) return false;
    if (x + w > image.width || y + h > image.height) return false;
    uint8_t *p = (uint8_t *)image.bytes.data();
    
    p += x + y * image.width;
    while (h--) {
        for (int i=0; i<w; i++) {
            if (p[i]) return true;
        }
        p+=image.width;
    }
    return false;
}

image::TImage image::extractImageSection(TImage &image)
{
    TImage extractedImage;
    
    int minX, maxX, minY, maxY;
    
    if (image.bytes.empty()) return extractedImage;
    
    uint8_t *p = (uint8_t *)image.bytes.data();
    
    maxX = 0;
    maxY = 0;
    minX = image.width - 1;
    minY = image.height - 1;
    
    for (int y=0; y<image.height; y++) {
        for (int x=0; x<image.width; x++) {
            if (!p[x + y * image.width]) continue;
            if (minX > x) minX = x;
            if (maxX < x) maxX = x;
            if (minY > y) minY = y;
            if (maxY < y) maxY = y;
        }
    }
    
    if (maxX < minX || maxY < minY)
        return extractedImage;
    
    
    int width = maxX - minX + 1;
    int height = maxY - minY + 1;
    
    extractedImage = createImage(width, height, image.bpp);
    if (extractedImage.bytes.empty()) return extractedImage;
    copyImage(extractedImage, 0, 0, image, minX, minY, width, height);
    
    return extractedImage;
}

void image::invertImage(TImage &image)
{
    size_t length = (size_t)((float)image.width / (8.0 / (float)image.bpp)) * image.height;
    uint8_t *bytes = image.bytes.data();
    while (length--) {
        *bytes = ~*bytes;
        bytes++;
    }
}
