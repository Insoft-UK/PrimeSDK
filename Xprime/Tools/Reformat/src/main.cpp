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
#include "../../PrimePlus/src/utf.hpp"

#include "singleton.hpp"
#include "common.hpp"

#include "preprocessor.hpp"
#include "strings.hpp"

#include "../version_code.h"
#include "timer.hpp"

#define NAME "PPL Reformat"
#define COMMAND_NAME "pplref"

using namespace ppl;

static Preprocessor preprocessor = Preprocessor();
static Strings strings = Strings();
static std::vector<std::string> operators = { ":=", "==", "▶", "≥", "≤", "≠", "-", "+", "*", "/" };



void terminator() {
    std::cout << MessageType::Error << "An internal preprocessing problem occurred. Please review the syntax before this point.\n";
    exit(-1);
}

void (*old_terminate)() = std::set_terminate(terminator);

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

std::string clean_whitespace(const std::string& input) {
    std::string output;
    char current = '\0';
    
    auto iswordc = [](char c) {
        return std::isalnum(static_cast<unsigned char>(c)) || c == '_';
    };
    
    for (size_t i = 0; i < input.length(); i++) {
        if (std::isspace(static_cast<unsigned char>(current))) {
            if(iswordc(input[i]) && !output.empty() && iswordc(output.back())) {
                output += ' ';
            }
        }
        current = input[i];

        if (std::isspace(static_cast<unsigned char>(current))) {
            continue;
        }
        output += current;
    }
    

    return output;
}

std::string normalize_whitespace(const std::string& input) {
    std::string output;
    output.reserve(input.size());  // Optimize memory allocation

    bool in_whitespace = false;

    for (char ch : input) {
        if (std::isspace(static_cast<unsigned char>(ch))) {
            if (!in_whitespace) {
                output += ' ';
                in_whitespace = true;
            }
        } else {
            output += ch;
            in_whitespace = false;
        }
    }

    return output;
}

std::string normalize_operators(const std::string& input) {
    // List of all operators to normalize
    
    std::string result;
    size_t i = 0;
    
    while (i < input.size()) {
        bool matched = false;
        
        for (const std::string& op : operators) {
            if (input.compare(i, op.size(), op) == 0) {
                if (!result.empty() && result.back() != ' ') result += ' ';
                result += op;
                i += op.size();
                if (i < input.size() && input[i] != ' ') result += ' ';
                matched = true;
                break;
            }
        }
        
        if (!matched) {
            result += input[i++];
        }
    }
    
    // Final cleanup: collapse multiple spaces
    std::istringstream iss(result);
    std::string word, cleaned;
    while (iss >> word) {
        if (!cleaned.empty()) cleaned += ' ';
        cleaned += word;
    }
    
    return cleaned;
}

std::string insert_space_after_chars(const std::string& input, const std::unordered_set<char>& chars) {
    std::string output;
    size_t len = input.length();

    for (size_t i = 0; i < len; ++i) {
        char c = input[i];
        output += c;

        if (chars.find(c) != chars.end()) {
            size_t j = i + 1;

            // Skip existing spaces
            while (j < len && std::isspace(static_cast<unsigned char>(input[j]))) {
                ++j;
            }

            // Insert exactly one space **only if** the next char is not already a space
            if (i + 1 >= len || input[i + 1] != ' ') {
                output += ' ';
            }

            // Skip over any extra spaces already in input
            i = j - 1;
        }
    }

    return output;
}

std::string insert_space_before_word_after_closing_paren(const std::string& input) {
    std::string output;
    size_t len = input.length();

    for (size_t i = 0; i < len; ++i) {
        char curr = input[i];

        // Check if current is start of a word and prev is ')'
        if (i > 0 && std::isalpha(static_cast<unsigned char>(curr)) &&
            input[i - 1] == ')' &&
            (i == 1 || input[i - 2] != ' ')) {
            output += ' ';
        }

        output += curr;
    }

    return output;
}

// MARK: - Formatting And Writing

std::string reformatLine(const std::string& str) {
    std::regex re;
    std::ifstream infile;
    std::string result = str;
    static std::string indentation("");
    Singleton *singleton = Singleton::shared();
    
    if (preprocessor.python) {
        // We're presently handling Python code.
        preprocessor.parse(result);
        result += '\n';
        return result;
    }
    
    if (preprocessor.parse(result)) {
        if (preprocessor.python) {
            // Indicating Python code ahead with the #PYTHON preprocessor, we maintain the line unchanged and return to the calling function.
            result += '\n';
            return result;
        }
        
        result += '\n';
        return result;
    }
    
    /*
     While parsing the contents, strings may inadvertently undergo parsing, leading
     to potential disruptions in the string's content.
     To address this issue, we prioritize the preservation of any existing strings.
     Subsequently, after parsing, any strings that have been universally altered can
     be restored to their original state.
     */
    strings.preserveStrings(result);
    strings.blankOutStrings(result);

    // Remove any leading white spaces before or after.
    trim(result);
    
    if (result.length() < 1) {
        return result;
    }
    
    // Remove any comments.
    singleton->comments.preserveComment(result);
    singleton->comments.removeComment(result);
    
    
    
    result = capitalize_words(result, {
        "begin", "end", "return", "kill", "if", "then", "else", "xor", "or", "and", "not",
        "case", "default", "iferr", "ifte", "for", "from", "step", "downto", "to", "do",
        "while", "repeat", "until", "break", "continue", "export", "const", "local", "key"
    });
    result = capitalize_words(result, {"log", "cos", "sin", "tan", "ln", "min", "max"});
    result = replace_words(result, {"FROM"}, ":=");
    result = clean_whitespace(result);
    result = replace_operators(result);
    result = fix_unary_minus(result);
    result = normalize_operators(result);
    result = insert_space_after_chars(result, {',', ';'});
    result = insert_space_before_word_after_closing_paren(result);
    
    
    re = std::regex(R"(\b(?:BEGIN|IF|CASE|FOR|WHILE|REPEAT)\b)", std::regex_constants::icase);
    for(auto it = std::sregex_iterator(result.begin(), result.end(), re); it != std::sregex_iterator(); ++it) {
        singleton->nestingLevel++;
        singleton->scope = Singleton::Scope::Local;
    }
    
    re = std::regex(R"(\b(?:END|UNTIL)\b)", std::regex_constants::icase);
    for(auto it = std::sregex_iterator(result.begin(), result.end(), re); it != std::sregex_iterator(); ++it) {
        singleton->nestingLevel--;
        if (0 == singleton->nestingLevel) {
            singleton->scope = Singleton::Scope::Global;
        }
    }
    
    
    if (Singleton::Scope::Local == singleton->scope) {
        if (!regex_search(result, std::regex(R"(\b(?:BEGIN|IF|CASE|FOR|WHILE|REPEAT)\b)", std::regex_constants::icase))) {
            result.insert(0, std::string(Singleton::shared()->nestingLevel * INDENT_WIDTH, ' '));
        } else {
            result.insert(0, std::string((Singleton::shared()->nestingLevel - 1) * INDENT_WIDTH, ' '));
        }
        result = regex_replace(result, std::regex(R"(\(\s*\))"), "");
    }

    strings.restoreStrings(result);
    singleton->comments.restoreComment(result);
    rtrim(result);
    
    if (Singleton::shared()->nestingLevel == 1) {
        result = regex_replace(result, std::regex(R"(END;)"), "$0\n");
    }
    
    if (Singleton::shared()->nestingLevel == 0 && result != "END;") {
        result = result.insert(0, "\n");
    }
    
    result = regex_replace(result, std::regex(R"(^ *(\[|\d))"), std::string((Singleton::shared()->nestingLevel + 1) * INDENT_WIDTH, ' ') + "$1");
    
    result += "\n";
    return result;
}



std::string reformatAllLines(std::istringstream& iss)
{
    std::string str;
    std::string result;
    
    while(getline(iss, str)) {
        result.append(reformatLine(str));
        Singleton::shared()->incrementLineNumber();
    }
    
    return result;
}

std::string reformatPrgm(std::ifstream& infile)
{
    // Read in the whole of the file into a `std::string`
    std::string str;
    std::wstring wstr;
    
    wstr = utf::read_utf16(infile);
    if (wstr.empty()) {
        char c;
        while (!infile.eof()) {
            infile.get(c);
            str += c;
            infile.peek();
        }
    } else {
        str = utf::to_utf8(wstr);
    }
    
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
    
    return reformatAllLines(iss);
}



// MARK: - Command Line

void help(void)
{
    std::cout << "Copyright (C) 2024-" << YEAR << " Insoft. All rights reserved.\n";
    std::cout << "Insoft "<< NAME << " version, " << VERSION_NUMBER << " (BUILD " << VERSION_CODE << ")\n";
    std::cout << "\n";
    std::cout << "Usage: " << COMMAND_NAME << " <input-file>\n\n";
    std::cout << "Additional Commands:\n";
    std::cout << "  " << COMMAND_NAME << " {-version | -help}\n";
    std::cout << "    -version                 Display the version information.\n";
    std::cout << "    -help                    Show this help message.\n";
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
    std::cout << "Insoft "<< NAME << " version, " << VERSION_NUMBER << " (BUILD " << VERSION_CODE << ")\n\n";
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
    
    out_filename = path.parent_path().string() + "/" + path.stem().string() + "-ref.prgm";
    
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

    
    // Start measuring time
    Timer timer;
    
    std::string str;

    str = reformatPrgm(infile);
    utf::save_as_utf16(out_filename, str);
    
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
    
    std::cout << "File '" << regex_replace(out_filename, std::regex(R"(.*/)"), "") << "' succefuly created.\n";
    
    return 0;
}
