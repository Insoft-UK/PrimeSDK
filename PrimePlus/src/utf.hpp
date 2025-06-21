// The MIT License (MIT)
//
// Copyright (c) 2023-2025 Insoft. All rights reserved.
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

#ifndef utf_hpp
#define utf_hpp

#include <sstream>
#include <fstream>
#include <cstdlib>

namespace utf {
    std::string to_utf8(const std::wstring& wstr);
    std::wstring to_utf16(const std::string& str);
    
    uint16_t utf16(const char *str);
    
    std::wstring read_as_utf16(std::ifstream& is);
    std::wstring read_utf16(std::ifstream& is);
    std::wstring load_utf16(const std::string& filepath);
    
    size_t write_utf8(std::ofstream& os, const std::string& str);
    bool save_as_utf8(const std::string& filepath, const std::string& str);
    
    size_t write_as_utf16(std::ofstream& os, const std::string& str);
    size_t write_utf16(std::ofstream& os, const std::string& str);
    bool save_as_utf16(const std::string& filepath, const std::string& str);
};

#endif /* utf_hpp */
