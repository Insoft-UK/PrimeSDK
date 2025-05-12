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


typedef struct {
    uint16_t   bitmapOffset;    // Offset address into the bitmap data.
    uint8_t    width, height;   // Bitmap dimensions in pixels.
    uint8_t    xAdvance;        // Distance to advance cursor in the x-axis.
    int8_t     dX;              // Used to position the glyph within the cell in the horizontal direction.
    int8_t     dY;              // Distance from the baseline of the character to the top of the glyph.
} TGlyph;

typedef struct {
    uint8_t   *bitmap;          // Glyph bitmaps, concatenated.
    TGlyph    *glyph;           // Glyph array.
    uint8_t   first;           // The first UTF16 value of your first character.
    uint8_t   last;            // The last UTF16 value of your last character.
    uint8_t    yAdvance;        // Newline distance in the y-axis.
} TFont;

/*
 The UTF8 table stores the index of each glyph within the glyph table, indicating where the glyph entry
 for a particular UTF16 character resides. The table does not include entries for the first and last glyph
 indices, as they are unnecessary: the first glyph index always corresponds to the first glyph entry, and
 the last glyph in the font is always the last glyph entry.
 */

typedef struct {
    std::vector<uint8_t> data;
    std::vector<TGlyph> glyphs;
    uint8_t first;
    uint8_t last;
    uint8_t yAdvance;
} TAdafruitFont;

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
    std::cout << "  -o <output-file>   Specify the filename for generated .hpprgm file.\n";
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

static void createUTF16LEFile(const std::string& filename, const std::string str) {
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

static std::string loadUTF8File(const std::string &filename)
{
    std::ifstream infile;
    std::string str;
    
    // Open the file in text mode
    infile.open(filename, std::ios::in);
    
    // Check if the file is successfully opened
    if (!infile.is_open()) {
        return str;
    }
    
    std::stringstream buffer;
    buffer << infile.rdbuf();
    str = buffer.str();

    infile.close();
    
    return str;
}


static int parseNumber(const std::string &str)
{
    std::regex hexPattern("^0x[\\da-fA-F]+$");
    std::regex decPattern("^[+-]?\\d+$");
    std::regex octPattern("^0[0-8]+$");
    std::regex binPattern("^0b[01]+$");
    
    if (std::regex_match(str, hexPattern)) return std::stoi(str, nullptr, 16);
    if (std::regex_match(str, decPattern)) return std::stoi(str, nullptr, 10);
    if (std::regex_match(str, octPattern)) return std::stoi(str, nullptr, 8);
    if (std::regex_match(str, binPattern)) return std::stoi(str, nullptr, 2);
    
    return 0;
}

static bool parseHAdafruitFont(const std::string &filename, TAdafruitFont &font)
{
    std::ifstream infile;
    std::string utf8;
    
    utf8 = loadUTF8File(filename);
    
    // Check if the file is successfully opened
    if (utf8.empty()) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return false;
    }
    
    std::smatch match;
    std::regex_search(utf8, match, std::regex(R"(const (?:unsigned char|uint8_t) \w+\[\] PROGMEM = \{([^}]*))"));
    if (match[1].matched) {
        auto s = match.str(1);
        while (std::regex_search(s, match, std::regex(R"((?:0x)?[\d[a-fA-F]{1,2})"))) {
            font.data.push_back(parseNumber(match.str()));
            s = match.suffix().str();
        }
    } else {
        std::cout << "Failed to find <Bitmap Data>.\n";
        return false;
    }
    
    auto s = utf8;
    while (std::regex_search(s, match, std::regex(R"(\{ *((?:0x)?[\d[a-fA-F]+) *, *(-?[xb\da-fA-F]+) *, *(-?[xb\da-fA-F]+) *, *(-?[xb\da-fA-F]+) *, *(-?[xb\da-fA-F]+) *, *(-?[xb\da-fA-F]+) *\})"))) {
        TGlyph glyph;
        glyph.bitmapOffset = parseNumber(match.str(1));
        glyph.width = parseNumber(match.str(2));
        glyph.height = parseNumber(match.str(3));
        glyph.xAdvance = parseNumber(match.str(4));
        glyph.dX = parseNumber(match.str(5));
        glyph.dY = parseNumber(match.str(6));
        font.glyphs.push_back(glyph);
        s = match.suffix().str();
    }
    if (font.glyphs.empty()) {
        std::cout << "Failed to find <Glyph Table>.\n";
        return false;
    }
    
    if (std::regex_search(s, match, std::regex(R"(((?:0x)?[\da-fA-F]+)\s*,\s*((?:0x)?[\da-fA-F]+)\s*,\s*((?:0x)?[\da-fA-F]+)\s*\};)"))) {
        font.first = parseNumber(match.str(1));
        font.last = parseNumber(match.str(2));
        font.yAdvance = parseNumber(match.str(3));
    } else {
        std::cout << "Failed to find <Font>.\n";
        return false;
    }
    
    return true;
}

static uint8_t mirrorByte(uint8_t b)
{
    b = ((b & 0xF0) >> 4) | ((b & 0x0F) << 4);  // Swap upper and lower 4 bits
    b = ((b & 0xCC) >> 2) | ((b & 0x33) << 2);  // Swap pairs of bits
    b = ((b & 0xAA) >> 1) | ((b & 0x55) << 1);  // Swap individual bits
    return b;
}

// A list is limited to 10,000 elements. Attempting to create a longer list will result in error 38 (Insufficient memory) being thrown.
static std::string createPPLList(const void *data, const size_t lengthInBytes, const int columns, bool le = true) {
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
    
    if (length) {
        n = *bytes++;
        
        if (!le) {
            n = swap_endian<uint64_t>(n);
        }
        
        os << " ,";
        if (count % columns == 0) {
            os << (count ? "\n  " : "  ");
        }
        
        // TODO: improve this code for readability.
        os << "#" << std::uppercase << std::hex << std::setfill('0') << std::setw(16) << (n & (0xFFFFFFFFFFFFFFFF >> ((8-length) * 8))) << ":64h";
    }
    
    return os.str();
}

static std::string createHpprgmAdafruitFont(TAdafruitFont &adafruitFont, std::string &name)
{
    std::ostringstream os;
    
    for (auto it = adafruitFont.data.begin(); it < adafruitFont.data.end(); it++) {
        *it = mirrorByte(*it);
    }
    
    os << "#pragma mode( separator(.,;) integer(h64) )\n\n"
       << "// Generated by Insoft Adafruit GFX Font Converter version, " << VERSION_NUMBER << "\n"
       << "// Copyright (C) 2024-" << YEAR << " Insoft. All rights reserved.\n\n"
       << "EXPORT " << name << " := {\n"
       << " {\n" << createPPLList(adafruitFont.data.data(), adafruitFont.data.size(), 16) << "\n"
       << " },{\n"
       << createPPLList(adafruitFont.glyphs.data(), adafruitFont.glyphs.size() * sizeof(TGlyph), 16) << "\n"
       << " }, " << (int)adafruitFont.first << ", " << (int)adafruitFont.last << ", " << (int)adafruitFont.yAdvance << "\n"
       << "};";
    
    return os.str();
}

static void convertAdafruitFontToHpprgm(std::string &in_filename, std::string &out_filename, std::string &name)
{
    TAdafruitFont adafruitFont;
    std::string str;
    
    if (!parseHAdafruitFont(in_filename, adafruitFont)) {
        std::cout << "Failed to find valid Adafruit Font data.\n";
        exit(2);
    }

    str = createHpprgmAdafruitFont(adafruitFont, name);
    createUTF16LEFile(out_filename, str);
}



// MARK: - Main

int main(int argc, const char **argv)
{
    if (argc == 1) {
        error();
        return 0;
    }
    
    std::string args(argv[0]);
    std::string in_filename, out_filename, name, prefix, sufix;

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
    
    
    if (name.empty()) {
        name = std::filesystem::path(in_filename).stem().string();
    }
    
    

    /*
     Initially, we display the command-line application’s basic information,
     including its name, version, and copyright details.
     */
    info();
    
    /*
     If the input file does not have an extension, default is .h is applied.
     */
    if (std::filesystem::path(in_filename).extension().empty()) {
        in_filename.append(".h");
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
    
    std::string in_extension = std::filesystem::path(in_filename).extension();
    
    /*
     If the output file does not have an extension, a default is applied based on
     the input file’s extension:
     
     • For an input file with a .h extension, the default output extension is .hpprgm.
     • For an input file with a .hpprgm extension, the default output extension is .h.
     */
    if (std::filesystem::path(out_filename).extension().empty()) {
        out_filename.append(".hpprgm");
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

