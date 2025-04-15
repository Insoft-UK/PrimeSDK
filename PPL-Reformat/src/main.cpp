/*
 The MIT License (MIT)
 
 Copyright (c) 2024 Insoft. All rights reserved.
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <regex>
#include <cstring>
#include <iomanip>

#include <sys/time.h>

#include "singleton.hpp"
#include "common.hpp"

#include "preprocessor.hpp"
#include "strings.hpp"

#include "build.h"
#include "timer.hpp"

using namespace ppl;

static Preprocessor preprocessor = Preprocessor();
static Strings strings = Strings();



void terminator() {
    std::cout << MessageType::Error << "An internal preprocessing problem occurred. Please review the syntax before this point.\n";
    exit(-1);
}

void (*old_terminate)() = std::set_terminate(terminator);



// MARK: - Utills



uint32_t utf8_to_utf16(const char* utf8) {
    uint8_t *utf8_char = (uint8_t *)utf8;
    uint16_t utf16_char = *utf8_char;
    
    if ((utf8_char[0] & 0b11110000) == 0b11100000) {
        utf16_char = utf8_char[0] & 0b11111;
        utf16_char <<= 6;
        utf16_char |= utf8_char[1] & 0b111111;
        utf16_char <<= 6;
        utf16_char |= utf8_char[2] & 0b111111;
        return utf16_char;
    }
    
    // 110xxxxx 10xxxxxx
    if ((utf8_char[0] & 0b11100000) == 0b11000000) {
        utf16_char = utf8_char[0] & 0b11111;
        utf16_char <<= 6;
        utf16_char |= utf8_char[1] & 0b111111;
        return utf16_char;
    }
    
    return utf16_char;
}

std::string utf16_to_utf8(const uint16_t* utf16_str, size_t utf16_size) {
    std::string utf8_str;
    
    for (size_t i = 0; i < utf16_size; ++i) {
        uint16_t utf16_char = utf16_str[i];
        
#ifndef __LITTLE_ENDIAN__
        utf16_char = utf16_char >> 8 | utf16_char << 8;
#endif

        if (utf16_char < 0x0080) {
            // 1-byte UTF-8
            utf8_str += static_cast<char>(utf16_char);
        }
        else if (utf16_char < 0x0800) {
            // 2-byte UTF-8
            utf8_str += static_cast<char>(0xC0 | ((utf16_char >> 6) & 0x1F));
            utf8_str += static_cast<char>(0x80 | (utf16_char & 0x3F));
        }
        else {
            // 3-byte UTF-8
            utf8_str += static_cast<char>(0xE0 | ((utf16_char >> 12) & 0x0F));
            utf8_str += static_cast<char>(0x80 | ((utf16_char >> 6) & 0x3F));
            utf8_str += static_cast<char>(0x80 | (utf16_char & 0x3F));
        }
    }
    
    return utf8_str;
}

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

// TODO: .hpprgrm file format detection and handling.
bool isHPPrgrmFileFormat(std::ifstream& infile)
{
    uint32_t u32;
    infile.read((char *)&u32, sizeof(uint32_t));
    
#ifndef __LITTLE_ENDIAN__
    u32 = swap_endian(u32);
#endif
    
    if (u32 != 0x7C618AB2) {
        goto invalid;
    }
    
    while (!infile.eof()) {
        infile.read((char *)&u32, sizeof(uint32_t));
#ifndef __LITTLE_ENDIAN__
    u32 = swap_endian(u32);
#endif
        if (u32 == 0x9B00C000) return true;
        infile.peek();
    }
    
invalid:
    infile.seekg(0);
    return false;
}

bool isUTF16le(std::ifstream &infile)
{
    uint16_t byte_order_mark;
    infile.read((char *)&byte_order_mark, sizeof(uint16_t));
    
#ifndef __LITTLE_ENDIAN__
    byte_order_mark = byte_order_mark >> 8 | byte_order_mark << 8;
#endif
    if (byte_order_mark == 0xFEFF) return true;
    
    infile.seekg(0);
    return false;
}

// Function to remove whitespaces around specific operators using regular expressions
std::string removeWhitespaceAroundOperators(const std::string& str) {
    // Regular expression pattern to match spaces around the specified operators
    // Operators: {}[]()≤≥≠<>=*/+-▶.,;:!^
    std::regex re(R"(\s*([{}[\]()≤≥≠<>=*/+\-▶.,;:!^])\s*)");

    // Replace matches with the operator and no surrounding spaces
    std::string result = std::regex_replace(str, re, "$1");

    return result;
}

std::string base10ToBase32(unsigned int num) {
    if (num == 0) {
        return "0";  // Edge case: if the number is 0, return "0"
    }

    std::string result;
    const char digits[] = "0123456789ABCDEFGHIJKLMNabcdefgh";  // Base-32 digits
    
    // Keep dividing the number by 32 and store the remainders
    while (num > 0) {
        int remainder = num % 32;  // Get the current base-32 digit
        result += digits[remainder];  // Add the corresponding character
        num /= 32;  // Reduce the number
    }

    // The digits are accumulated in reverse order, so reverse the result string
    std::reverse(result.begin(), result.end());

    return result;
}

// MARK: - Formatting And Writing

void reformatLine(std::string& ln, std::ofstream& outfile) {
    std::regex re;
    std::ifstream infile;
    static std::string indentation("");
    Singleton *singleton = Singleton::shared();

    
    if (preprocessor.python) {
        // We're presently handling Python code.
        preprocessor.parse(ln);
        ln += '\n';
        return;
    }
    
    if (preprocessor.parse(ln)) {
        if (preprocessor.python) {
            // Indicating Python code ahead with the #PYTHON preprocessor, we maintain the line unchanged and return to the calling function.
            ln += '\n';
            return;
        }
        
        ln = std::string("");
        return;
    }
    
    /*
     While parsing the contents, strings may inadvertently undergo parsing, leading
     to potential disruptions in the string's content.
     To address this issue, we prioritize the preservation of any existing strings.
     Subsequently, after parsing, any strings that have been universally altered can
     be restored to their original state.
     */
    strings.preserveStrings(ln);
    strings.blankOutStrings(ln);

    // Remove any leading white spaces before or after.
    trim(ln);
    
    if (ln.length() < 1) {
        ln = std::string("");
        return;
    }
    
    // Remove any comments.
    singleton->comments.preserveComment(ln);
    singleton->comments.removeComment(ln);
    
    ln = removeWhitespaceAroundOperators(ln);
    
    ln = regex_replace(ln, std::regex(R"(>=)"), "≥");
    ln = regex_replace(ln, std::regex(R"(<=)"), "≤");
    ln = regex_replace(ln, std::regex(R"(<>)"), "≠");

    ln = regex_replace(ln, std::regex(R"(,)"), ", ");
    ln = regex_replace(ln, std::regex(R"(^ +(\} *;))"), "$1\n");
    
    /*
     To prevent correcting over-modifications, first replace all double `==` with a single `=`.
     
     Converting standalone `=` to `==` initially can lead to unintended changes like `<=`, `>=`,
     `:=`, and `==` turning into `<==`, `>==`, `:==`, and `===`.
     
     By first reverting all double `==` back to a single `=`, and ensuring that only standalone
     `=` or `:=` with surrounding whitespace are targeted, we can then safely convert `=` to `==`
     without affecting other operators.
     */
    ln = regex_replace(ln, std::regex(R"(==)"), "=");

    // Ensuring that standalone `≥`, `≤`, `≠`, `=`, `:=`, `+`, `-`, `*` and `/` have surrounding whitespace.
    re = R"(≥|≤|≠|=|:=|\+|-|\*|\/)";
    ln = regex_replace(ln, re, " $0 ");
    
    re = R"(([≥≤≠=\+|\-|\*|\/]) +- +)";
    ln = regex_replace(ln, re, "$1 -");
    
    if (!regex_search(ln, std::regex(R"(LOCAL [A-Za-z]\w* = )"))) {
        // We can now safely convert `=` to `==` without affecting other operators.
        ln = regex_replace(ln, std::regex(R"( = )"), " == ");
    }
    
    
    re = std::regex(R"(\b(?:BEGIN|IF|CASE|FOR|WHILE|REPEAT)\b)", std::regex_constants::icase);
    for(auto it = std::sregex_iterator(ln.begin(), ln.end(), re); it != std::sregex_iterator(); ++it) {
        singleton->nestingLevel++;
        singleton->scope = Singleton::Scope::Local;
    }
    
    re = std::regex(R"(\b(?:END|UNTIL)\b)", std::regex_constants::icase);
    for(auto it = std::sregex_iterator(ln.begin(), ln.end(), re); it != std::sregex_iterator(); ++it) {
        singleton->nestingLevel--;
        if (0 == singleton->nestingLevel) {
            singleton->scope = Singleton::Scope::Global;
        }
    }
    
    
    if (Singleton::Scope::Local == singleton->scope) {
        if (!regex_search(ln, std::regex(R"(\b(?:BEGIN|IF|CASE|FOR|WHILE|REPEAT)\b)", std::regex_constants::icase))) {
            ln.insert(0, std::string(Singleton::shared()->nestingLevel * INDENT_WIDTH, ' '));
        } else {
            ln.insert(0, std::string((Singleton::shared()->nestingLevel - 1) * INDENT_WIDTH, ' '));
        }
        ln = regex_replace(ln, std::regex(R"(\(\s*\))"), "");
    }

    strings.restoreStrings(ln);
    singleton->comments.restoreComment(ln);
    rtrim(ln);
    
    if (Singleton::Scope::Global == singleton->scope) {
        ln = regex_replace(ln, std::regex(R"(END;)"), "$0\n");
    }
    
    ln = regex_replace(ln, std::regex(R"(// *-+$)"), "// MARK: -");
    ln = regex_replace(ln, std::regex(R"( +(// *.+))"), "\n" + std::string(Singleton::shared()->nestingLevel * INDENT_WIDTH, ' ') + "$1");
    ln = regex_replace(ln, std::regex(R"(^ *(\[|\d))"), std::string((Singleton::shared()->nestingLevel + 1) * INDENT_WIDTH, ' ') + "$1");
    
    ln += "\n";
}

void writeUTF16Line(const std::string& ln, std::ofstream& outfile) {
    for ( int n = 0; n < ln.length(); n++) {
        uint8_t *ascii = (uint8_t *)&ln.at(n);
        if (ln.at(n) == '\r') continue;
        
        // Output as UTF-16LE
        if (*ascii >= 0x80) {
            uint16_t utf16 = utf8_to_utf16(&ln.at(n));
            
#ifndef __LITTLE_ENDIAN__
            utf16 = utf16 >> 8 | utf16 << 8;
#endif
            outfile.write((const char *)&utf16, 2);
            if ((*ascii & 0b11100000) == 0b11000000) n++;
            if ((*ascii & 0b11110000) == 0b11100000) n+=2;
            if ((*ascii & 0b11111000) == 0b11110000) n+=3;
        } else {
            outfile.put(ln.at(n));
            outfile.put('\0');
        }
    }
}

void formatAndWriteLine(std::string& str, std::ofstream& outfile) {
    Singleton& singleton = *Singleton::shared();
    
    reformatLine(str, outfile);
    writeUTF16Line(str, outfile);
    
    singleton.incrementLineNumber();
}

void processAndWriteLines(std::istringstream& iss, std::ofstream& outfile)
{
    std::string str;
    
    while(getline(iss, str)) {
        formatAndWriteLine(str, outfile);
    }
}

void convertAndFormatFile(std::ifstream& infile, std::ofstream& outfile)
{
    if (!isUTF16le(infile)) {
        infile.close();
        return;
    }
   
    // Read in the whole of the file into a `std::string`
    std::string str;
    
    char c;
    while (!infile.eof()) {
        infile.get(c);
        str += c;
        infile.peek();
    }
    
    
    // The UTF16-LE data first needs to be converted to UTF8 before it can be proccessed.
    uint16_t *utf16_str = (uint16_t *)str.c_str();
    str = utf16_to_utf8(utf16_str, str.size() / 2);
    
    std::regex re;

    /*
     Pre-correct any `THEN`, `DO` or `REPEAT` statements that are followed by other statements on the
     same line by moving the additional statement(s) to the next line. This ensures that the code
     is correctly processed, as it separates the conditional or loop structure from the subsequent
     statements for proper handling.
     */
    re = std::regex(R"(\b(THEN|DO|REPEAT)\b)", std::regex_constants::icase);
    str = regex_replace(str, re, "$0\n");
    
    // Make sure all `LOCAL` & `CONST` are on seperate lines.
    re = std::regex(R"(\b(LOCAL|CONST)\b)", std::regex_constants::icase);
    str = regex_replace(str, re, "\n$0");

    re = std::regex(R"(\bEND;)", std::regex_constants::icase);
    str = regex_replace(str, re, "\n$0");
    
    std::istringstream iss;
    iss.str(str);
    processAndWriteLines(iss, outfile);
}



// MARK: - Command Line

/*
 The decimalToBase24 function converts a given
 base 10 integer into its base 24 representation using a
 specific set of characters. The character set is
 comprised of the following 24 symbols:

     •    Numbers: 0, 1, 2, 3, 4, 5, 6, 7, 8, 9
     •    Letters: C, D, F, H, J, K, M, N, R, U, V, W, X, Y
     
 Character Selection:
 The choice of characters was made to avoid confusion
 with common alphanumeric representations, ensuring
 that each character is visually distinct and easily
 recognizable. This set excludes characters that closely
 resemble each other or numerical digits, promoting
 clarity in representation.
 */
static std::string decimalToBase24(int num) {
    if (num == 0) {
        return "C";
    }

    const std::string base24Chars = "0123456789CDFHJKMNRUVWXY";
    std::string base24;

    while (num > 0) {
        int remainder = num % 24;
        base24 = base24Chars[remainder] + base24; // Prepend character
        num /= 24; // Integer division
    }

    return base24;
}

static std::string getBuildCode(void) {
    std::string str;
    int majorVersionNumber = BUILD_NUMBER / 100000;
    str = std::to_string(majorVersionNumber) + decimalToBase24(BUILD_NUMBER - majorVersionNumber * 100000);
    return str;
}

void help(void)
{
    int rev = BUILD_NUMBER / 1000 % 10;
    
    std::cout << "Copyright (C) 2024 Insoft. All rights reserved.\n";
    std::cout << "Insoft PPL Reformat version, " << BUILD_NUMBER / 100000 << "." << BUILD_NUMBER / 10000 % 10 << (rev ? "." + std::to_string(rev) : "")
    << " (BUILD " << getBuildCode() << "-" << decimalToBase24(BUILD_DATE) << ")\n\n";
    std::cout << "Usage: pplref <input-file> \n\n";
    std::cout << "Additional Commands:\n";
    std::cout << "  pplref {-version | -help}\n";
    std::cout << "    -version                 Display the version information.\n";
    std::cout << "    -help                    Show this help message.\n";
}

void version(void) {
    std::cout << "Copyright (C) 2024 Insoft. All rights reserved.\n";
    std::cout << "Insoft PPL Reformat version, " << BUILD_NUMBER / 100000 << "." << BUILD_NUMBER / 10000 % 10 << "." << BUILD_NUMBER / 1000 % 10
    << " (BUILD " << getBuildCode() << ")\n";
    std::cout << "Built on: " << CURRENT_DATE << "\n";
    std::cout << "Licence: MIT License\n\n";
    std::cout << "For more information, visit: http://www.insoft.uk\n";
}

void error(void)
{
    std::cout << "pplref: try 'pplref -help' for more information\n";
    exit(0);
}

void info(void) {
    std::cout << "Copyright (c) 2024 Insoft. All rights reserved.\n";
    int rev = BUILD_NUMBER / 1000 % 10;
    std::cout << "Insoft PPL Reformat version, " << BUILD_NUMBER / 100000 << "." << BUILD_NUMBER / 10000 % 10 << (rev ? "." + std::to_string(rev) : "")
    << " (BUILD " << getBuildCode() << "-" << decimalToBase24(BUILD_DATE) << ")\n\n";
}

// Custom facet to use comma as the thousands separator
struct comma_numpunct : std::numpunct<char> {
protected:
    virtual char do_thousands_sep() const override {
        return ',';  // Define the thousands separator as a comma
    }

    virtual std::string do_grouping() const override {
        return "\3";  // Group by 3 digits
    }
};

// MARK: - Main
int main(int argc, char **argv) {
    std::string in_filename, out_filename;

    if ( argc == 1 )
    {
        error();
        exit(100);
    }
    
    for( int n = 1; n < argc; n++ ) {
        std::string args(argv[n]);
        
        if ( args == "--help" ) {
            help();
            exit(102);
        }
        
        if ( strcmp( argv[n], "--version" ) == 0 ) {
            version();
            return 0;
        }
        
        in_filename = argv[n];
        std::regex re(R"(.\w*$)");
        std::smatch extension;
    }
    
    info();
    
    out_filename = in_filename;
    if (out_filename.rfind(".")) {
        out_filename.replace(out_filename.rfind("."), out_filename.length() - out_filename.rfind("."), "-ref.hpprgm");
    }
    
    std::ofstream outfile;
    outfile.open(out_filename, std::ios::out | std::ios::binary);
    if(!outfile.is_open())
    {
        error();
        return 0;
    }
    
    std::ifstream infile;
    infile.open(in_filename, std::ios::in | std::ios::binary);
    if(!infile.is_open())
    {
        outfile.close();
        error();
        return 0;
    }
    
    // The "hpprgm" file format requires UTF-16LE.
    
    
    outfile.put(0xFF);
    outfile.put(0xFE);
    
    // Start measuring time
    Timer timer;
    
    std::string str;

    convertAndFormatFile( infile, outfile );
    
    // Stop measuring time and calculate the elapsed time.
    long long elapsed_time = timer.elapsed();
    
    // Display elasps time in secononds.
    std::cout << "Completed in " << std::fixed << std::setprecision(2) << elapsed_time / 1e9 << " seconds\n";
    
    infile.close();
    outfile.close();
    
    if (hasErrors() == true) {
        std::cout << "ERRORS!\n";
        remove(out_filename.c_str());
        return 0;
    }
    
    // Percentage Reduction = (Original Size - New Size) / Original Size * 100
    std::ifstream::pos_type original_size = file_size(in_filename);
    std::ifstream::pos_type new_size = file_size(out_filename);
    
    // Create a locale with the custom comma-based numpunct
    std::locale commaLocale(std::locale::classic(), new comma_numpunct);
    std::cout.imbue(commaLocale);
    
    if (original_size > new_size) {
        std::cout << "Reformatting reduced file size by " << (original_size - new_size) * 100 / original_size;
        std::cout << "% or " << original_size - new_size << " bytes.\n";
    }
    else {
        std::cout << "Reformatting increased file size by " << (new_size - original_size) * 100 / new_size;
        std::cout << "% or " << new_size - original_size << " bytes.\n";
    }
    std::cout << "UTF-16LE file '" << regex_replace(out_filename, std::regex(R"(.*/)"), "") << "' succefuly created.\n";
    
    return 0;
}
