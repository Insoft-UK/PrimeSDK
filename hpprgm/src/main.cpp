// The MIT License (MIT)
//
// Copyright (c) 2025 Insoft. All rights reserved.
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
#include <sstream>
#include <vector>
#include <regex>
#include <fstream>
#include <iomanip>


static bool verbose = false;

// TODO: Impliment "Indices of glyphs"

#include "../version_code.h"
#define NAME "HP Prime Program Tool"
#define COMMAND_NAME "hpprgm"

// MARK: - Command Line
void version(void) {
    std::cout << "Insoft "<< NAME << " version, " << VERSION_NUMBER << " (BUILD " << VERSION_CODE << ")\n";
    std::cout << "Copyright (C) " << YEAR << " Insoft. All rights reserved.\n";
    std::cout << "Built on: " << DATE << "\n";
    std::cout << "Licence: MIT License\n\n";
    std::cout << "For more information, visit: http://www.insoft.uk\n";
}

void error(void) {
    std::cout << COMMAND_NAME << ": try '" << COMMAND_NAME << " --help' for more information\n";
    exit(0);
}

void info(void) {
    std::cout << "Insoft "<< NAME << " version, " << VERSION_NUMBER << "\n";
    std::cout << "Copyright (C) " << YEAR << " Insoft. All rights reserved.\n\n";
}

void help(void) {
    std::cout
    << "Insoft "<< NAME << " version, " << VERSION_NUMBER << " (BUILD " << VERSION_CODE << ")"
    << "Copyright (C) " << YEAR << " Insoft. All rights reserved."
    << ""
    << "Usage: " << COMMAND_NAME << " <input-file> [-o <output-file>] [-v flags]"
    << ""
    << "Options:"
    << "  -o <output-file>   Specify the filename for generated .hpprgm file."
    << "  -v                 Enable verbose output for detailed processing information."
    << ""
    << "Verbose Flags:"
    << "  s                  Size of extracted PPL code in bytes."
    << ""
    << "Additional Commands:"
    << "  " << COMMAND_NAME << " {--version | --help}"
    << "    --version        Display version information."
    << "    --help           Show this help message.";
}

// MARK: -


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


bool isHPPrgrmFileFormat(std::ifstream &infile)
{
    uint32_t u32;
    infile.read((char *)&u32, sizeof(uint32_t));
    
#ifdef __LITTLE_ENDIAN__
    u32 = swap_endian(u32);
#endif
    
    if (u32 != 0x7C618AB2) {
        goto invalid;
    }
    
    while (!infile.eof()) {
        infile.read((char *)&u32, sizeof(uint32_t));
#ifdef __LITTLE_ENDIAN__
    u32 = swap_endian(u32);
#endif
        if (u32 == 0x9B00C000) return true;
        infile.peek();
    }
    
invalid:
    infile.seekg(std::ios::beg);
    return false;
}

// MARK: - Main

int main(int argc, const char **argv)
{
    if (argc == 1) {
        error();
        return 0;
    }
    
    std::string args(argv[0]);
    std::string in_filename, out_filename;

    for( int n = 1; n < argc; n++ ) {
        if (*argv[n] == '-') {
            std::string args(argv[n]);
            
            if (args == "-o") {
                if (++n > argc) error();
                out_filename = argv[n];
                continue;
            }

            if (args == "--help") {
                help();
                return 0;
            }
            
            if (args == "--version") {
                version();
                return 0;
            }
            
            if (args == "-v") {
                if (++n > argc) error();
                args = argv[n];
                verbose = true;
                continue;
            }
            
            error();
            return 0;
        }
        
        in_filename = argv[n];
    }
    

    /*
     Initially, we display the command-line application’s basic information,
     including its name, version, and copyright details.
     */
    info();
    
    /*
     If the input file does not have an extension, default is .hpprgm is applied.
     */
    if (std::filesystem::path(in_filename).extension().empty()) {
        in_filename.append(".hpprgm");
    }
    
    /*
     If no output file is specified, the program will use the input file’s name
     (excluding its extension) as the output file name.
     */
    if (out_filename.empty() || out_filename == in_filename) {
        out_filename = std::filesystem::path(in_filename).parent_path();
        out_filename.append("/");
        out_filename.append(std::filesystem::path(in_filename).stem().string());
    }
    
    /*
     If the output file does not have an extension, a default .ppl is applied.
     */
    if (std::filesystem::path(out_filename).extension().empty()) {
        out_filename.append(".ppl");
    }

    /*
     We need to ensure that the specified output filename includes a path.
     If no path is provided, we prepend the path from the input file.
     */
    if (std::filesystem::path(out_filename).parent_path().empty()) {
        out_filename.insert(0, "/");
        out_filename.insert(0, std::filesystem::path(in_filename).parent_path());
    }
    
    /*
     The output file must differ from the input file. If they are the same, the
     process will be halted and an error message returned to the user.
     */
    if (in_filename == out_filename) {
        std::cout << "Error: The output file must differ from the input file. Please specify a different output file name.\n";
        return 0;
    }
    
    if (std::filesystem::exists(in_filename)) {
        std::ifstream infile;
        
        infile.open(in_filename, std::ios::in | std::ios::binary);
        if(!infile.is_open()) return 0;
        
        if (!isHPPrgrmFileFormat(infile)) {
            // G1 .hpprgm file format.
            uint32_t offset;
            infile.read(reinterpret_cast<char*>(&offset), sizeof(offset));
#ifndef __LITTLE_ENDIAN__
            offset = swap_endian(offset);
#endif
            offset += 8;
            infile.seekg(offset, std::ios::beg);

            if (!infile.good()) {
                std::cerr << "Seek failed (possibly past EOF or bad stream).\n";
                infile.close();
                return 0;
            }
        }
        
        std::ofstream outfile;
        outfile.open(out_filename, std::ios::out | std::ios::binary);
        if(!outfile.is_open()) {
            infile.close();
            return 0;
        }
        
        uint16_t utf16le = 0xFFFE;
#ifdef __LITTLE_ENDIAN__
        utf16le = swap_endian(utf16le);
#endif
        outfile.write((char *)&utf16le, 2);
        do {
            infile.read((char *)&utf16le, 2);
            if(!utf16le) break;
            outfile.write((char *)&utf16le, 2);
            infile.peek();
        } while (!infile.eof());
          
        infile.close();
        outfile.close();
        return 0;
    }

    
    std::cout << "Error: The specified input ‘" << std::filesystem::path(in_filename).filename() << "‘ file is invalid or not supported. Please ensure the file exists and has a valid format.\n";
    
    return 0;
}

