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

#include "bmp.hpp"

#include <fstream>

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

static void flipBitmapImageVertically(const TBitmap& bitmap)
{
    uint8_t *byte = (uint8_t *)bitmap.bytes.data();
    int w = (int)((float)bitmap.width / (8.0 / (float)bitmap.bpp));
    int h = (int)bitmap.height;
    
    for (int row = 0; row < h / 2; ++row)
        for (int col = 0; col < w; ++col)
            std::swap(byte[col + row * w], byte[col + (h - 1 - row) * w]);
    
}

TBitmap loadBitmapImage(const std::string& filename)
{
    BIPHeader bip_header;
    
    std::ifstream infile;
    
    TBitmap bitmap{};
    
    
    infile.open(filename, std::ios::in | std::ios::binary);
    if (!infile.is_open()) {
        return bitmap;
    }
    
    infile.read((char *)&bip_header, sizeof(BIPHeader));

    if (strncmp(bip_header.fileHeader.bfType, "BM", 2) != 0) {
        infile.close();
        return bitmap;
    }
    
    bitmap.bpp = bip_header.biBitCount;
    bitmap.bytes.reserve(bip_header.biSizeImage);
    bitmap.bytes.resize(bip_header.biSizeImage);
    if (bitmap.bytes.empty()) {
        infile.close();
        return bitmap;
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
#ifdef __LITTLE_ENDIAN__
            color = swap_endian(color);
#endif
            bitmap.palette.push_back(color | 255);
        }
    }
    
    bitmap.palette.resize(bip_header.biClrUsed);
    
    bitmap.width = abs(bip_header.biWidth);
    bitmap.height = abs(bip_header.biHeight);
    size_t length = (size_t)((float)bitmap.width / (8.0 / (float)bip_header.biBitCount));
    uint8_t* bytes = (uint8_t *)bitmap.bytes.data();
    
    infile.seekg(bip_header.fileHeader.bfOffBits, std::ios_base::beg);
    for (int r = 0; r < bitmap.height; ++r) {
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
        flipBitmapImageVertically(bitmap);
    
    return bitmap;
}


