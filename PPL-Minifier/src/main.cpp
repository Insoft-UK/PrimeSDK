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
#include <unordered_set>

#include <sys/time.h>

#include "singleton.hpp"
#include "common.hpp"
#include "hpprgm.hpp"

#include "preprocessor.hpp"
#include "strings.hpp"

#include "../version_code.h"
#include "timer.hpp"

#define NAME "PPL Minifier"
#define COMMAND_NAME "pplmin"

static Preprocessor preprocessor = Preprocessor();
static Strings strings = Strings();
static bool preserveFunctionNames = false;

void terminator() {
  std::cout << MessageType::Error << "An internal preprocessing problem occurred. Please review the syntax before this point.\n";
  exit(-1);
}
 
void (*old_terminate)() = std::set_terminate(terminator);


void preProcess(std::string &ln, std::ofstream &outfile);

// MARK: - Extensions

namespace std::filesystem {
    std::filesystem::path expand_tilde(const std::filesystem::path& path) {
        if (!path.empty() && path.string().starts_with("~")) {
#ifdef _WIN32
            const char* home = std::getenv("USERPROFILE");
#else
            const char* home = std::getenv("HOME");
#endif
            
            if (home) {
                return std::filesystem::path(std::string(home) + path.string().substr(1));  // Replace '~' with $HOME
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

// MARK: - Utills



uint32_t utf8_to_utf16(const char *utf8) {
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

// TODO: .hpprgrm file format detection and handling.
bool isHPPrgrmFileFormat(std::ifstream &infile)
{
    uint32_t u32;
    infile.read((char *)&u32, sizeof(uint32_t));
    
#ifndef __LITTLE_ENDIAN__
    u32 = std::byteswap(u32);
#endif
    
    if (u32 != 0x7C618AB2) {
        goto invalid;
    }
    
    while (!infile.eof()) {
        infile.read((char *)&u32, sizeof(uint32_t));
#ifndef __LITTLE_ENDIAN__
    u32 = std::byteswap(u32);
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

std::string fix_unary_minus(const std::string& input) {
    const std::string ops = "+-*/=:";

    // We only need to check ":=" pair, no need to store more.
    // Using a simple check instead of unordered_set for this single exception.
    const std::string exception = ":=";

    std::string output;
    output.reserve(input.size() + 10); // Reserve slightly more for spaces

    for (size_t i = 0; i < input.size(); ++i) {
        char curr = input[i];
        output += curr;

        if (i + 1 < input.size()) {
            char next = input[i + 1];

            if (ops.find(curr) != std::string::npos &&
                ops.find(next) != std::string::npos &&
                next == '-') {

                // Check if pair is ":=", skip adding space in that case
                if (!(curr == exception[0] && next == exception[1])) {
                    output += ' ';
                }
            }
        }
    }

    return output;
}

std::string to_lower(const std::string& s) {
    std::string result = s;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return result;
}

std::string to_upper(const std::string& s) {
    std::string result = s;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return std::toupper(c); });
    return result;
}

std::string replace_operators(const std::string& input) {
    std::string output;
    output.reserve(input.size());  // Reserve space to reduce reallocations

    for (std::size_t i = 0; i < input.size(); ++i) {
        if (i + 1 < input.size()) {
            // Lookahead for 2-character operators
            if (input[i] == '>' && input[i + 1] == '=') {
                output += "≥";
                ++i;
                continue;
            }
            if (input[i] == '<' && input[i + 1] == '=') {
                output += "≤";
                ++i;
                continue;
            }
            if (input[i] == '=' && input[i + 1] == '>') {
                output += "▶";
                ++i;
                continue;
            }
            if (input[i] == '<' && input[i + 1] == '>') {
                output += "≠";
                ++i;
                continue;
            }
            if (input[i] == '=' && input[i + 1] == '=') {
                output += "=";
                ++i;
                continue;
            }
        }

        // Default: copy character
        output += input[i];
    }

    return output;
}

std::string replace_words(const std::string& input, const std::vector<std::string>& words, const std::string& replacement) {
    // Create lowercase word set
    std::unordered_set<std::string> wordset;
    for (const auto& w : words) {
        wordset.insert(to_lower(w));
    }

    std::string result;
    size_t i = 0;
    
    while (i < input.size()) {
        if (!isalpha(static_cast<unsigned char>(input[i])) && input[i] != '_') {
            result += input[i];
            ++i;
            continue;
        }
        size_t start = i;
        
        while (i < input.size() && (isalpha(static_cast<unsigned char>(input[i])) || input[i] == '_')) {
            ++i;
        }
        
        std::string word = input.substr(start, i - start);
        std::string lowerWord = to_lower(word);
        
        if (wordset.count(lowerWord)) {
            result += replacement;
            continue;
        }
        
        result += word;
    }
    
    return result;
}

std::string capitalize_words(const std::string& input, const std::unordered_set<std::string>& words) {
    // Create lowercase word set
    std::unordered_set<std::string> wordset;
    for (const auto& w : words) {
        wordset.insert(to_lower(w));
    }
    
    std::string result;
    size_t i = 0;
    
    while (i < input.size()) {
        if (!isalpha(static_cast<unsigned char>(input[i])) && input[i] != '_') {
            result += input[i];
            ++i;
            continue;
        }
        size_t start = i;
        
        while (i < input.size() && (isalpha(static_cast<unsigned char>(input[i])) || input[i] == '_')) {
            ++i;
        }
        
        std::string word = input.substr(start, i - start);
        std::string lowercase = to_lower(word);
        
        if (wordset.count(lowercase)) {
            result += to_upper(lowercase);
            continue;
        }
        
        result += word;
    }
    
    return result;
}

// MARK: - Minifying And Writing

void minifieLine(std::string &ln, std::ofstream &outfile) {
    std::regex re;
    std::smatch match;
    std::ifstream infile;
    
    Singleton *singleton = Singleton::shared();
    
    static int globalVariableAliasCount = -1, localVariableAliasCount = -1, functionAliasCount = -1;
    
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
     While parsing the contents, strings may inadvertently undergo parsing, leading to potential disruptions in the string's content.
     To address this issue, we prioritize the preservation of any existing strings. Subsequently, after parsing, any strings that have
     been universally altered can be restored to their original state.
     */
    strings.preserveStrings(ln);
    strings.blankOutStrings(ln);
    
    // Remove any comments.
    size_t pos = ln.find("//");
    if (pos != std::string::npos) {
        ln.resize(pos);
    }
    
    // Remove sequences of whitespaces to a single whitespace.
    ln = std::regex_replace(ln, std::regex(R"(\s+)"), " ");

    // Remove any leading white spaces before or after.
    trim(ln);
    
    if (ln.length() < 1) {
        ln = std::string("");
        return;
    }
    
    ln = capitalize_words(ln, {
        "begin", "end", "return", "kill", "if", "then", "else", "xor", "or", "and", "not",
        "case", "default", "iferr", "ifte", "for", "from", "step", "downto", "to", "do",
        "while", "repeat", "until", "break", "continue", "export", "const", "local", "key"
    });
    ln = capitalize_words(ln, {"log", "cos", "sin", "tan", "ln", "min", "max"});
    ln = replace_words(ln, {"FROM"}, ":=");
    ln = clean_whitespace(ln);
    ln = replace_operators(ln);
    ln = fix_unary_minus(ln);
    
    re = std::regex(R"(\b(?:BEGIN|IF|CASE|FOR|WHILE|REPEAT|FOR|WHILE|REPEAT)\b)", std::regex_constants::icase);
    for(auto it = std::sregex_iterator(ln.begin(), ln.end(), re); it != std::sregex_iterator(); ++it) {
        singleton->nestingLevel++;
        singleton->scope = Singleton::Scope::Local;
    }
    
    re = std::regex(R"(\b(?:END|UNTIL)\b)", std::regex_constants::icase);
    for(auto it = std::sregex_iterator(ln.begin(), ln.end(), re); it != std::sregex_iterator(); ++it) {
        singleton->nestingLevel--;
        if (0 == singleton->nestingLevel) {
            singleton->scope = Singleton::Scope::Global;
            singleton->aliases.removeAllLocalAliases();
            localVariableAliasCount = -1;
        }
    }
    
    Aliases::TIdentity identity;
    
    if (singleton->scope == Singleton::Scope::Global) {
        identity.scope = Aliases::Scope::Global;
        
        // Function
        re = R"(^([A-Za-z]\w*)\(([\w,]*)\);?$)";
        if (regex_search(ln, match, re)) {
            if (!preserveFunctionNames) {
                identity.type = Aliases::Type::Function;
                identity.identifier = match.str(1);
                identity.real = "f" + base10ToBase32(++functionAliasCount);
                singleton->aliases.append(identity);
            }
            
            std::string s = match.str(2);
            int count = -1;
            identity.scope = Aliases::Scope::Local;
            identity.type = Aliases::Type::Property;
            re = R"([A-Za-z]\w*)";
            for(auto it = std::sregex_iterator(s.begin(), s.end(), re); it != std::sregex_iterator(); ++it) {
                if (it->str().length() < 3) continue;
                identity.identifier = it->str();
                identity.real = "p" + base10ToBase32(++count);
                
                if (ln.back() == ';') {
                    ln = regex_replace(ln, std::regex(identity.identifier), identity.real);
                } else {
                    if (!singleton->aliases.exists(identity)) {
                        singleton->aliases.append(identity);
                    }
                }
            }
            identity.scope = Aliases::Scope::Global;
        }
        
        
        // Global Variable
        re = R"(\b(?:LOCAL )?([A-Za-z]\w*)(?::=.*);)";
        if (regex_search(ln, match, re)) {
            if (match.str(1).length() > 3) {
                identity.type = Aliases::Type::Variable;
                identity.identifier = match.str(1);
                identity.real = "g" + base10ToBase32(++globalVariableAliasCount);
                
                singleton->aliases.append(identity);
            }
        }
    }
    
    if (singleton->scope == Singleton::Scope::Local) {
        identity.scope = Aliases::Scope::Local;
        
        // LOCAL
        if (regex_search(ln, match, std::regex(R"(\bLOCAL (?:[A-Za-z]\w*[,;])+)", std::regex_constants::icase))) {
            std::string matched = match.str();
            re = R"([A-Za-z]\w*(?=[,;]))";
            
            for(auto it = std::sregex_iterator(matched.begin(), matched.end(), re); it != std::sregex_iterator(); ++it) {
                if (it->str().length() < 3) continue;
                identity.type = Aliases::Type::Variable;
                identity.identifier = it->str();
                identity.real = "v" + base10ToBase32(++localVariableAliasCount);
                
                singleton->aliases.append(identity);
            }
        }
        
        if (regex_search(ln, match, std::regex(R"(\bLOCAL ([A-Za-z]\w*)(?::=))", std::regex_constants::icase))) {
            identity.type = Aliases::Type::Variable;
            identity.identifier = match.str(1);
            identity.real = "v" + base10ToBase32(++localVariableAliasCount);
            
            singleton->aliases.append(identity);
        }
        
        ln = regex_replace(ln, std::regex(R"(\(\))"), "");
        
        while (regex_search(ln, match, std::regex(R"(^[A-Za-z]\w*:=[^;]*;)"))) {
            std::string matched = match.str();
            
            /*
             eg. v1:=v2+v4;
             Group  0 v1:=v2+v4;
                    1 v1
                    2 v2+v4
            */
            re = R"(([A-Za-z]\w*):=(.*);)";
            auto it = std::sregex_token_iterator {
                matched.begin(), matched.end(), re, {2, 1}
            };
            if (it != std::sregex_token_iterator()) {
                std::stringstream ss;
                ss << *it++ << "▶" << *it << ";";
                ln = ln.replace(match.position(), match.length(), ss.str());
            }
        }
        
        ln = regex_replace(ln, std::regex(R"( +FROM +)", std::regex_constants::icase), ":=");
    }
    
    ln = singleton->aliases.resolveAliasesInText(ln);
    strings.restoreStrings(ln);
    
    ln = regex_replace(ln, std::regex(R"([^;,\[\]\{\}]$)"), "$0\n");
}

void writeUTF16Line(const std::string& ln, std::ofstream& outfile) {
    for ( int n = 0; n < ln.length(); n++) {
        uint8_t *ascii = (uint8_t *)&ln.at(n);
        if (ln.at(n) == '\e') continue;
        
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

void minifieAndWriteLine(std::string& str, std::ofstream& outfile) {
    Singleton& singleton = *Singleton::shared();
    
    minifieLine(str, outfile);
    writeUTF16Line(str, outfile);
    
    singleton.incrementLineNumber();
}

void processAndWriteLines(std::istringstream &iss, std::ofstream &outfile)
{
    std::string str;
    
    while(getline(iss, str)) {
        minifieAndWriteLine(str, outfile);
    }
}

void convertAndFormatFile(std::ifstream &infile, std::ofstream &outfile)
{
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
    re = R"(\b(THEN|DO|REPEAT)\b)";
    str = regex_replace(str, re, "$0\n");
    
    // Make sure all `LOCAL` are on seperate lines.
    re = R"(\b(LOCAL|CASE|IF)\b)";
    str = regex_replace(str, re, "\n$0");

    re = R"(\bEND;)";
    str = regex_replace(str, re, "\n$0");
    
    std::istringstream iss;
    iss.str(str);
    processAndWriteLines(iss, outfile);
}


void version(void) {
    std::cout
    << "Copyright (C) 2024 Insoft. All rights reserved."
    << "Insoft "<< NAME << " version, " << VERSION_NUMBER << " (BUILD " << VERSION_CODE << ")"
    << "Built on: " << DATE << ""
    << "Licence: MIT License\n"
    << "For more information, visit: http://www.insoft.uk\n";
}

void error(void) {
    std::cout << COMMAND_NAME << ": try '" << COMMAND_NAME << " --help' for more information\n";
    exit(0);
}

void info(void) {
    std::cout 
    << "Copyright (c) 2024 Insoft. All rights reserved."
    << "Insoft "<< NAME << " version, " << VERSION_NUMBER << " (BUILD " << VERSION_CODE << ")\n\n";
}

void help(void) {
    std::cout 
    << "Copyright (C) 2024-" << YEAR << " Insoft. All rights reserved."
    << "Insoft "<< NAME << " version, " << VERSION_NUMBER << " (BUILD " << VERSION_CODE << ")"
    << "\n"
    << "Usage: " << COMMAND_NAME << " <input-file>\n"
    << "\n"
    << "Options:\n"
    << "  -f                      Preserve function names.\n"
    << "\n"
    << "Additional Commands:"
    << "  " << COMMAND_NAME << " {-version | -help}"
    << "    -version              Display the version information."
    << "    -help                 Show this help message.\n";
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
        exit(0);
    }
    
    for( int n = 1; n < argc; n++ ) {
        std::string args(argv[n]);
        
        if ( args == "-help" ) {
            help();
            exit(0);
        }
        
        if ( args == "-f" ) {
            preserveFunctionNames = true;
            continue;
        }
        
        if ( strcmp( argv[n], "-version" ) == 0 ) {
            version();
            return 0;
        }
        
        in_filename = std::filesystem::expand_tilde(argv[n]);
        std::regex re(R"(.\w*$)");
        std::smatch extension;
    }
    
    info();
    
    if (std::filesystem::path(in_filename).parent_path().empty()) {
        in_filename = in_filename.insert(0, "./");
    }
    std::filesystem::path path = in_filename;
    
    path = std::filesystem::expand_tilde(path);
    
    if (path.extension().empty()) {
        path.append(".prgm");
    }
    if (!std::filesystem::exists(path) || path.extension() == ".hpprgm") {
        error();
        return 0;
    }
    
    out_filename = path.parent_path().string() + "/" + path.stem().string() + "-min.prgm";
    
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

    if (!isUTF16le(infile)) {
        infile.close();
        outfile.close();
        std::cout << "ERRORS! not a valid UTF16le file.\n";
        remove(out_filename.c_str());
        return 0;
    }
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
    
    if (!Singleton::shared()->aliases.descendingOrder && Singleton::shared()->aliases.verbose) {
        Singleton::shared()->aliases.dumpIdentities();
    }
    
    // Percentage Reduction = (Original Size - New Size) / Original Size * 100
    std::ifstream::pos_type original_size = file_size(in_filename);
    std::ifstream::pos_type new_size = file_size(out_filename);
    
    // Create a locale with the custom comma-based numpunct
    std::locale commaLocale(std::locale::classic(), new comma_numpunct);
    std::cout.imbue(commaLocale);
    
    std::cout << "Reduction of " << (original_size - new_size) * 100 / original_size;
    std::cout << "% or " << original_size - new_size << " bytes.\n";
    
    std::cout << "UTF-16LE file '" << regex_replace(out_filename, std::regex(R"(.*/)"), "") << "' succefuly created.\n";
    
    
    return 0;
}
