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

#include "version_code.h"
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
            os << (count ? "\n  " : "  ");
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


// MARK: - Command Line


void help(void)
{
    std::cout << "Copyright (C) 2024-" << YEAR << " Insoft. All rights reserved.\n";
    std::cout << "Insoft "<< NAME << " version, " << VERSION_NUMBER << " (BUILD " << VERSION_CODE << ")\n";
    std::cout << "\n";
    std::cout << "Usage: " << COMMAND_NAME << " <input-file> [-o <output-file>] [-c <columns>] [-n <name>] [-g <1…9>] [-ppl] \n";
    std::cout << "\n";
    std::cout << "Options:\n";
    std::cout << "  -o <output-file>           Specify the filename for generated PPL code.\n";
    std::cout << "  -c <columns>               Number of columns.\n";
    std::cout << "  -n <name>                  Custom name.\n";
    std::cout << "  -g <1…9>                   Graphic object 1-9 to use if file is an image.\n";
    std::cout << "  -ppl                       Wrap PPL code between #PPL...#END\n";
    std::cout << "  -endian <le|be>            Endianes le(default).\n";
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


void saveAs(std::string& filename, const std::string& str) {
    std::ofstream outfile;
    outfile.open(filename, std::ios::out | std::ios::binary);

    if(!outfile.is_open()) {
        error();
        exit(0x02);
    }
    
    bool utf16le = false;
    
    if (std::string::npos != filename.rfind(".hpprgm")) utf16le = true;
    
    if (utf16le) {
        outfile.put(0xFF);
        outfile.put(0xFE);
    }
    
    uint8_t* ptr = (uint8_t*)str.c_str();
    for ( int n = 0; n < str.length(); n++) {
        if (0xc2 == ptr[0]) {
            ptr++;
            continue;
        }
        
        if (utf16le) {
            if (0xE0 <= ptr[0]) {
                // 3 Bytes
                uint16_t utf16 = ptr[0];
                utf16 <<= 6;
                utf16 |= ptr[1] & 0b111111;
                utf16 <<= 6;
                utf16 |= ptr[1] & 0b111111;
                
#ifndef __LITTLE_ENDIAN__
                utf16 = utf16 >> 8 | utf16 << 8;
#endif
                outfile.write((const char *)&utf16, 2);
                
                ptr+=3;
                continue;
            }
        }
        
        if ('\r' == ptr[0]) {
            ptr++;
            continue;
        }

        // Output as UTF-16LE
        outfile.put(*ptr++);
        if (utf16le) outfile.put('\0');
    }
    
    outfile.close();
}


// MARK: - Main

int main(int argc, const char * argv[]) {
    std::string in_filename, out_filename, prefix, sufix, name;
    int columns = 8;
    int grob = 1;
    bool pplus = false;
    bool le = true;

    if ( argc == 1 )
    {
        error();
        exit( 0 );
    }
   
    for( int n = 1; n < argc; n++ ) {
        if ( strcmp( argv[n], "-o" ) == 0 || strcmp( argv[n], "--out" ) == 0 ) {
            if ( n + 1 >= argc ) {
                error();
                exit(100);
            }
            out_filename = argv[n + 1];
            if (std::string::npos == out_filename.rfind('.')) {
                out_filename += ".hpprgm";
            }

            n++;
            continue;
        }
        
        if ( strcmp( argv[n], "--help" ) == 0 ) {
            help();
            exit(0);
        }
        
        if ( strcmp( argv[n], "--version" ) == 0 ) {
            version();
            exit(0);
            return 0;
        }
        
        if ( strcmp( argv[n], "-ppl" ) == 0 ) {
            pplus = true;
            continue;
        }
        
        if ( strcmp( argv[n], "-endian" ) == 0 ) {
            if ( n + 1 >= argc ) {
                info();
                exit(0);
            }
            
            n++;
            if (strcmp( argv[n], "le" ) == 0) le = true;
            if (strcmp( argv[n], "be" ) == 0) le = false;
        
            continue;
        }
        
        if ( strcmp( argv[n], "-g" ) == 0 ) {
            if ( n + 1 >= argc ) {
                info();
                exit(0);
            }
            
            n++;
            grob = atoi(argv[n]);
        
            continue;
        }
        
        if ( strcmp( argv[n], "-c" ) == 0 ) {
            if ( n + 1 >= argc ) {
                info();
                exit(0);
            }
            
            n++;
            columns = atoi(argv[n]);
        
            continue;
        }
        
        if ( strcmp( argv[n], "-n" ) == 0 )
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
    
    if (out_filename.empty()) {
        out_filename = regex_replace(in_filename, std::regex(R"((\.\w+)?$)"), "");
        out_filename += ".hpprgm";
    }
    
    
    if (!filesize(in_filename.c_str())) {
        std::cout << "file '" << in_filename << "' not found.\n";
        exit(0x01);
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
    
    
    std::string utf8;
    std::ostringstream os;
    utf8.append("#pragma mode( separator(.,;) integer(h64) )\n\n");
    if (pplus) utf8.append("#PPL\n");
    
    
    switch (bitmap.bpp) {
        case 0:
            utf8 += "CONST " + name + ":= {" + ppl(bitmap.bytes.data(), lengthInBytes, columns, le) + "};\n";
            break;
            
        case 1:
        case 4:
        case 8:
            os << "CONST " << name << "_palt := {\n  ";
            
            for (int i = 0; i < bitmap.palette.size(); i += 1) {
                uint32_t color = bitmap.palette.at(i);
#ifdef __LITTLE_ENDIAN__
                color = swap_endian(color);
#endif
                color &= 0xFFFFFF;
                if (i) os << ", ";
                if (i % 16 == 0 && i) os << "\n  ";
                os << "#" << std::uppercase << std::hex << std::setfill('0') << std::setw(6) << color << ":64h";
            }
            os << "\n};\n\n";
            
            os << "CONST " << name << "_data := {\n" << ppl(bitmap.bytes.data(), lengthInBytes, columns, le) << "\n};\n\n";
            os << "GROB_P(G" << grob << ", " << std::dec << bitmap.width << ", " << bitmap.height << ", " << name << "_data, " << name << "_palt);\n";
            utf8.append(os.str());
            break;
        
            
        default:
            os << "CONST " << name << "_data := {\n" << ppl(bitmap.bytes.data(), lengthInBytes, columns, le) << "\n};\n\n";
            os << "DIMGROB_P(G" << grob << ", " << bitmap.width << ", " << bitmap.height << ", " << name << "_data);\n";
            utf8.append(os.str());
            break;
    }
    if (pplus) utf8.append("#END\n");
    
    saveAs(out_filename, utf8);
    
    std::cout << "UTF-16LE file '" << regex_replace(out_filename, std::regex(R"(.*/)"), "") << "' succefuly created.\n";
    
    return 0;
}
