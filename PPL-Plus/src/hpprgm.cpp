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

#include "hpprgm.hpp"

static std::wstring read_wstring_until_null(std::ifstream &is, std::streampos offset = 0) {
    // Seek to the desired offset (if any)
    if (offset > 0) {
        is.seekg(offset, std::ios::beg);
    }

    std::wstring result;
    while (true) {
        char16_t ch;
        // Read 2 bytes (UTF-16LE)
        is.read(reinterpret_cast<char*>(&ch), sizeof(ch));

        if (!is || ch == 0x0000) {
            break; // EOF or null terminator
        }

        result += static_cast<wchar_t>(ch);
        is.peek();
        if (is.eof()) break;
    }

    return result;
}

static std::wstring utf16_code(const std::string &s)
{
    std::wstring wstr;
    std::ifstream is;
    
    is.open(s, std::ios::in | std::ios::binary);
    wstr = read_wstring_until_null(is, 2);
    is.close();
    return wstr;
}

static std::wstring g1_code(const std::string &s)
{
    uint32_t u32;
    std::streampos pos, codePos;
    std::wstring wstr;
    std::ifstream is;
    
    is.open(s, std::ios::in | std::ios::binary);
    is.read(reinterpret_cast<char*>(&u32), sizeof(u32));
    
    is.seekg(u32, std::ios::cur);
    is.read(reinterpret_cast<char*>(&u32), sizeof(u32));
    
    codePos = is.tellg();
    is.seekg(u32, std::ios::cur);
    pos = is.tellg();
    
    is.seekg(0, std::ios::end);
    if (is.tellg() != pos) {
        is.close();
    }
    
    wstr = read_wstring_until_null(is, codePos);
    return wstr;
}

static std::wstring g2_code(const std::string &s)
{
    std::wstring wstr;
    std::ifstream is;
    uint32_t u32;
    
    is.open(s, std::ios::in | std::ios::binary);
    
    while (!is.eof()) {
        is.read((char *)&u32, sizeof(uint32_t));
        if (u32 == 0x00C0009B) break;;
        is.peek();
    }
    
    wstr = read_wstring_until_null(is, is.tellg());
    is.close();
    
    return wstr;
}

bool hpprgm::is_g1(const std::string &s)
{
    std::ifstream is;
    uint32_t header_size, code_size;
    std::filesystem::path path;
    
    path = std::filesystem::path(s);
    if (!std::filesystem::exists(path)) return false;
    auto filesize = std::filesystem::file_size(path);
    
    is.open(s, std::ios::in | std::ios::binary);
    is.read(reinterpret_cast<char*>(&header_size), sizeof(header_size));
    if (filesize < header_size + 4) {
        is.close();
        return false;
    }
    is.seekg(header_size, std::ios::cur);
    is.read(reinterpret_cast<char*>(&code_size), sizeof(code_size));
    is.close();
    
    return filesize == 4 + header_size + 4 + code_size;
}

bool hpprgm::is_g2(const std::string &s)
{
    std::ifstream is;
    uint32_t sig;
    
    is.open(s, std::ios::in | std::ios::binary);
    is.read(reinterpret_cast<char*>(&sig), sizeof(sig));
    is.close();
    
    return sig == 0xB28A617C;
}

bool hpprgm::is_utf16le(const std::string &s)
{
    std::ifstream is;
    uint16_t sig;
    
    is.open(s, std::ios::in | std::ios::binary);
    is.read(reinterpret_cast<char*>(&sig), sizeof(sig));
    is.close();
    
    return sig == 0xFEFF;
}

std::wstring hpprgm::load(const std::string &s)
{
    std::ifstream is;
    
    if (!std::filesystem::exists(s)) return std::wstring();
    is.open(s, std::ios::in | std::ios::binary);
    
    if (is_utf16le(s)) return utf16_code(s);
    if (is_g2(s)) return g2_code(s);
    if (is_g1(s)) return g1_code(s);
    
    return std::wstring();
    
}

