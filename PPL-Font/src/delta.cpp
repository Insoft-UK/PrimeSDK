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

#include "delta.hpp"

std::vector<uint8_t> delta::encode(const std::vector<uint8_t> &bitStream)
{
    if (bitStream.empty()) return {};

    std::vector<uint8_t> encodedStream;
    encodedStream.push_back(bitStream[0]); // Store the first byte as-is

    for (size_t i = 1; i < bitStream.size(); i++) {
        encodedStream.push_back(bitStream[i] ^ bitStream[i - 1]); // XOR each byte with the previous one
    }

    return encodedStream;
}

std::vector<uint8_t> delta::decode(const std::vector<uint8_t> &encodedStream)
{
    if (encodedStream.empty()) return {};

    std::vector<uint8_t> decodedStream;
    decodedStream.push_back(encodedStream[0]); // Store the first byte as-is

    for (size_t i = 1; i < encodedStream.size(); i++) {
        decodedStream.push_back(decodedStream[i - 1] ^ encodedStream[i]); // Reverse XOR operation
    }

    return decodedStream;
}


