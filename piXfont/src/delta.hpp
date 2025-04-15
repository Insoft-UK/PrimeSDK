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

#ifndef delta_hpp
#define delta_hpp

#include <iostream>
#include <cstdint>
#include <vector>

namespace delta {
    /**
     @brief    Performs delta encoding on a bit stream.
     @param    bitStream The input bit stream represented as a vector of 8-bit values.
     @return   A vector of 8-bit values containing the delta-encoded data.
     */
    std::vector<uint8_t> encode(const std::vector<uint8_t> &bitStream);
    
    /**
     @brief    Decodes a delta-encoded bit stream.
     @param    encodedStream The input delta-encoded bit stream represented as a vector of 8-bit values.
     @return   A vector of 8-bit values containing the decoded original data.
     */
    std::vector<uint8_t> decode(const std::vector<uint8_t> &encodedStream);
}

#endif /* delta_hpp */
