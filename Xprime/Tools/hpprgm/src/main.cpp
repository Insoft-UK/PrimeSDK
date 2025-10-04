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
    << "  -o <output-file>   Specify the filename for generated .hpprgm or .prgm file."
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

// MARK: - Extensions

namespace std::filesystem {
    std::string expand_tilde(const std::string& path) {
        if (!path.empty() && path.starts_with("~")) {
#ifdef _WIN32
            const char* home = std::getenv("USERPROFILE");
#else
            const char* home = std::getenv("HOME");
#endif
            
            if (home) {
                return std::string(home) + path.substr(1);  // Replace '~' with $HOME
            }
        }
        return path;  // return as-is if no tilde or no HOME
    }
}

#if __cplusplus >= 202302L
    #include <bit>
    using std::byteswap;
#elif __cplusplus >= 201103L
    #include <cstdint>
    namespace std {
        template <typename T>
        T byteswap(T u)
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
    }
#else
    #error "C++11 or newer is required"
#endif

// MARK: - Other


bool isHPPrgrmFileFormat(std::ifstream &infile)
{
    uint32_t u32;
    infile.read((char *)&u32, sizeof(uint32_t));
    
#ifdef __LITTLE_ENDIAN__
    u32 = std::byteswap(u32);
#endif
    
    if (u32 != 0x7C618AB2) {
        goto invalid;
    }
    
    while (!infile.eof()) {
        infile.read((char *)&u32, sizeof(uint32_t));
#ifdef __LITTLE_ENDIAN__
    u32 = std::byteswap(u32);
#endif
        if (u32 == 0x9B00C000) return true;
        infile.peek();
    }
    
invalid:
    infile.seekg(std::ios::beg);
    return false;
}

void saveAsPrgm(const std::string& filepath, std::ifstream& infile) {
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
            return;
        }
    }
    
    std::ofstream outfile;
    outfile.open(filepath, std::ios::out | std::ios::binary);
    if(!outfile.is_open()) {
        infile.close();
        return;
    }
    
    uint16_t utf16le = 0xFFFE;
#ifdef __LITTLE_ENDIAN__
    utf16le = std::byteswap(utf16le);
#endif
    outfile.write((char *)&utf16le, 2);
    do {
        infile.read((char *)&utf16le, 2);
        if(!utf16le) break;
        outfile.write((char *)&utf16le, 2);
        infile.peek();
    } while (!infile.eof());
}

void saveAsG1Hpprgm(const std::string& filepath, std::ifstream& infile) {
    
    infile.seekg(0, std::ios::end);
    // Get the current file size.
    std::streampos codeSize = infile.tellg();
    infile.seekg(2, std::ios::beg);
    
    std::ofstream outfile;
    outfile.open(filepath, std::ios::out | std::ios::binary);
    if(!outfile.is_open()) {
        infile.close();
        return;
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
    for (int i = 0; i < 12; ++i) {
        outfile.put(0x00);
    }
    
    uint32_t size = (uint32_t)codeSize;
    outfile.write(reinterpret_cast<const char*>(&size), sizeof(size));
    
    /**
     0x0004-0x----: Code in UTF-16 LE until 00 00
     */
    std::vector<uint8_t> ppl;
    ppl.resize(codeSize);
    infile.read((char *)ppl.data(), codeSize);
    outfile.write((char *)ppl.data(), codeSize);
    
    outfile.put(0x00);
    outfile.put(0x00);
    
    outfile.close();
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
        
        in_filename = std::filesystem::expand_tilde(argv[n]);
    }
    
    if (std::filesystem::path(in_filename).parent_path().empty()) {
        in_filename.insert(0, "./");
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
     If the output file does not have an extension, a default .prgm or .hpprgm is applied.
     */
    if (std::filesystem::path(out_filename).extension().empty()) {
        out_filename.append(std::filesystem::path(in_filename).extension() == ".hpprgm" ? ".prgm" : ".hpprgm");
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
    
    if (!std::filesystem::exists(in_filename)) {
        std::cout << "Error: The specified input ‘" << std::filesystem::path(in_filename).filename() << "‘ file is invalid or not supported. Please ensure the file exists and has a valid format.\n";
    }
    
    std::ifstream infile;
    
    infile.open(in_filename, std::ios::in | std::ios::binary);
    if(!infile.is_open()) {
        std::cout << "Error: The specified input ‘" << std::filesystem::path(in_filename).filename() << "‘ file not found.\n";
        return 0;
    }
    
    if (std::filesystem::path(out_filename).extension() == ".prgm") {
        saveAsPrgm(out_filename, infile);
    }
    
    if (std::filesystem::path(out_filename).extension() == ".hpprgm") {
        saveAsG1Hpprgm(out_filename, infile);
    }
    
    if (std::filesystem::exists(out_filename)) {
        std::cout << "File " << std::filesystem::path(out_filename).filename() << " succefuly created.\n";
    }
    
    return 0;
}

