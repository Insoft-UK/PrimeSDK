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

#include <iostream>
#include <fstream>
#include <sstream>
#include <regex>
#include <cstring>
#include <iomanip>
#include <cstdint>
#include "../../PrimePlus/src/utf.hpp"

#include "../version_code.h"
#include "bmp.hpp"

#define NAME "GROB"
#define COMMAND_NAME "grob"

// MARK: - Functions

template <typename T>
T swap_endian(T u)
{
    static_assert (CHAR_BIT == 8, "CHAR_BIT != 8");

    union
    {
        T u;
        unsigned char u8[sizeof(T)];
    } source, dest;

    source.u = u;

    for (size_t k = 0; k < sizeof(T); k++)
        dest.u8[k] = source.u8[sizeof(T) - k - 1];

    return dest.u;
}

// A list is limited to 10,000 elements. Attempting to create a longer list will result in error 38 (Insufficient memory) being thrown.
static std::string ppl(const void *data, const size_t lengthInBytes, const int columns, bool le = true) {
    std::ostringstream os;
    uint64_t n;
    size_t count = 0;
    size_t length = lengthInBytes;
    uint64_t *bytes = (uint64_t *)data;
    
    while (length >= 8) {
        n = *bytes++;
        
        if (!le) {
            n = swap_endian<uint64_t>(n);
        }
        
#ifndef __LITTLE_ENDIAN__
        /*
         This platform utilizes big-endian, not little-endian. To ensure
         that data is processed correctly when generating the list, we
         must convert between big-endian and little-endian.
         */
        if (le) n = swap_endian<uint64_t>(n);
#endif

        if (count) os << ", ";
        if (count % columns == 0) {
            os << (count ? "\n    " : "    ");
        }
        os << "#" << std::uppercase << std::hex << std::setfill('0') << std::setw(16) << n << ":64h";
        
        count += 1;
        length -= 8;
    }
    return os.str();
}

static std::ifstream::pos_type filesize(const char* filename)
{
    std::ifstream in(filename, std::ifstream::ate | std::ifstream::binary);
    std::ifstream::pos_type pos = in.tellg();
    in.close();
    return pos;
}

static size_t loadBinaryFile(const char* filename, TBitmap &bitmap) {
    size_t fsize;
    std::ifstream infile;
    
    if ((fsize = filesize(filename)) == 0) return 0;
    
    infile.open(filename, std::ios::in | std::ios::binary);
    
    if (!infile.is_open()) return 0;

    bitmap.bytes.reserve(fsize);
    bitmap.bytes.resize(fsize);
    infile.read((char *)bitmap.bytes.data(), fsize);
    bitmap.bpp = 0;
    
    infile.close();
    return fsize;
}

std::string expandTilde(const std::string &path) {
    if (path.starts_with("~/")) {
        const char* home = getenv("HOME");
        if (home) {
            return std::string(home) + path.substr(1);  // Replace '~' with $HOME
        }
    }
    return path;
}


// MARK: - Command Line


void help(void)
{
    std::cout << "Copyright (C) 2024-" << YEAR << " Insoft. All rights reserved.\n";
    std::cout << "Insoft "<< NAME << " version, " << VERSION_NUMBER << " (BUILD " << VERSION_CODE << ")\n";
    std::cout << "\n";
    std::cout << "Usage: " << COMMAND_NAME << " <input-file> [-o <output-file>] [-c <columns>] [-n <name>] [-g<1-9>] [-ppl] \n";
    std::cout << "\n";
    std::cout << "Options:\n";
    std::cout << "  -o <output-file>           Specify the filename for generated PPL code.\n";
    std::cout << "  -c <columns>               Number of columns.\n";
    std::cout << "  -n <name>                  Custom name.\n";
    std::cout << "  -G<1-9>                    Graphic object G1-G9 to use if file is an image.\n";
    std::cout << "  --pragma                   Include \"#pragma mode( separator(.,;) integer(h64) )\" line.\n";
    std::cout << "  --endian <le|be>           Endianes le(default).\n";
    std::cout << "\n";
    std::cout << "Additional Commands:\n";
    std::cout << "  " << COMMAND_NAME << " {--version | --help}\n";
    std::cout << "    --version                Display the version information.\n";
    std::cout << "    --help                   Show this help message.\n";
}

void version(void) {
    std::cout << "Copyright (C) 2024 Insoft. All rights reserved.\n";
    std::cout << "Insoft "<< NAME << " version, " << VERSION_NUMBER << " (BUILD " << VERSION_CODE << ")\n";
    std::cout << "Built on: " << DATE << "\n";
    std::cout << "Licence: MIT License\n\n";
    std::cout << "For more information, visit: http://www.insoft.uk\n";
}

void error(void) {
    std::cout << COMMAND_NAME << ": try '" << COMMAND_NAME << " --help' for more information\n";
    exit(0);
}

void info(void) {
    std::cout << "Copyright (c) 2024 Insoft. All rights reserved.\n";
    std::cout << "Insoft "<< NAME << " version, " << VERSION_NUMBER << "\n\n";
}

// MARK: - Main

int main(int argc, const char * argv[]) {
    std::string in_filename, out_filename, prefix, sufix, name;
    int columns = 8;
    std::string grob("G0");
    bool le = true;
    
    std::string utf8;
    std::ostringstream os;

    if ( argc == 1 )
    {
        error();
        exit( 0 );
    }
   
    for( int n = 1; n < argc; n++ ) {
        std::string args = argv[n];
        
        if (args == "-o" || args == "--out") {
            if ( n + 1 >= argc ) {
                error();
                exit(100);
            }
            out_filename = argv[n + 1];
            out_filename = expandTilde(out_filename);
            if (std::filesystem::path(out_filename).extension().empty()) out_filename.append(".prgm");
    
            n++;
            continue;
        }
        
        if (args == "--help") {
            help();
            exit(0);
        }
        
        if (args == "--version") {
            version();
            exit(0);
            return 0;
        }
        
        if (args == "--pragma") {
            utf8.append("#pragma mode( separator(.,;) integer(h64) )\n\n");
            continue;
        }
        
        if (args == "--endian") {
            if ( n + 1 >= argc ) {
                info();
                exit(0);
            }
            
            n++;
            if (strcmp( argv[n], "le" ) == 0) le = true;
            if (strcmp( argv[n], "be" ) == 0) le = false;
        
            continue;
        }
        
        if (args.substr(0,2) == "-G") {
            grob = args.substr(1);
            continue;
        }
        
        if (args == "-c") {
            if ( n + 1 >= argc ) {
                info();
                exit(0);
            }
            
            n++;
            columns = atoi(argv[n]);
        
            continue;
        }
        
        if (args == "-n")
        {
            if ( n + 1 >= argc ) {
                error();
                exit(-1);
            }
            
            n++;
            name = argv[n];
        
            continue;
        }
        
        
        if (in_filename.empty()) in_filename = argv[n];
    }
    
    info();
    
    in_filename = expandTilde(in_filename);
    
    if (!std::filesystem::exists(in_filename)) {
        std::cout << "file '" << in_filename << "' not found.\n";
        return 0;
    }
    
    if (out_filename.empty()) {
        std::filesystem::path path = std::filesystem::path(in_filename);
        out_filename = path.parent_path().string() + "/" + path.stem().string() + ".prgm";
    }
    
    if (name.empty()) {
        std::smatch match;
        regex_search(out_filename, match, std::regex(R"(^.*[\/\\](.*)\.(.*)$)"));
        name = match[1].str();
        name = regex_replace(name, std::regex(R"([-.])"), "_");
    }
    
    size_t lengthInBytes = 0;
    TBitmap bitmap{};
    bitmap = loadBitmapImage(in_filename);
    if (bitmap.bytes.empty()) {
        lengthInBytes = loadBinaryFile(in_filename.c_str(), bitmap);
    } else {
        switch (bitmap.bpp) {
            case 1:
                lengthInBytes = bitmap.width * bitmap.height / 8;
                columns = bitmap.width / 64;
                bitmap.palette.resize(0);
                bitmap.palette.push_back(0xFFFFFFFF);
                bitmap.palette.push_back(0xFF);
                if (!bitmap.bytes.empty()) {
                    uint8_t *bytes = (uint8_t *)bitmap.bytes.data();
                    for (int i = 0; i < lengthInBytes; i += 1) {
                        uint8_t result = 0;
                        for (int n = 0; n < 8; n += 1) {
                            result <<= 1;
                            result |= bytes[i] & 1;
                            bytes[i] >>= 1;
                        }
                        bytes[i] = result;
                    }
                }
                break;
                
            case 4:
                lengthInBytes = bitmap.width * bitmap.height / 2;
                columns = bitmap.width / 16;
                if (!bitmap.bytes.empty() && le) {
                    /*
                     Due to the use of little-endian format, when the 8-byte sequence
                     is interpreted as a single 64-bit number, the bytes are stored in
                     reverse order (from least significant to most significant).
                     Since this data represents an image where each nibble corresponds
                     to an index, we must first swap the nibbles in the entire data
                     sequence to ensure they remain in the correct order when read from
                     right to left.
                     */
                    
                    uint8_t *bytes = (uint8_t *)bitmap.bytes.data();
                    for (int i = 0; i < lengthInBytes; i += 1) {
                        // Swap nibbles
                        bytes[i] = bytes[i] >> 4 | bytes[i] << 4;
                    }
                }
                break;
                
            case 8:
                lengthInBytes = bitmap.width * bitmap.height;
                columns = bitmap.width / 8;
                break;
                
            case 16:
                lengthInBytes = bitmap.width * bitmap.height * 2;
                break;
                
            case 32:
                lengthInBytes = bitmap.width * bitmap.height * 4;
                break;
                
            default:
                return -1;
                break;
        }
    }

    if (columns < 1) columns = 1;
    

    switch (bitmap.bpp) {
        case 0:
            utf8 += name + ":= {" + ppl(bitmap.bytes.data(), lengthInBytes, columns, le) + "};\n";
            break;
            
        case 1:
        case 4:
        case 8:
            os << name << " := {\n";
            os << "  {\n" << ppl(bitmap.bytes.data(), lengthInBytes, columns, le) << "\n  },\n";
            os << "  { " << std::dec << bitmap.width << ", " << bitmap.height << " },\n";
            
            os << "  {\n    ";
            for (int i = 0; i < bitmap.palette.size(); i += 1) {
                uint32_t color = bitmap.palette.at(i);
#ifdef __LITTLE_ENDIAN__
                color = swap_endian(color);
#endif
                color &= 0xFFFFFF;
                if (i) os << ", ";
                if (i % 16 == 0 && i) os << "\n    ";
                os << "#" << std::uppercase << std::hex << std::setfill('0') << std::setw(6) << color << ":32h";
            }
            os << "\n  }\n};\n";
            
            if (grob != "G0") os << "\nGROB.Image(" << grob << ", " << name << ");\n";
            utf8.append(os.str());
            break;
        
            
        default:
            os << name << " := {\n";
            os << "  {\n" << ppl(bitmap.bytes.data(), lengthInBytes, columns, le) << "\n  },\n";
            os << "  { " << std::dec << bitmap.width << ", " << bitmap.height << " };\n}\n";
            if (grob != "G0") os << "\nGROB.Image(" << grob << ", " << name << ");\n";
            utf8.append(os.str());
            break;
    }
    
    utf::save_as_utf16(out_filename, utf8);
    
    std::cout << "File '" << regex_replace(out_filename, std::regex(R"(.*/)"), "") << "' succefuly created.\n";
    
    return 0;
}
