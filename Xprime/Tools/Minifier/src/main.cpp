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
#include <list>
#include "../../PrimePlus/src/utf.hpp"

#include <sys/time.h>

//#include "singleton.hpp"
#include "common.hpp"

#include "../version_code.h"
#include "timer.hpp"

#define NAME "PPL Minifier"
#define COMMAND_NAME "pplmin"

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

/**
 * @brief Cleans up whitespace in a string while preserving word separation and '\n'.
 *
 * This function removes all unnecessary whitespace characters (spaces, tabs, newlines, etc.)
 * from the input string. It ensures that only a single space is inserted between consecutive
 * word characters (letters, digits, or underscores) when needed to maintain logical separation.
 *
 * Non-word characters (such as punctuation) are not separated by spaces, and leading/trailing
 * whitespace is removed.
 *
 * @param input The input string to be cleaned.
 * @return A new string with cleaned and normalized whitespace.
 *
 * @note This is useful for normalizing input for parsers, code formatters, or text display
 *       where compact and readable word separation is desired.
 */
std::string cleanWhitespace(const std::string& input) {
    std::string output;
    bool lastWasWordChar = false;
    bool pendingSpace = false;

    auto isWordChar = [](char c) {
        return std::isalnum(static_cast<unsigned char>(c)) || c == '_';
    };

    for (char ch : input) {
        if (ch == '\n') {
            if (pendingSpace) {
                pendingSpace = false; // discard pending space before newline
            }
            output += '\n';
            lastWasWordChar = false;
        } else if (std::isspace(static_cast<unsigned char>(ch))) {
            if (lastWasWordChar) {
                pendingSpace = true;
            }
        } else {
            if (pendingSpace && lastWasWordChar && isWordChar(ch)) {
                output += ' ';
            }
            output += ch;
            lastWasWordChar = isWordChar(ch);
            pendingSpace = false;
        }
    }

    return output;
}

/**
 * @brief Replaces common two-character operators with their symbolic Unicode equivalents.
 *
 * This function scans the input string and replaces specific two-character operator
 * sequences with their corresponding Unicode symbols:
 *
 * - `>=` becomes `≥`
 * - `<=` becomes `≤`
 * - `=>` becomes `▶`
 * - `<>` becomes `≠`
 *
 * All other characters are copied as-is.
 *
 * @param input The input string potentially containing ASCII operator sequences.
 * @return A new string with supported operators replaced by Unicode symbols.
 *
 * @note Useful for rendering more readable mathematical or logical expressions in UI output,
 *       documents, or educational tools.
 */
std::string replaceOperators(const std::string& input) {
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

/**
 * @brief Converts all characters in a string to lowercase.
 *
 * This function takes an input string and returns a new string
 * with every character converted to its lowercase equivalent,
 * using the standard C locale rules.
 *
 * @param s The input string to convert.
 * @return A new string with all characters in lowercase.
 *
 * @note The conversion uses `std::tolower` with `unsigned char` casting to
 *       avoid undefined behavior for negative `char` values.
 *
 * Example:
 * toLower("Hello World!") returns "hello world!"
 */
std::string toLower(const std::string& s) {
    std::string result = s;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return result;
}

/**
 * @brief Converts all characters in a string to uppercase.
 *
 * This function takes an input string and returns a new string
 * with every character converted to its uppercase equivalent,
 * using the standard C locale rules.
 *
 * @param s The input string to convert.
 * @return A new string with all characters in uppercase.
 *
 * @note The conversion uses `std::toupper` with `unsigned char` casting to
 *       avoid undefined behavior for negative `char` values.
 *
 * Example:
 * toUpper("Hello World!") returns "HELLO WORLD!"
 */
std::string toUpper(const std::string& s) {
    std::string result = s;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return std::toupper(c); });
    return result;
}

/**
 * @brief Replaces specified words in a string with a given replacement string.
 *
 * This function scans the input string and replaces all occurrences of words
 * found in the provided list (case-insensitive) with the specified replacement string.
 * Words are defined as sequences of alphabetic characters and underscores (`_`).
 * Non-word characters are preserved as-is.
 *
 * @param input The input string to process.
 * @param words A vector of words to be replaced (case-insensitive).
 * @param replacement The string to replace each matched word with.
 * @return A new string with the specified words replaced.
 *
 * @note Matching is case-insensitive. The function treats underscores as part of words.
 *
 * Example"
 *   replaceWords("Hello world_123", {"world_123"}, "Earth") returns "Hello Earth"
 */
std::string replaceWords(const std::string& input, const std::vector<std::string>& words, const std::string& replacement) {
    // Create lowercase word set
    std::unordered_set<std::string> wordSet;
    for (const auto& w : words) {
        wordSet.insert(toLower(w));
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
        std::string lowercase = toLower(word);
        
        if (wordSet.count(lowercase)) {
            result += replacement;
            continue;
        }
        
        result += word;
    }
    
    return result;
}

/**
 * @brief Capitalizes specified words in a string by converting them to uppercase.
 *
 * This function scans the input string and converts to uppercase all occurrences
 * of words found in the provided set (case-insensitive). Words are defined as
 * sequences of alphabetic characters and underscores (`_`). Non-word characters
 * are preserved as-is.
 *
 * @param input The input string to process.
 * @param words An unordered set of words to capitalize (case-insensitive).
 * @return A new string with the specified words converted to uppercase.
 *
 * @note Matching is case-insensitive. The function treats underscores as part of words.
 *
 * Example usage:
 * capitalizeWords("hello world_test", {"world_test"}) returns "hello WORLD_TEST"
 */
std::string capitalizeWords(const std::string& input, const std::unordered_set<std::string>& words) {
    // Create lowercase word set
    std::unordered_set<std::string> wordSet;
    for (const auto& w : words) {
        wordSet.insert(toLower(w));
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
        std::string lowercase = toLower(word);
        
        if (wordSet.count(lowercase)) {
            result += toUpper(lowercase);
            continue;
        }
        
        result += word;
    }
    
    return result;
}

std::string replaceTabsWithSpaces(const std::string& input) {
    std::string result = input;
    for (char& ch : result) {
        if (ch == '\t') {
            ch = ' ';
        }
    }
    return result;
}

std::string reduceMultipleSpaces(const std::string& input) {
    std::ostringstream oss;
    bool inSpace = false;

    for (char ch : input) {
        if (ch == ' ') {
            if (!inSpace) {
                oss << ' ';
                inSpace = true;
            }
        } else {
            oss << ch;
            inSpace = false;
        }
    }

    return oss.str();
}

std::string removeBlankLines(const std::string& input) {
    std::istringstream iss(input);
    std::ostringstream oss;
    std::string line;

    while (std::getline(iss, line)) {
        // Check if the line has non-whitespace characters
        if (line.find_first_not_of(" \t\r") != std::string::npos) {
            oss << line << '\n';
        }
    }

    return oss.str();
}

std::string removeNewlineAfterSemicolon(const std::string& input) {
    std::string output;
    size_t len = input.length();

    for (size_t i = 0; i < len; ++i) {
        if (input[i] == '\n' && i > 0 && input[i - 1] == ';') {
            continue; // skip the newline after a semicolon
        }
        output += input[i];
    }

    return output;
}

std::string removeNewlineBeforeSymbols(const std::string& input) {
    std::string output;
    const std::string symbols = "{}[](),";
    size_t i = 0;
    size_t len = input.length();

    while (i < len) {
        if (input[i] == '\n') {
            size_t j = i + 1;
            // Skip spaces/tabs after newline
            while (j < len && (input[j] == ' ' || input[j] == '\t')) {
                ++j;
            }
            // If the first non-whitespace char is a target symbol, skip the newline and spaces
            if (j < len && symbols.find(input[j]) != std::string::npos) {
                i = j; // Skip to the symbol, skipping \n and spaces
                continue;
            }
        }
        output += input[i];
        ++i;
    }

    return output;
}

std::list<std::string> extractPythonBlocks(const std::string& str) {
    std::list<std::string> blocks;
    const std::string startTag = "#PYTHON";
    const std::string endTag = "#END";
    
    size_t pos = 0;

    while (true) {
        size_t start = str.find(startTag, pos);
        if (start == std::string::npos)
            break;

        start += startTag.length();  // move past the #PYTHON tag

        size_t end = str.find(endTag, start);
        if (end == std::string::npos)
            break;  // no matching #END, so stop

        blocks.push_back(str.substr(start, end - start));
        pos = end + endTag.length();  // move past this #END
    }

    return blocks;
}


std::string blankOutPythonBlocks(const std::string& str) {
    std::string result;
    const std::string startTag = "#PYTHON";
    const std::string endTag = "#END";
    
    size_t pos = 0;

    while (pos < str.length()) {
        size_t start = str.find(startTag, pos);

        if (start == std::string::npos) {
            result.append(str, pos, str.length() - pos);
            break;
        }

        // Append everything before #PYTHON
        result.append(str, pos, start - pos);

        size_t end = str.find(endTag, start + startTag.length());
        if (end == std::string::npos) {
            // No matching #END — treat rest as normal text
            result.append(str, start, str.length() - start);
            break;
        }

        // Keep the #PYTHON and #END markers, but blank out in between
        result += startTag;
        result.append(end - (start + startTag.length()), ' ');
        result += endTag;

        pos = end + endTag.length();
    }

    return result;
}

std::string restorePythonBlocks(const std::string& str, std::list<std::string>& blocks) {
    if (blocks.empty()) return str;

    const std::string startTag = "#PYTHON";
    const std::string endTag = "#END";

    std::string result;
    size_t pos = 0;

    while (pos < str.size()) {
        size_t start = str.find(startTag, pos);
        if (start == std::string::npos) {
            result.append(str, pos, str.size() - pos);  // append rest
            break;
        }

        // Append text before #PYTHON
        result.append(str, pos, start - pos);

        size_t end = str.find(endTag, start + startTag.length());
        if (end == std::string::npos || blocks.empty()) {
            // No matching #END or no block left — append rest
            result.append(str, start, str.size() - start);
            break;
        }

        // Append #PYTHON
        result.append(str, start, startTag.length());

        // Append original block content
        result.append(blocks.front());
        blocks.pop_front();

        // Append #END
        result.append(str, end, endTag.length());

        pos = end + endTag.length();
    }

    return result;
}


std::string separatePythonMarkers(const std::string& input) {
    std::istringstream iss(input);
    std::ostringstream oss;
    std::string line;

    const std::string markers[] = {"#PYTHON", "#END"};

    while (std::getline(iss, line)) {
        size_t pos = 0;

        while (pos < line.size()) {
            bool foundMarker = false;
            for (const std::string& marker : markers) {
                size_t markerPos = line.find(marker, pos);
                if (markerPos != std::string::npos) {
                    // Add any content before the marker (if any) as a separate line
                    if (markerPos > pos) {
                        oss << line.substr(pos, markerPos - pos) << '\n';
                    }
                    // Add the marker as its own line
                    oss << marker << '\n';
                    pos = markerPos + marker.length();
                    foundMarker = true;
                    break;
                }
            }

            if (!foundMarker) {
                // No more markers on this line, output the rest
                oss << line.substr(pos) << '\n';
                break;
            }
        }
    }

    return oss.str();
}

/**
 * @brief Extracts and preserves all double-quoted substrings from the input string.
 *
 * Handles escaped quotes (e.g., \" inside quoted text) and does not use regex.
 *
 * @param str The input string.
 * @return std::list<std::string> A list of quoted substrings, including the quote characters.
 */
std::list<std::string> preserveStrings(const std::string& str) {
    std::list<std::string> strings;
    bool inQuotes = false;
    std::string current;
    
    for (size_t i = 0; i < str.size(); ++i) {
        char c = str[i];

        if (!inQuotes) {
            if (c == '"') {
                inQuotes = true;
                current.clear();
                current += c;  // start quote
            }
        } else {
            current += c;

            if (c == '"' && (i == 0 || str[i - 1] != '\\')) {
                // End of quoted string (unescaped quote)
                inQuotes = false;
                strings.push_back(current);
            }
        }
    }

    return strings;
}
/**
 * @brief Replaces all double-quoted substrings in the input string with "".
 *
 * Handles escaped quotes (e.g., \" inside strings) and does not use regex.
 *
 * @param str The input string to process.
 * @return std::string A new string with quoted substrings replaced by "".
 */
std::string blankOutStrings(const std::string& str) {
    std::string result;
    bool inQuotes = false;
    size_t start = 0;

    for (size_t i = 0; i < str.length(); ++i) {
        // Start of quoted string
        if (!inQuotes && str[i] == '"') {
            inQuotes = true;
            result.append(str, start, i - start);  // Append text before quote
            start = i; // mark quote start
        }
        // Inside quoted string
        else if (inQuotes && str[i] == '"' && (i == 0 || str[i - 1] != '\\')) {
            // End of quoted string
            inQuotes = false;
            result += "\"\"";  // Replace quoted string with empty quotes
            start = i + 1;     // Next copy chunk starts after closing quote
        }
    }

    // Append remaining text after last quoted section
    if (start < str.size()) {
        result.append(str, start, str.size() - start);
    }

    return result;
}

/**
 * @brief Restores quoted strings into a string that had them blanked out.
 *
 * @param str The string with blanked-out quoted substrings (e.g., `""`).
 * @param strings A list of original quoted substrings, in the order they appeared.
 * @return std::string A new string with the original quoted substrings restored.
 */
std::string restoreStrings(const std::string& str, std::list<std::string>& strings) {
    static const std::regex re(R"("[^"]*")");

    if (strings.empty()) return str;

    std::string result;
    std::size_t lastPos = 0;

    auto stringIt = strings.begin();
    for (auto it = std::sregex_iterator(str.begin(), str.end(), re);
         it != std::sregex_iterator() && stringIt != strings.end(); ++it, ++stringIt)
    {
        const std::smatch& match = *it;

        // Append the part before the match
        result.append(str, lastPos, match.position() - lastPos);

        // Append the preserved quoted string
        result.append(*stringIt);

        // Update the last position
        lastPos = match.position() + match.length();
    }

    // Append the remaining part of the string after the last match
    result.append(str, lastPos, std::string::npos);

    return result;
}

// MARK: - Utills

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

/**
 * @brief Inserts a space after operator characters when followed by a unary minus,
 *        except in the case of the assignment operator `:=`.
 *
 * This function scans the input string and ensures that a space is inserted between
 * consecutive operator characters when the second is a minus (`-`). This helps
 * disambiguate unary minus usage in expressions like `a*-b`, transforming it to `a* -b`.
 *
 * A special exception is made for the `:=` operator, which is preserved without
 * inserting a space.
 *
 * @param input The input string potentially containing unary minus after operators.
 * @return A new string with appropriate spaces inserted to clarify unary minus usage.
 *
 * @note This function assumes a fixed set of operator characters: `+`, `-`, `*`, `/`, `=`, and `:`.
 *       It is particularly useful for preprocessing mathematical expressions to improve readability
 *       or prepare them for parsing.
 *
 * Example usage:
 * fixUnaryMinus("a*-b") returns "a* -b"
 * fixUnaryMinus("x:=y") returns "x:=y"
 */
std::string fixUnaryMinus(const std::string& input) {
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

static bool is_utf16(const std::string& filepath) {
    std::ifstream is;
    is.open(filepath, std::ios::in | std::ios::binary);
    if(!is.is_open()) {
        return false;
    }
    
    uint16_t byte_order_mark;
    
    is.read(reinterpret_cast<char*>(&byte_order_mark), sizeof(byte_order_mark));
    is.close();
    
    return byte_order_mark == 0xFEFF;
}

std::string minifiePrgm(std::ifstream &infile)
{
    // Read in the whole of the file into a `std::string`
    std::string str;
    std::wstring wstr;
    
    wstr = utf::read_utf16(infile);
    if (wstr.empty()) {
        char c;
        infile.seekg(0);
        while (!infile.eof()) {
            infile.get(c);
            str += c;
            infile.peek();
        }
    } else {
        str = utf::to_utf8(wstr);
    }
    
    auto python = extractPythonBlocks(str);
    str = blankOutPythonBlocks(str);
    
    auto strings = preserveStrings(str);
    str = blankOutStrings(str);
    
    str = replaceTabsWithSpaces(str);
    str = reduceMultipleSpaces(str);
    str = removeBlankLines(str);
    str = cleanWhitespace(str);
    str = replaceOperators(str);
    
    str = capitalizeWords(str, {
        "begin", "end", "return", "kill", "if", "then", "else", "xor", "or", "and", "not",
        "case", "default", "iferr", "ifte", "for", "from", "step", "downto", "to", "do",
        "while", "repeat", "until", "break", "continue", "export", "const", "local", "key"
    });
    str = capitalizeWords(str, {"log", "cos", "sin", "tan", "result", "min", "max"});
    str = replaceWords(str, {"FROM"}, ":=");
    str = fixUnaryMinus(str);
    str = removeNewlineAfterSemicolon(str);
    str = removeNewlineBeforeSymbols(str);
    
    str = restoreStrings(str, strings);
    str = separatePythonMarkers(str);
    str = restorePythonBlocks(str, python);
    

    std::regex re(R"((\bEXPORT )?([a-zA-Z]\w*)\([a-zA-Z,]*\)\s*(?=BEGIN\b))");
    std::sregex_iterator begin(str.begin(), str.end(), re);
    std::sregex_iterator end;
    std::string result = str;
    int fn = 0;
    
    for (auto it = begin; it != end; ++it) {
        std::smatch match = *it;

        std::string export_kw = match[1].str();  // optional "EXPORT "
        std::string function_name = match[2].str();
        
        if (export_kw.empty() && !preserveFunctionNames) {
            result = replaceWords(result, {function_name}, "fn" + base10ToBase32(fn++));
        }
    }
    return result;
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
        
        if ( args == "-o" ) {
            if ( n + 1 >= argc ) {
                error();
                exit(0);
            }
            out_filename = std::filesystem::expand_tilde(argv[++n]);
            continue;
        }
        
        if ( strcmp( argv[n], "-version" ) == 0 ) {
            version();
            return 0;
        }
        
        in_filename = std::filesystem::expand_tilde(argv[n]);
    }
    
    info();
    
    if (std::filesystem::path(in_filename).parent_path().empty()) {
        in_filename = in_filename.insert(0, "./");
    }
    std::filesystem::path path = in_filename;
    
    if (path.extension().empty()) {
        path.append(".prgm");
    }
    if (!std::filesystem::exists(path) || path.extension() == ".hpprgm") {
        error();
        return 0;
    }
    
    if (out_filename.empty())
        out_filename = path.parent_path().string() + "/" + path.stem().string() + "-min.prgm";
    

    // Start measuring time
    Timer timer;
    
    std::string str;

    std::ifstream infile;
    infile.open(in_filename, std::ios::in | std::ios::binary);
    if (!infile.is_open()) {
        error();
        return 0;
    }
    str = minifiePrgm(infile);
    infile.close();
    
    utf::save_as_utf16(out_filename, str);
    
    // Stop measuring time and calculate the elapsed time.
    long long elapsed_time = timer.elapsed();
    
    // Display elasps time in secononds.
    std::cout << "Completed in " << std::fixed << std::setprecision(2) << elapsed_time / 1e9 << " seconds\n";

    
    if (hasErrors() == true) {
        std::cout << "ERRORS!\n";
        remove(out_filename.c_str());
        return 0;
    }
    
//    if (!Singleton::shared()->aliases.descendingOrder && Singleton::shared()->aliases.verbose) {
//        Singleton::shared()->aliases.dumpIdentities();
//    }
    
    // Percentage Reduction = (Original Size - New Size) / Original Size * 100
    std::ifstream::pos_type original_size = file_size(in_filename);
    std::ifstream::pos_type new_size = file_size(out_filename);
    
    if (is_utf16(in_filename) == false) original_size = original_size * 2;
    
    
    // Create a locale with the custom comma-based numpunct
    std::locale commaLocale(std::locale::classic(), new comma_numpunct);
    std::cout.imbue(commaLocale);
    
    std::cout << "Reduction of " << (original_size - new_size) * 100 / original_size;
    std::cout << "%\n";
    
    std::cout << "File '" << regex_replace(out_filename, std::regex(R"(.*/)"), "") << "' succefuly created.\n";
    
    
    return 0;
}
