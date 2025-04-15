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

#include "bmp.hpp"

#include <fstream>
#include <iostream>

/* Windows 3.x bitmap file header */
typedef struct __attribute__((__packed__)) {
    char      bfType[2];   /* magic - always 'B' 'M' */
    uint32_t  bfSize;
    uint16_t  bfReserved1;
    uint16_t  bfReserved2;
    uint32_t  bfOffBits;    /* offset in bytes to actual bitmap data */
} BMPHeader;

/* Windows 3.x bitmap full header, including file header */

typedef struct __attribute__((__packed__)) {
    BMPHeader fileHeader;
    uint32_t  biSize;
    int32_t   biWidth;
    int32_t   biHeight;
    int16_t   biPlanes;           // Number of colour planes, set to 1
    int16_t   biBitCount;         // Colour bits per pixel. 1 4 8 24 or 32
    uint32_t  biCompression;      // *Code for the compression scheme
    uint32_t  biSizeImage;        // *Size of the bitmap bits in bytes
    int32_t   biXPelsPerMeter;    // *Horizontal resolution in pixels per meter
    int32_t   biYPelsPerMeter;    // *Vertical resolution in pixels per meter
    uint32_t  biClrUsed;          // *Number of colours defined in the palette
    uint32_t  biClImportant;      // *Number of important colours in the image
} BIPHeader;

template <typename T> static T swap_endian(T u) {
    static_assert (CHAR_BIT == 8, "CHAR_BIT != 8");

    union {
        T u;
        unsigned char u8[sizeof(T)];
    } source, dest;

    source.u = u;

    for (size_t k = 0; k < sizeof(T); k++)
        dest.u8[k] = source.u8[sizeof(T) - k - 1];

    return dest.u;
}

static void flipBitmapImageVertically(const bmp::TImage &image)
{
    uint8_t *byte = (uint8_t *)image.bytes.data();
    int w = (int)((float)image.width / (8.0 / (float)image.bpp));
    int h = (int)image.height;
    
    for (int row = 0; row < h / 2; ++row)
        for (int col = 0; col < w; ++col)
            std::swap(byte[col + row * w], byte[col + (h - 1 - row) * w]);
    
}

bmp::TImage bmp::load(const char *filename)
{
    BIPHeader bip_header;
    
    std::ifstream infile;
    
    TImage image;
    
    
    infile.open(filename, std::ios::in | std::ios::binary);
    if (!infile.is_open()) {
        return image;
    }
    
    infile.read((char *)&bip_header, sizeof(BIPHeader));

    if (strncmp(bip_header.fileHeader.bfType, "BM", 2) != 0) {
        infile.close();
        return image;
    }
    
    image.bpp = bip_header.biBitCount;
    if (bip_header.biSizeImage == 0) {
        /*
         If the BMP file uses BI_RGB (no compression), biSizeImage
         may be set to 0 because the size can be inferred from the
         image width, height, and bit depth.
         */
        bip_header.biSizeImage = (bip_header.biWidth * bip_header.biBitCount + 31) / 32 * 4 * abs(bip_header.biHeight);
    }
    image.bytes.reserve(bip_header.biSizeImage);
    image.bytes.resize(bip_header.biSizeImage);
    if (image.bytes.empty()) {
        infile.close();
        return image;
    }
    
    /*
     We verify whether the image data begins immediately after the file header.
     If it does not, we ensure that biClrUsed is set correctly. Some software
     that generates BMP files with a palette may incorrectly set biClrUsed to
     zero, even when a palette is present, and this value needs to be corrected.
     */
    if (bip_header.biClrUsed == 0) {
        if (bip_header.fileHeader.bfOffBits > sizeof(BIPHeader)) {
            bip_header.biClrUsed = (bip_header.fileHeader.bfOffBits - sizeof(BIPHeader)) / sizeof(uint32_t);
            bip_header.biClImportant = bip_header.biClrUsed;
        }
    }
    
    if (bip_header.biClrUsed) {
        for (int i = 0; i < bip_header.biClrUsed; i += 1) {
            uint32_t color;
            infile.read((char *)&color, sizeof(uint32_t));
            // Color Table is stored in ARGB (be) arangment.
#ifdef __LITTLE_ENDIAN__
            // ARGB (be) to ARGB (le)
            color = swap_endian(color);
#endif
            color = color | 255;

            image.palette.push_back(color);
        }
    }
    
    image.palette.resize(bip_header.biClrUsed);
    
    image.width = abs(bip_header.biWidth);
    image.height = abs(bip_header.biHeight);
    size_t length = (size_t)((float)image.width / (8.0 / (float)bip_header.biBitCount));
    uint8_t* bytes = (uint8_t *)image.bytes.data();
    
    infile.seekg(bip_header.fileHeader.bfOffBits, std::ios_base::beg);
    for (int r = 0; r < image.height; ++r) {
        infile.read((char *)&bytes[length * r], length);
        if (infile.gcount() != length) {
            std::cout << filename << " Read failed!\n";
            break;
        }
        
        /*
         Each scan line is zero padded to the nearest 4-byte boundary.
         
         If the image has a width that is not divisible by four, say, 21 bytes, there
         would be 3 bytes of padding at the end of every scan line.
         */
        if (length % 4)
            infile.seekg(4 - length % 4, std::ios_base::cur);
    }
    
    infile.close();
    
    if (bip_header.biHeight > 0)
        flipBitmapImageVertically(image);
    
    
    return image;
}

bool bmp::save(const char *filename, const TImage &image)
{
    BIPHeader bip_header = {
        .fileHeader = {
            .bfType = {'B', 'M'},
            .bfSize = 0,
            .bfOffBits = 0
        },
            .biSize = sizeof(BIPHeader) - sizeof(BMPHeader),
            .biWidth = image.width,
            .biHeight = image.height,
            .biPlanes = 1,
            .biCompression = static_cast<uint32_t>(image.bpp == 16 ? 3 : 0),
            .biBitCount = image.bpp,
            .biSizeImage = static_cast<uint32_t>((image.width * image.bpp + 31) / 32 * 4 * (image.height)),
            .biClrUsed = image.bpp >= 16 ? 0 : static_cast<uint32_t>(image.palette.size()),
            .biClImportant = image.bpp >= 16 ? 0 : static_cast<uint32_t>(image.palette.size())
    };
    
    bip_header.biSizeImage = (bip_header.biWidth * bip_header.biBitCount + 31) / 32 * 4 * abs(bip_header.biHeight);
    
    bip_header.fileHeader.bfType[0] = 'B';
    bip_header.fileHeader.bfType[1] = 'M';
    bip_header.fileHeader.bfSize = bip_header.biSizeImage + (uint32_t)image.palette.size() * sizeof(uint32_t);
    bip_header.fileHeader.bfOffBits = sizeof(BIPHeader) + bip_header.biClrUsed * sizeof(uint32_t);

    std::ofstream outfile;
    outfile.open(filename, std::ios::out | std::ios::binary);
    if (!outfile.is_open()) {
        return false;
    }
    
    outfile.write((char *)&bip_header, sizeof(BIPHeader));
    if (bip_header.biClrUsed) {
        outfile.write((char *)image.palette.data(), bip_header.biClrUsed * sizeof(uint32_t));
    }
    
    int bytesPerLine = (float)image.width / (8.0 / (float)image.bpp);
    char *pixelData = (char *)image.bytes.data();
    
    flipBitmapImageVertically(image);
    
    for (int line = 0; line < image.height; line++) {
        outfile.write(pixelData, bytesPerLine);
        pixelData += bytesPerLine;
        
        /*
         Each scan line is zero padded to the nearest 4-byte boundary.
         
         If the image has a width that is not divisible by four, say, 21 bytes, there
         would be 3 bytes of padding at the end of every scan line.
         */
        int padding = bip_header.biSizeImage / abs(bip_header.biHeight) - bytesPerLine;
        while (padding--) {
            outfile.put(0);
        }
    }
    
    
    outfile.close();
    return true;
}


