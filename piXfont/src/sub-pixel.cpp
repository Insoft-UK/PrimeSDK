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

#include "sub-pixel.hpp"

static void applyOneThirdFilter(uint8_t *input, uint8_t *output, const int width)
{
    // Handle the first pixel separately (no left neighbor)
    *output++ = (*input + *input + *(input + 1)) / 3;
    input++;

    // Process all middle pixels
    for (int i = 1; i < width - 1; i++) {
        *output++ = (*(input - 1) + *input + *(input + 1)) / 3;
        input++;
    }

    // Handle the last pixel separately (no right neighbor)
    *output = (*(input - 1) + *input + *input) / 3;
}

static void applyOneNinthFilter(uint8_t *input, uint8_t *output, const int width)
{
    for (int i = 0; i < width; i++) {
        int sum = 0;
        for (int j = -1; j <= 1; j++) {   // Use 3-pixel neighborhood
            int idx = i + j;
            if (idx >= 0 && idx < width) {
                sum += input[idx];
            } else {
                sum += input[i];  // Edge case: replicate borders
            }
        }
        *output++ = sum / 3;  // Apply 1/9 filter
    }
}

void processSubPixels(uint8_t *input, uint32_t *output, const int width)
{
    uint8_t filteredOneThird[width];
    uint8_t filteredOneNinth[width];

    // 1/3 Filtering
    applyOneThirdFilter(input, filteredOneThird, width);

    // 1/9 Filtering
    applyOneNinthFilter(filteredOneThird, filteredOneNinth, width);
  
    uint8_t *filter = filteredOneNinth;

    // Convert filtered sub-pixels into final pixels
    for (int i = 0; i < width; i += 3) {
        uint8_t r = *filter++;
        uint8_t g = *filter++;
        uint8_t b = *filter++;

        // ARGB
        *output++ = (0xFF << 24) | (r << 16) | (g << 8) | b;
    }
}

