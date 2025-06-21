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
#include "utf.hpp"

static std::wstring extractPPLCode(const std::string& s) {
    uint32_t u32;
    std::streampos pos, codePos;
    std::wstring wstr;
    std::ifstream is;
    
    is.open(s, std::ios::in | std::ios::binary);
    
    while (!is.eof()) {
        is.read((char *)&u32, sizeof(uint32_t));
        if (u32 == 0x00C0009B) {
            is.seekg(is.tellg(), std::ios::beg);
            wstr = utf::read_as_utf16(is);
            is.close();
            
            return wstr;
        }
        is.peek();
    }
    
    is.seekg(0, std::ios::beg);
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
    
    is.seekg(codePos, std::ios::beg);
    wstr = utf::read_as_utf16(is);
    return wstr;
}


static bool isG1(const std::string& filepath) {
    std::ifstream is;
    uint32_t header_size, code_size;
    std::filesystem::path path;
    
    path = std::filesystem::path(filepath);
    if (!std::filesystem::exists(path)) return false;
    auto filesize = std::filesystem::file_size(path);
    
    is.open(filepath, std::ios::in | std::ios::binary);
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

static bool isG2(const std::string& filepath) {
    std::ifstream is;
    uint32_t sig;
    
    is.open(filepath, std::ios::in | std::ios::binary);
    is.read(reinterpret_cast<char*>(&sig), sizeof(sig));
    is.close();
    
    return sig == 0xB28A617C;
}

std::wstring hpprgm::load(const std::string& filepath) {
    std::wstring wstr;
    
    if (!std::filesystem::exists(filepath)) return wstr;
    
    if (std::filesystem::path(filepath).extension() != ".hpprgm") wstr = utf::load_utf16(filepath);
    if (std::filesystem::path(filepath).extension() == ".hpprgm") {
        if (isG2(filepath) || isG1(filepath)) wstr = extractPPLCode(filepath);
    }
    return wstr;
}


bool hpprgm::save(const std::string& filepath, const std::string& str) {
    
    std::ofstream outfile;
    outfile.open(filepath, std::ios::out | std::ios::binary);
    if(!outfile.is_open()) {
        return false;
    }
    
    // HEADER
    /**
     0x0000-0x0003: Header Size, excludes itself (so the header begins at offset 4)
     */
    outfile.put(0x0C); // 12
    outfile.put(0x00);
    outfile.put(0x00);
    outfile.put(0x00);
    
    // Write the 12-byte UTF-16LE header.
    /**
     0x0004-0x0005: Number of variables in table.
     0x0006-0x0007: Number of uknown?
     0x0008-0x0009: Number of exported functions in table.
     0x000A-0x000F: Conn. kit generates 7F 01 00 00 00 00 but all zeros seems to work too.
     */
    for (int i = 0; i < 12 + 4; ++i) {
        outfile.put(0x00);
    }
    
    /**
     0x0004-0x----: Code in UTF-16 LE until 00 00
     */
    uint32_t size = (uint32_t)utf::write_as_utf16(outfile, str);
    
    outfile.put(0x00);
    outfile.put(0x00);
    
    outfile.seekp(16, std::ios::beg);
    outfile.write(reinterpret_cast<const char*>(&size), sizeof(size));
    outfile.seekp(std::ios::end);
    
    outfile.close();
    return true;
}

