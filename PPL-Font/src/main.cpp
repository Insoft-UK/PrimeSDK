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
#include <sstream>
#include <vector>
#include <regex>
#include <fstream>
#include <iomanip>

#include "common.hpp"
#include "hpprgm.hpp"

#include "font.hpp"

using namespace cmn;

static bool verbose = false;

// TODO: Impliment "Indices of glyphs"

#include "../version_code.h"
#define NAME "Adafruit GFX Font Converter"
#define COMMAND_NAME "pplfont"

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
    std::cout << "Insoft "<< NAME << " version, " << VERSION_NUMBER << " (BUILD " << VERSION_CODE << ")\n";
    std::cout << "Copyright (C) " << YEAR << " Insoft. All rights reserved.\n";
    std::cout << "\n";
    std::cout << "Usage: " << COMMAND_NAME << " <input-file> [-o <output-file>] [-n <name>] [-v flags]\n";
    std::cout << "\n";
    std::cout << "Options:\n";
    std::cout << "  -o <output-file>   Specify the filename for generated .h or .hpprgm file.\n";
    std::cout << "  -n <name>          Font name.\n";
    std::cout << "  -v                 Enable verbose output for detailed processing information.\n";
    std::cout << "\n";
    std::cout << "Verbose Flags:\n";
    std::cout << "  f                  Font details.\n";
    std::cout << "  g                  Glyph details.\n";
    std::cout << "\n";
    std::cout << "Additional Commands:\n";
    std::cout << "  " << COMMAND_NAME << " {--version | --help}\n";
    std::cout << "    --version        Display version information.\n";
    std::cout << "    --help           Show this help message.\n";
}

// MARK: -




// MARK: - File IO

void createUTF8File(const std::string &filename, const std::string &str)
{
    std::ofstream outfile;
    
    outfile.open(filename, std::ios::out | std::ios::binary);
    if (outfile.is_open()) {
        outfile.write(str.c_str(), str.length());
        outfile.close();
    }
}

void createUTF16LEFile(const std::string& filename, const std::string str) {
    std::ofstream outfile;
    
    outfile.open(filename, std::ios::out | std::ios::binary);
    if(!outfile.is_open()) {
        error();
        exit(0x02);
    }
    
    outfile.put(0xFF);
    outfile.put(0xFE);
    
    uint8_t* ptr = (uint8_t*)str.c_str();
    for ( int n = 0; n < str.length(); n++) {
        if (0xc2 == ptr[0]) {
            ptr++;
            continue;
        }
        
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
        
        
        if ('\r' == ptr[0]) {
            ptr++;
            continue;
        }

        // Output as UTF-16LE
        outfile.put(*ptr++);
        outfile.put('\0');
    }
    
    outfile.close();
}

std::string loadTextFile(const std::string &filename)
{
    std::ifstream infile;
    std::string str;
    
    // Open the file in text mode
    infile.open(filename, std::ios::in);
    
    // Check if the file is successfully opened
    if (!infile.is_open()) {
        return str;
    }
    
    if (is_utf16le(infile)) {
        char c;
        while (!infile.eof()) {
            infile.get(c);
            str += c;
            infile.peek();
        }
        
        uint16_t *utf16_str = (uint16_t *)str.c_str();
        str = utf16_to_utf8(utf16_str, str.size() / 2);
    } else {
        // Use a stringstream to read the file's content into the string
        std::stringstream buffer;
        buffer << infile.rdbuf();
        str = buffer.str();
    }

    infile.close();
    
    return str;
}

// MARK: - Deleaing with Glyphs

void removeLeadingBlankGlyphs(font::TAdafruitFont &adafruitFont)
{
    /*
     Remove unnecessary blank glyph entries one by one from the beginning until
     a non-blank entry is found or the [space] (ASCII 32) glyph is reached, ensuring
     not to go past the last glyph, then stop.
    */
    for (int i = adafruitFont.first; i < adafruitFont.last && i < 32; i++) {
        // If width or height is zero, it's a blank entry.
        if (adafruitFont.glyphs.front().width) break;
        
        std::reverse(adafruitFont.glyphs.begin(), adafruitFont.glyphs.end());
        adafruitFont.glyphs.pop_back();
        std::reverse(adafruitFont.glyphs.begin(), adafruitFont.glyphs.end());
        adafruitFont.first++;
    }
}

void removeTrailingBlankGlyphs(font::TAdafruitFont &adafruitFont)
{
    /*
     Remove blank glyph entries from the end one by one until a non-blank entry
     is found or the first glyph is reached, then stop.
     */
    for (int i = adafruitFont.last; i > adafruitFont.first; i--) {
        // If width or height is zero, it's a blank entry.
        if (adafruitFont.glyphs.back().width) break;
        
        adafruitFont.glyphs.pop_back();
        adafruitFont.last--;
    }
}

void trimBlankGlyphs(font::TAdafruitFont &adafruitFont)
{
    removeLeadingBlankGlyphs(adafruitFont);
    removeTrailingBlankGlyphs(adafruitFont);
}




// MARK: - Decoding

bool parseHAdafruitFont(const std::string &filename, font::TAdafruitFont &font)
{
    std::ifstream infile;
    std::string utf8;
    
    utf8 = loadTextFile(filename);
    
    // Check if the file is successfully opened
    if (utf8.empty()) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return false;
    }
    
    std::smatch match;
    std::regex_search(utf8, match, std::regex(R"(const uint8_t \w+\[\] PROGMEM = \{([^}]*))"));
    if (match[1].matched) {
        auto s = match.str(1);
        while (std::regex_search(s, match, std::regex(R"((?:0x)?[\d[a-fA-F]{1,2})"))) {
            font.data.push_back(parse_number(match.str()));
            s = match.suffix().str();
        }
    } else {
        std::cout << "Failed to find <Bitmap Data>.\n";
        return false;
    }
    
    auto s = utf8;
    while (std::regex_search(s, match, std::regex(R"(\{ *((?:0x)?[\d[a-fA-F]+) *, *(-?[xb\da-fA-F]+) *, *(-?[xb\da-fA-F]+) *, *(-?[xb\da-fA-F]+) *, *(-?[xb\da-fA-F]+) *, *(-?[xb\da-fA-F]+) *\})"))) {
        font::TGlyph glyph;
        glyph.bitmapOffset = parse_number(match.str(1));
        glyph.width = parse_number(match.str(2));
        glyph.height = parse_number(match.str(3));
        glyph.xAdvance = parse_number(match.str(4));
        glyph.dX = parse_number(match.str(5));
        glyph.dY = parse_number(match.str(6));
        font.glyphs.push_back(glyph);
        s = match.suffix().str();
    }
    if (font.glyphs.empty()) {
        std::cout << "Failed to find <Glyph Table>.\n";
        return false;
    }
    
    if (std::regex_search(s, match, std::regex(R"(((?:0x)?[\da-fA-F]+)\s*,\s*((?:0x)?[\da-fA-F]+)\s*,\s*((?:0x)?[\da-fA-F]+)\s*\};)"))) {
        font.first = parse_number(match.str(1));
        font.last = parse_number(match.str(2));
        font.yAdvance = parse_number(match.str(3));
    } else {
        std::cout << "Failed to find <Font>.\n";
        return false;
    }
    
    return true;
}


// MARK: - Converting

void convertAdafruitFontToHpprgm(std::string &in_filename, std::string &out_filename, std::string &name)
{
    font::TAdafruitFont adafruitFont;
    std::string str;
    
    if (!parseHAdafruitFont(in_filename, adafruitFont)) {
        std::cout << "Failed to find valid Adafruit Font data.\n";
        exit(2);
    }

    str = hpprgm::buildHpprgmAdafruitFont(adafruitFont, name);
    createUTF16LEFile(out_filename, str);
}





// MARK: - Main

int main(int argc, const char * argv[])
{
    if ( argc == 1 ) {
        error();
        return 0;
    }
    
    std::string args(argv[0]);

    
    std::string in_filename, out_filename, name, prefix, sufix;
    int columns = 16;
    
    
    bool fixed = false;
    bool leftAlign = false;
    
    font::TAdafruitFont adafruitFont = {
        .first = 0,
        .last = 255,
        .yAdvance = 8
    };
    
    for( int n = 1; n < argc; n++ ) {
        if (*argv[n] == '-') {
            std::string args(argv[n]);
            
            if (args == "-o") {
                if (++n > argc) error();
                out_filename = argv[n];
                continue;
            }
            
            
            
            if (args == "-n") {
                if (++n > argc) error();
                name = argv[n];
                continue;
            }
            
            
            
            
            if (args == "-c") {
                if (++n > argc) error();
                columns = parse_number(argv[n]);
                if (columns < 1) columns = 1;
                continue;
            }
            
            if (args == "-f") {
                if (++n > argc) error();
                adafruitFont.first = parse_number(argv[n]);
                if (adafruitFont.first < 0 || adafruitFont.first > 255) adafruitFont.first = 0;
                continue;
            }
            
            if (args == "-l") {
                if (++n > argc) error();
                adafruitFont.last = parse_number(argv[n]);
                if (adafruitFont.last < 0 || adafruitFont.last > 255) adafruitFont.last = 255;
                continue;
            }
            
            
            
            if (args == "-F") {
                fixed = true;
                continue;
            }
            
            
            
            if (args == "-a") {
                leftAlign = true;
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
        
        if (!in_filename.empty()) error();
        in_filename = argv[n];
    }
    
    
    if (name.empty()) {
        name = std::filesystem::path(in_filename).stem().string();
    }
    
    /*
     We ensure that if an invalid ‘last’ or ‘first’ entry is provided, we
     revert to the default values for ‘last’ and ‘first’.
     */
    if (adafruitFont.last < adafruitFont.first) {
        adafruitFont.first = 0;
        adafruitFont.last = 255;
    }
    
    /*
     Initially, we display the command-line application’s basic information,
     including its name, version, and copyright details.
     */
    info();
    
    /*
     If no output file is specified, the program will use the input file’s name
     (excluding its extension) as the output file name.
     */
    if (out_filename.empty() || out_filename == in_filename) {
        out_filename = std::filesystem::path(in_filename).parent_path();
        out_filename.append("/");
        out_filename.append(std::filesystem::path(in_filename).stem().string());
    }
    
    std::string in_extension = std::filesystem::path(in_filename).extension();
    
    /*
     If the output file does not have an extension, a default is applied based on
     the input file’s extension:
     
     • For an input file with a .h extension, the default output extension is .hpprgm.
     • For an input file with a .hpprgm extension, the default output extension is .h.
     */
    if (std::filesystem::path(out_filename).extension().empty()) {
        if (in_extension == ".h") out_filename.append(".hpprgm");
        if (in_extension == ".hpprgm") out_filename.append(".h");
    }
    
    std::string out_extension = std::filesystem::path(out_filename).extension();
    
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
    
    /*
     If the input file is a .h file, the output must be either .hpprgm or .bmp;
     otherwise, the process is halted and an error is reported to the user.
     */
    if (in_extension == ".h") {
        if (out_extension == ".hpprgm") {
            convertAdafruitFontToHpprgm(in_filename, out_filename, name);
            std::cout << "Adafruit GFX Font for HP Prime " << std::filesystem::path(out_filename).filename() << " has been succefuly created.\n";
            return 0;
        }
        
        std::cout << "Error: For ‘." << in_extension << "’ input file, the output file must have a ‘.hpprgm’ extension. Please choose a valid output file type.\n";
        return 0;
    }
    
    
    
    std::cout << "Error: The specified input ‘" << std::filesystem::path(in_filename).filename() << "‘ file is invalid or not supported. Please ensure the file exists and has a valid format.\n";
    
    return 0;
}

