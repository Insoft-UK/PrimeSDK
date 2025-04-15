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


#pragma once

#include <iostream>
#include <sstream>
#include <stdint.h>
#include <vector>

namespace image {
    typedef struct {
        uint16_t width;
        uint16_t height;
        uint8_t  bpp;
        std::vector<uint32_t> palette;
        std::vector<uint8_t> bytes;
    } TImage;
    
    enum BPP : uint8_t {
        TrueColor = 32,
        HighColor = 16,
        Index256Colors = 8,
        Index16Colors = 4,
        Index4Colors = 2,
        Monochrome = 1
    };
    
    /**
     @brief    Loads a file in the Bitmap (BMP) or Portable Bitmap (PBM)  format.
     @param    filename The filename of the Bitmap (BMP) or Portable Bitmap (PBM)  to be loaded.
     @return   A structure containing the bitmap image data.
     */
    TImage loadImage(const char *filename);
    
    /**
     @brief    Saves a file in the Bitmap (BMP) format.
     @param    filename The filename of the Bitmap (BMP)  to be saved.
     @param    image A structure containing the bitmap image data.
     */
    bool saveImage(const char *filename, const TImage &image);
    
    void setPaletteToRGB332(image::TImage &image);
    
    /**
     @brief Creates a bitmap with the specified dimensions.
     @param w The width of the bitmap.
     @param h The height of the bitmap.
     @return A structure containing the bitmap image data.
     */
    TImage createImage(int w, int h, uint8_t bpp);
    
    /**
     @brief Copies a section of a bitmap to another bitmap.
     @param dst The bitmap to which the section will be copied.
     @param dx The horizontal position where the copied section will be placed within the destination bitmap.
     @param dy The vertical axis position within the destination bitmap where the copied section will be placed.
     @param src The bitmap from which the section will be copied.
     @param x The x-axis from which the bitmap will be copied.
     @param y The y-axis source of the bitmap to be copyed from.
     @param w The width of the bitmap to be copied.
     @param h The height of the bitmap to be copied.
     */
    void copyImage(const TImage &dst, int dx, int dy, const TImage &src, int x, int y, uint16_t w, uint16_t h);
    
    /**
     @brief Converts a monochrome bitmap to a grayscale pixmap, where each pixel is represented by a single byte.
     @param monochrome The monochrome bitmap to be converted to a grayscale pixmap bitmap.
     @return A structure containing the bitmap image data.
     */
    TImage convertMonochromeToGrayScale(const TImage monochrome);
    
    
    /**
     @brief Converts a monochrome bitmap to a pixmap, where each pixel is represented by a single byte.
     @param monochrome The monochrome bitmap to be converted to a pixmap bitmap.
     @return A structure containing the bitmap image data.
     */
    TImage convertMonochromeToIndex(const TImage monochrome);
    
    /**
     @brief Converts a monochrome bitmap to a pixmap, where each pixel is represented by a single byte.
     @param monochrome The monochrome bitmap to be converted to a pixmap bitmap.
     @return A structure containing the bitmap image data.
     */
    TImage convert16ColorTo256Color(const TImage &image);
    
    TImage convert256ColorImageToRGBAImage(const TImage &image);
    
    /**
     @brief Takes an input image and identifies if the image contains an actual image at the specified section.
     @param image The input image that is to be inspected.
     @param x The start x-axis position from which the pixmap will be inspected at.
     @param y The start y-axis position from which the pixmap will be inspected at.
     @param w The width of the pixmap section to inspect.
     @param h The height of the pixmap section to inspect.
     */
    bool containsImage(const TImage &image, uint16_t x, uint16_t y, uint16_t w, uint16_t h);
    
    /**
     @brief Takes an input image and identifies and extracts a section of the image that contains an actual image.
     @param image The input image from which a section containing an actual image will be extracted.
     */
    TImage extractImageSection(TImage &image);
    
    /**
     @brief Takes an input image and inverts it.
     @param image The input image that will to inverte.
     */
    void invertImage(TImage &image);
}
