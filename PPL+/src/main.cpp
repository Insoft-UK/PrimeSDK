// The MIT License (MIT)
//
// Copyright (c) 2023 Insoft. All rights reserved.
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
#include <cmath>
#include <sys/time.h>
#include <ctime>
#include <vector>
#include <iterator>
#include <cstdlib>
#include <iconv.h>
#include <unordered_set>

#include "timer.hpp"
#include "singleton.hpp"
#include "common.hpp"

#include "preprocessor.hpp"
#include "dictionary.hpp"
#include "alias.hpp"
//#include "strings.hpp"
#include "calc.hpp"
#include "hpprgm.hpp"
#include "utf.hpp"

#include "../version_code.h"

#define NAME "PPL+ Pre-Processor for PPL"
#define COMMAND_NAME "ppl+"
#define INDENT_WIDTH indentation

static unsigned int indentation = 2;

using pplplus::Singleton;
using pplplus::Aliases;
using pplplus::Alias;
using pplplus::Calc;
using pplplus::Dictionary;
using pplplus::Preprocessor;

using std::regex_replace;
using std::sregex_iterator;
using std::sregex_token_iterator;

namespace fs = std::filesystem;
namespace rc = std::regex_constants;

static Preprocessor preprocessor = Preprocessor();
static std::string assignment = "=";
static std::vector<std::string> operators = { ":=", "==", "▶", "≥", "≤", "≠", "-", "+", "*", "/" };

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
        T bitswap(T u)
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

void terminator() {
    std::cout << MessageType::CriticalError << "An internal pre-processing problem occurred. Please review the syntax before this point.\n";
    exit(0);
}
void (*old_terminate)() = std::set_terminate(terminator);

// MARK: - Helper Functions

/**
 * @brief Cleans up whitespace in a string while preserving word separation.
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
 * @brief Expands single `=` characters into `==` for comparison, preserving special cases.
 *
 * This function processes an input string and replaces single `=` characters with `==`,
 * which is commonly used for comparison in many programming languages. It handles the
 * following exceptions:
 *
 * - `:=` (colon-equals) is left unchanged, as it may represent assignment in some languages.
 * - `==` is preserved and not duplicated.
 *
 * This is useful for transforming assignment-style expressions into comparison-style
 * expressions for parsing or analysis.
 *
 * @param input The input string containing `=` characters.
 * @return A new string with applicable `=` characters expanded to `==`.
 *
 * @note This function assumes a language syntax where `==` is the comparison operator and
 *       `:=` has a distinct meaning. It does not perform full syntax validation.
 */
std::string expandAssignmentEquals(const std::string& input) {
    std::string output;
    output.reserve(input.size() * 2);  // Worst-case growth

    for (size_t i = 0; i < input.size(); ++i) {
        if (input[i] == '=') {
            // Check if preceded by ':' (:=), do not expand
            if (i > 0 && input[i - 1] == ':') {
                output += '=';
            }
            // Check if followed by '=' (already '=='), copy as-is
            else if (i + 1 < input.size() && input[i + 1] == '=') {
                output += "==";
                ++i;  // skip next '='
            }
            else {
                output += "==";
            }
        } else {
            output += input[i];
        }
    }

    return output;
}

/**
 * @brief Converts single assignment `=` operators to `:=`, preserving comparison and existing assignment syntax.
 *
 * This function scans the input string and replaces standalone `=` characters with `:=`,
 * often used in languages like Pascal or certain calculators to represent assignment.
 * It carefully handles the following cases:
 *
 * - `==` (equality comparison) is preserved and not modified.
 * - `:=` (already valid assignment syntax) is left unchanged.
 * - Any other standalone `=` is converted to `:=`.
 *
 * @param input The input string potentially containing assignment or comparison operators.
 * @return A new string with appropriate `=` characters converted to `:=`.
 *
 * @note Useful for translating code or expressions from languages that use `=` for assignment
 *       into those that use `:=`, while avoiding accidental changes to comparison or already-valid syntax.
 */
std::string convertAssignToColonEqual(const std::string& input) {
    std::string output;
    output.reserve(input.size() * 2);  // Conservative buffer size

    for (size_t i = 0; i < input.size(); ++i) {
        if (input[i] == '=') {
            // Check for '=='
            if (i + 1 < input.size() && input[i + 1] == '=') {
                output += "==";
                ++i;
            }
            // Check for ':=' (don't modify)
            else if (i > 0 && input[i - 1] == ':') {
                output += '=';
            }
            // Replace single '=' with ':='
            else {
                output += ":=";
            }
        } else {
            output += input[i];
        }
    }

    return output;
}

/**
 * @brief Normalizes spacing around operators in a string.
 *
 * This function ensures that operators from a predefined list are properly spaced
 * within the input string. It scans the string, identifies any known operators,
 * and inserts spaces before and after them if needed to ensure consistent formatting.
 *
 * After operator normalization, the function also removes any extra or redundant
 * whitespace, ensuring clean and readable output.
 *
 * @param input The input string that may contain operators with inconsistent spacing.
 * @return A new string with operators consistently spaced and extraneous whitespace removed.
 *
 * @note The function relies on a global or external list of `operators` (std::vector<std::string>)
 *       to define which sequences are considered operators.
 *
 * Example usage:
 * // Given operators = { "+", "-", "*", "/", "==", ">=", "<=", "=" };
 * normalizeOperators("a+  b==c") returns "a + b == c"
 */
std::string normalizeOperators(const std::string& input) {
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

/**
 * @brief Splits a string into a vector of substrings using commas as delimiters.
 *
 * This function tokenizes the input string based on commas (`,`) and returns
 * a vector containing each separated segment as an individual string.
 * Empty tokens between consecutive commas are preserved.
 *
 * @param input The input string to be split.
 * @return A vector of substrings split at each comma.
 *
 * @note Leading or trailing whitespace within tokens is not trimmed.
 *       Use additional processing if whitespace cleanup is needed.
 *
 * Example usage:
 * splitCommas("a,b,c") returns {"a", "b", "c"}
 * splitCommas("one,,three") returns {"one", "", "three"}
 */
std::vector<std::string> splitCommas(const std::string& input) {
    std::vector<std::string> result;
    std::stringstream ss(input);
    std::string token;

    while (getline(ss, token, ',')) {
        result.push_back(token);
    }

    return result;
}

/**
 * @brief Splits a string into substrings at commas, supporting escaped commas.
 *
 * This function tokenizes the input string by commas (`,`) while allowing
 * commas to be escaped using a backslash (`\\`). Escaped commas are treated
 * as literal characters and are included in the resulting token rather than
 * splitting the string at that point.
 *
 * - A sequence like `\\,` becomes a literal comma in the result.
 * - Other escape sequences (e.g. `\\n`) are preserved with the backslash.
 *
 * @param input The input string to be split.
 * @return A vector of strings split on unescaped commas.
 *
 * @note This function does not handle more advanced escaping rules like double escaping (`\\\\,`),
 *       and does not trim whitespace around tokens.
 *
 * Example usage:
 * splitEscapedCommas("one,two,three") returns {"one", "two", "three"}
 * splitEscapedCommas("one\\,with\\,commas,two") returns {"one,with,commas", "two"}
 * splitEscapedCommas("escaped\\\\slash,test") returns {"escaped\\slash", "test"}
 */
std::vector<std::string> splitEscapedCommas(const std::string& input) {
    std::vector<std::string> result;
    std::string token;
    bool escape = false;

    for (size_t i = 0; i < input.size(); ++i) {
        char c = input[i];

        if (escape) {
            // Handle escaped character
            if (c == ',') {
                token += ',';  // Turn \, into ,
            } else {
                token += '\\'; // Preserve the backslash
                token += c;
            }
            escape = false;
        }
        else if (c == '\\') {
            escape = true;
        }
        else if (c == ',') {
            result.push_back(token);
            token.clear();
        }
        else {
            token += c;
        }
    }

    result.push_back(token); // Add last token
    return result;
}

/**
 * @brief Processes escape sequences in a string and replaces them with corresponding characters.
 *
 * This function scans the input string for backslash-prefixed escape sequences and replaces
 * them with their corresponding characters or strings. Supported escape sequences include:
 *
 * - `\\n` → newline character (`\n`)
 * - `\\s` → space (`' '`)
 * - `\\t` → tab-like spacing (4 spaces)
 * - `\\i` → indentation (number of spaces defined by `INDENT_WIDTH`)
 *
 * Unrecognized escape sequences are preserved as-is (i.e., the backslash and the following character).
 *
 * @param input The input string potentially containing escape sequences.
 * @return A new string with escape sequences expanded into their corresponding characters.
 *
 * @note The `INDENT_WIDTH` constant must be defined elsewhere in the code to specify how many spaces `\\i` produces.
 *
 * Example usage:
 * processEscapes("Line\\nIndented\\iText") might return:
 * "Line\nIndented    Text" (with spaces determined by `INDENT_WIDTH`)
 */
std::string processEscapes(const std::string& input) {
    std::string result;
    for (size_t i = 0; i < input.length(); ++i) {
        if (input[i] != '\\' || i + 1 == input.length()) {
            result += input[i];
            continue;
        }
        char next = input[i + 1];
        if (next == 'n') {
            result += '\n';
            ++i;
            continue;
        }
        if (next == 's') {
            result += ' ';
            ++i;
            continue;
        }
        if (next == 't') {
            result += (std::string(4, ' '));
            ++i;
            continue;
        }
        if (next == 'i') {
            result += (std::string(INDENT_WIDTH, ' '));
            ++i;
            continue;
        }
        result += input[i]; // add the backslash
        result += next;     // add the next character as is
        ++i; // skip the next character
    }
    return result;
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

/**
 * @brief Inserts a single space after specified characters in a string.
 *
 * This function scans the input string and ensures that after every character
 * found in the provided set, there is exactly one space inserted. If spaces
 * already exist immediately after such characters, they are collapsed to a
 * single space. If there is no space, one is added.
 *
 * @param input The input string to process.
 * @param chars An unordered set of characters after which to insert a space.
 * @return A new string with spaces inserted as specified.
 *
 * @note Existing whitespace sequences following the specified characters are
 *       replaced with a single space.
 *
 * Example usage:
 * insertSpaceAfterChars("a,b;c", {',', ';'}) returns "a, b; c"
 */
std::string insertSpaceAfterChars(const std::string& input, const std::unordered_set<char>& chars) {
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

/**
 * @brief Inserts a space before a word that immediately follows a closing parenthesis without a space.
 *
 * This function scans the input string and inserts a space before any alphabetic character
 * that directly follows a closing parenthesis `')'` when there is no space in between.
 * This helps separate words that start immediately after a closing parenthesis.
 *
 * @param input The input string to process.
 * @return A new string with spaces inserted before words following a closing parenthesis.
 *
 * Example usage:
 * insertSpaceAfterClosingParen("foo(bar)baz") returns "foo(bar) baz"
 * insertSpaceAfterClosingParen("foo(bar) baz") returns "foo(bar) baz"
 */
std::string insertSpaceAfterClosingParen(const std::string& input) {
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

/**
 * @brief Extracts a single-line PPL comment from a given string.
 *
 * This function searches for the occurrence of a PPL  comment
 * marker (`//`) in the input string and returns the comment, excluding the `//`.
 * If no comment is found, an empty string is returned.
 *
 * @param str The input string potentially containing a PPL comment.
 * @return The extracted comment starting with `//`, or an empty string if none is found.
 */
std::string extractComment(const std::string &str) {
    std::string output;
    size_t pos = str.find("//");
    
    if (pos != std::string::npos) {
        output = str.substr(pos + 2, str.length() - pos - 2);
        trim(output);
    }
    return output;
}

/**
 * @brief Removes a PPL single-line comment from a line of source code.
 *
 * This function scans a string representing a line of PPL (HP Prime Programming Language)
 * source code and removes any single-line comment starting with `//`.
 * If no comment is found, the original string is returned unchanged.
 *
 * @param str A line of PPL source code.
 * @return The input line without the `//` comment, if present.
 *
 */

std::string removeComment(const std::string& str) {
    std::string output = str;
    size_t pos = str.find("//");
    if (pos != std::string::npos) {
        output.resize(pos);
    }
    
    return output;
}

/**
 * @brief Removes a triple-slash (`///`) comment from a line of source code.
 *
 * This function searches for the first occurrence of a `///` comment in the input
 * string and removes the comment portion. If a newline follows the comment, only the
 * comment part up to the newline is removed; otherwise, the remainder of the string
 * is removed. This is useful for stripping documentation-style comments (e.g., in PPL or C++).
 *
 * @param str A string potentially containing a `///` comment.
 * @return A copy of the input string with the `///` comment removed.
 *
 * @note This function does not remove standard `//` comments or handle multiline block comments.
 */
std::string removeTripleSlashComment(const std::string& str) {
    std::string output = str;
    
    size_t pos = output.find("///");
    if (pos == std::string::npos) return output;
    
    std::size_t newline = output.find('\n', pos);
    if (newline != std::string::npos) {
        // Remove from '///' up to (but not including) the newline
        output.erase(pos, newline - pos);
        return output;
    }
    
    // No newline after '///', remove till end of string
    output.erase(pos);
    
    return output;
}

/**
 * @brief Appends a comment to a line of code, ensuring proper spacing.
 *
 * Adds a PPL single-line comment (`// comment`) to the end of the given string,
 * making sure the comment starts with exactly one space after the last non-whitespace
 * character in the code.
 *
 * @param line The base line of code (without comment).
 * @param comment The comment text (without the leading slashes).
 * @return A new string with the comment appended properly.
 */
std::string applyComment(const std::string& line, const std::string& comment) {
    std::string trimmed = line;

    // Remove any trailing whitespace from the code line
    while (!trimmed.empty() && std::isspace(static_cast<unsigned char>(trimmed.back()))) {
        trimmed.pop_back();
    }

    // Append space + comment
    return trimmed + " // " + comment;
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

// MARK: - PPL+ To PPL Translater...
void reformatPPLLine(std::string& str) {
    std::regex re;
    
    auto strings = preserveStrings(str);
    str = blankOutStrings(str);
    
    std::string comment = extractComment(str);
    
    
    str = normalizeOperators(str);
    str = fixUnaryMinus(str);
    str = insertSpaceAfterChars(str, {','});
    str = insertSpaceAfterClosingParen(str);
    
    if (Singleton::shared()->scopeDepth > 0) {
        try {
            if (!regex_search(str, std::regex(R"(\b(BEGIN|IF|CASE|REPEAT|WHILE|FOR|ELSE|IFERR)\b)"))) {
                str.insert(0, std::string(Singleton::shared()->scopeDepth * INDENT_WIDTH, ' '));
            } else {
                str.insert(0, std::string((Singleton::shared()->scopeDepth - 1) * INDENT_WIDTH, ' '));
            }
        }
        catch (...) {
            std::cout << MessageType::CriticalError << "'" << str << "'\n";
            exit(0);
        }
        
        
        re = std::regex(R"(^ *(THEN)\b)", rc::icase);
        str = regex_replace(str, re, std::string((Singleton::shared()->scopeDepth - 1) * INDENT_WIDTH, ' ') + "$1");
        
      
        if (regex_search(str, std::regex(R"(\bEND;$)"))) {
            str = regex_replace(str, std::regex(R"(;(.+))"), ";\n" + std::string((Singleton::shared()->scopeDepth - 1) * INDENT_WIDTH, ' ') + "$1");
        } else {
            str = regex_replace(str, std::regex(R"(; *(.+))"), "; $1");
        }
    }
    
    if (Singleton::shared()->scopeDepth == 0) {
        str = regex_replace(str, std::regex(R"(END;)"), "$0\n");
        str = regex_replace(str, std::regex(R"(LOCAL )"), "");
    }
    
    
    str = regex_replace(str, std::regex(R"(([)};])([A-Z]))"), "$1 $2");
    
    re = R"(([^a-zA-Z ])(BEGIN|END|RETURN|KILL|IF|THEN|ELSE|XOR|OR|AND|NOT|CASE|DEFAULT|IFERR|IFTE|FOR|FROM|STEP|DOWNTO|TO|DO|WHILE|REPEAT|UNTIL|BREAK|CONTINUE|EXPORT|CONST|LOCAL|KEY))";
    str = regex_replace(str, re, "$1 $2");
    
    re = R"((BEGIN|END|RETURN|KILL|IF|THEN|ELSE|XOR|OR|AND|NOT|CASE|DEFAULT|IFERR|IFTE|FOR|FROM|STEP|DOWNTO|TO|DO|WHILE|REPEAT|UNTIL|BREAK|CONTINUE|EXPORT|CONST|LOCAL|KEY)([^a-zA-Z ;]))";
    str = regex_replace(str, re, "$1 $2");
    
    re = R"(([a-zA-Z]) +([{(]))";
    str = regex_replace(str, re, "$1$2");
    
    str = restoreStrings(str, strings);
    if (comment.length()) str = applyComment(str, comment);
}


std::string translatePPLPlusLine(const std::string& input, std::ofstream& outfile) {
    std::regex re;
    std::smatch match;
    std::ifstream infile;
    std::string output = input;
    
    // Remove any leading white spaces before or after.
    trim(output);
    
    output = preprocessor.parse(output);

    if (output.empty()) {
        return output;
    }
    
    if (output.substr(0,2) == "//") {
        output = output.insert(0, std::string(Singleton::shared()->scopeDepth * INDENT_WIDTH, ' '));
        output += '\n';
        return output;
    }
    

    /*
     While parsing the contents, strings may inadvertently undergo parsing, leading
     to potential disruptions in the string's content as well as comments.
     
     To address this issue, we prioritize the preservation of any existing strings
     and comments. After we prioritize the preservation of any existing strings and
     comments, we blank out the string/s.
     
     Subsequently, after parsing, any strings that have been blanked out can be
     restored to their original state.
     */
    auto strings = preserveStrings(output);
    output = blankOutStrings(output);
 
    std::string comment = extractComment(output);
    output = removeComment(output);
    
    output = cleanWhitespace(output);
    output = replaceOperators(output);
    
    // PPL by default uses := instead of C's = for assignment. Converting all = to PPL style :=
    if (assignment == "=") {
        output = convertAssignToColonEqual(output);
    } else {
        output = expandAssignmentEquals(output);
    }
    
    output = Singleton::shared()->aliases.resolveAllAliasesInText(output);
    
    /*
     A code stack provides a convenient way to store code snippets
     that can be retrieved and used later.
     */
    output = Singleton::shared()->codeStack.parse(output);

    
    if (Dictionary::isDictionaryDefinition(output)) {
        Dictionary::proccessDictionaryDefinition(output);
        output = Dictionary::removeDictionaryDefinition(output);
    }
    if (output.empty()) return output;
    
    // Keywords
    output = capitalizeWords(output, {
        "begin", "end", "return", "kill", "if", "then", "else", "xor", "or", "and", "not",
        "case", "default", "iferr", "ifte", "for", "from", "step", "downto", "to", "do",
        "while", "repeat", "until", "break", "continue", "export", "const", "local", "key"
    });
    
    output = capitalizeWords(output, {"log", "cos", "sin", "tan", "ln", "min", "max"});

    
    //MARK: User Define Alias Parsing
    
    re = R"(^alias\b *(@)?([A-Za-z_]\w*(?:::[a-zA-Z]\w*)*):=([a-zA-Z→][\w→]*(?:\.[a-zA-Z→][\w→]*)*);$)";
    if (regex_search(output, match, re)) {
        Aliases::TIdentity identity;
        identity.identifier = match[2].str();
        identity.real = match[3].str();
        identity.type = Aliases::Type::Alias;
        identity.scope = match[1].matched ? 0 : Singleton::shared()->scopeDepth;
        
        Singleton::shared()->aliases.append(identity);
        output = "";
        return output;
    }
    
   
    
    re = R"(\b(BEGIN|IF|FOR|CASE|REPEAT|WHILE|IFERR)\b)";
    for(auto it = sregex_iterator(output.begin(), output.end(), re); it != sregex_iterator(); ++it) {
        Singleton::shared()->increaseScopeDepth();
    }
    
    re = R"(\b(END|UNTIL)\b)";
    for(auto it = sregex_iterator(output.begin(), output.end(), re); it != sregex_iterator(); ++it) {
        Singleton::shared()->decreaseScopeDepth();
        if (Singleton::shared()->scopeDepth == 0) {
            output += '\n';
        }
       
        Singleton::shared()->aliases.removeAllOutOfScopeAliases();
        Singleton::shared()->regexp.removeAllOutOfScopeRegexps();
    }
    
    
    if (Singleton::shared()->scopeDepth == 0) {
        re = R"(^ *(KS?A?_[A-Z\d][a-z]*) *$)";
        sregex_token_iterator it = sregex_token_iterator {
            output.begin(), output.end(), re, {1}
        };
        if (it != sregex_token_iterator()) {
            std::string s = *it;
            output = "KEY " + s + "()";
        }
    }
    
    output = Singleton::shared()->autoname.parse(output);
    output = Alias::parse(output);
    output = Calc::parse(output);
    
    reformatPPLLine(output);
    output = processEscapes(output);
   
    output = restoreStrings(output, strings);
    
    if (comment.length()) output = applyComment(output, comment);
    
    
    output += '\n';
    return output;
}



void loadRegexLib(const fs::path path, const bool verbose) {
    std::string utf8;
    std::ifstream infile;
    
    infile.open(path, std::ios::in);
    if (!infile.is_open()) {
        return;
    }
    
    if (verbose) std::cout << "Library " << (path.filename() == ".base.re" ? "base" : path.stem()) << " successfully loaded.\n";
    
    while (getline(infile, utf8)) {
        utf8.insert(0, "regex ");
        Singleton::shared()->regexp.parse(utf8);
    }
    
    infile.close();
}

void loadRegexLibs(const std::string path, const bool verbose) {
    loadRegexLib(path + "/.base.re", verbose);
    
    try {
        for (const auto& entry : fs::directory_iterator(path)) {
            if (fs::path(entry.path()).extension() != ".re" || fs::path(entry.path()).filename() == ".base.re") {
                continue;
            }
            loadRegexLib(entry.path(), verbose);
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << "error: " << e.what() << '\n';
    }
}



std::string embedPPLCode(const std::string& filepath) {
    std::ifstream is;
    std::string str;
    
    fs::path path = filepath;
    is.open(filepath, std::ios::in);
    if (!is.is_open()) return str;
    if (path.extension() == ".hpprgm" || path.extension() == ".prgm") {
        std::wstring wstr = hpprgm::load(filepath);
        
        if (!wstr.empty()) {
            str = utf::to_utf8(wstr);
            str = regex_replace(str, std::regex(R"(^ *#pragma mode *\(.+\) *\n+)"), "");
            is.close();
            return str;
        }
    }
    std::string line;
    while (getline(is, line)) {
        line += '\n';
        str += line;
    }
    is.close();
    return str;
}

bool verbose(void) {
    if (Singleton::shared()->aliases.verbose) return true;
    if (preprocessor.verbose) return true;
    
    return false;
}

enum BlockType
{
    BlockType_Python, BlockType_PPL, BlockType_PrimePlus
};

bool isPythonBlock(const std::string str) {
    return str.find("#PYTHON") != std::string::npos;
}

bool isPPLBlock(const std::string str) {
    return str.find("#PPL") != std::string::npos;
}

void processPPLBlock(std::ifstream& infile, std::ofstream& outfile) {
    std::string str;
    
    Singleton::shared()->incrementLineNumber();
    
    while(getline(infile, str)) {
        if (str.find("#END") != std::string::npos) {
            Singleton::shared()->incrementLineNumber();
            return;
        }
        
        str.append("\n");
        utf::write(str, outfile);
        Singleton::shared()->incrementLineNumber();
    }
}

void processPythonBlock(std::ifstream& infile, std::ofstream& outfile, std::string& input) {
    std::regex re;
    std::string str;
    Aliases aliases;
    aliases.verbose = Singleton::shared()->aliases.verbose;
    
    Singleton::shared()->incrementLineNumber();
    
    input = cleanWhitespace(input);

    size_t start = input.find('(');
    size_t end = input.find(')', start);
        
    if (start != std::string::npos && end != std::string::npos && end > start) {
        std::vector<std::string> arguments = splitCommas(input.substr(start + 1, end - start - 1));
        input = "#PYTHON (";
        int index = 0, n = 0;
        
        Aliases::TIdentity identity = {
            .type = Aliases::Type::Argument
        };
        for (const std::string& argument : arguments) {
            if (index++) input.append(",");
            start = argument.find(':');
            
            if (start != std::string::npos) {
                input.append(argument.substr(0, start));
                identity.identifier = argument.substr(start + 1, argument.length() - start - 1);
                identity.real = "argv[" + std::to_string(++n) + "]";
                aliases.append(identity);
                continue;
            }
            input.append(argument);
        }
        input.append(")");
    }
        
    
    utf::write(input + '\n', outfile);
    std::smatch match;

    while(getline(infile, str)) {
        if (str.find("#END") != std::string::npos) {
            utf::write("#END\n", outfile);
            Singleton::shared()->incrementLineNumber();
            return;
        }
        
        Singleton::shared()->incrementLineNumber();
        str = aliases.resolveAllAliasesInText(str);
        
        // alias aliasname as realname
        re = R"(^ *alias +([A-Za-z_]\w*) *as *([a-zA-Z][\w\[\]]*) *$)";
        if (regex_search(str, match, re)) {
            Aliases::TIdentity identity;
            identity.identifier = match[1].str();
            identity.real = match[2].str();
            identity.type = Aliases::Type::Alias;
            identity.scope = -1;
            
            aliases.append(identity);
            str = "";
            continue;
        }
        
        str.append("\n");
        utf::write(str, outfile);
    }
}

void translatePPLPlusToPPL(const fs::path& path, std::ofstream& outfile) {
    Singleton& singleton = *Singleton::shared();
    std::ifstream infile;
    std::regex re;
    std::string input;
    std::string output;
    std::smatch match;

    bool pragma = false;
    
    singleton.pushPath(path);
    
    infile.open(path,std::ios::in);
    if (!infile.is_open()) {
        return;
    }
    
    while (getline(infile, input)) {
        /*
         Handle any escape lines `\` by continuing to read line joining them all up as one long line.
         */
        
        if (!input.empty()) {
            while (input.at(input.length() - 1) == '\\' && !input.empty()) {
                input.resize(input.length() - 1);
                std::string s;
                getline(infile, s);
                input.append(s);
                Singleton::shared()->incrementLineNumber();
                if (s.empty()) break;
            }
        }
        
        input = removeTripleSlashComment(input);
        
        if (input.find("#EXIT") != std::string::npos) {
            break;
        }
        
        while (preprocessor.disregard == true) {
            input = preprocessor.parse(input);
            Singleton::shared()->incrementLineNumber();
            getline(infile, input);
        }
        
        if (isPythonBlock(input)) {
            processPythonBlock(infile, outfile, input);
            continue;
        }
        
        if (isPPLBlock(input)) {
            processPPLBlock(infile, outfile);
            continue;
        }
        
        // Handle `#pragma mode` for PPL+
        if (input.find("#pragma mode") != std::string::npos) {
            if (pragma) {
                Singleton::shared()->incrementLineNumber();
                continue;
            }
            pragma = true;
            re = R"(([a-zA-Z]\w*)\(([^()]*)\))";
            std::string s = input;
            input = "#pragma mode( ";
            for(auto it = sregex_iterator(s.begin(), s.end(), re); it != sregex_iterator(); ++it) {
                if (it->str(1) == "assignment") {
                    if (it->str(2) != ":=" && it->str(2) != "=") {
                        std::cout << MessageType::Warning << "#pragma mode: for '" << it->str() << "' invalid.\n";
                    }
                    if (it->str(2) == ":=") assignment = ":=";
                    if (it->str(2) == "=") assignment = "=";
                    continue;
                }
                if (it->str(1) == "indentation") {
                    indentation = atoi(it->str(2).c_str());
                    continue;
                }
                if (it->str(1) == "operators") {
                    operators = splitEscapedCommas(cleanWhitespace(it->str(2)));
                    continue;
                }
                input.append(it->str() + " ");
            }
            input.append(")");
            
            utf::write(input + "\n", outfile);
            Singleton::shared()->incrementLineNumber();
            continue;
        }
        
        
        if (preprocessor.isQuotedInclude(input)) {
            Singleton::shared()->incrementLineNumber();
            
            std::filesystem::path path = preprocessor.extractIncludePath(input);
            if (path.parent_path().empty() && fs::exists(path) == false) {
                path = singleton.getMainSourceDir().string() + "/" + path.filename().string();
            }
            if (path.extension() == ".hpprgm" || path.extension() == ".prgm") {
                output += embedPPLCode(path.string());
                continue;
            }
            if (!(fs::exists(path))) {
                std::cout << MessageType::Verbose << path.filename() << " file not found\n";
            } else {
                translatePPLPlusToPPL(path.string(), outfile);
            }
            continue;
        }
        
        if (preprocessor.isAngleInclude(input)) {
            Singleton::shared()->incrementLineNumber();
            std::filesystem::path path = preprocessor.extractIncludePath(input);
            if (path.extension().empty()) {
                path.replace_extension(".prgm+");
            }
            for (fs::path systemIncludePath : preprocessor.systemIncludePath) {
                if (fs::exists(systemIncludePath.string() + "/" + path.filename().string())) {
                    path = systemIncludePath.string() + "/" + path.filename().string();
                    break;
                }
            }
            if (!(fs::exists(path))) {
                std::cout << MessageType::Verbose << path.filename() << " file not found\n";
            } else {
                translatePPLPlusToPPL(path.string(), outfile);
            }
            continue;
        }
        
        
        if (Singleton::shared()->regexp.parse(input)) {
            input = regex_replace(input, std::regex(R"(^ *\bregex +([@<>=≠≤≥~])?`([^`]*)`(i)? *(.*)$)"), "");
        }
        
        Singleton::shared()->regexp.resolveAllRegularExpression(input);
    
        /*
         We need to perform pre-parsing to ensure that, in lines using subtitued
         PPL+ keywords for likes of END, IFERR and THEN are resolved.
         */
        input = replaceWords(input, {"endif", "wend", "next"}, "END");
        input = replaceWords(input, {"try"}, "IFERR");
        input = replaceWords(input, {"catch"}, "THEN");
  
        /*
         We need to perform pre-parsing to ensure that, in lines such as if
         condition then statement/s end;, the statement/s and end; are not on
         the same line. This ensures proper indentation can be applied during
         the reformatting stage of PPL code.
         */
        
        input = regex_replace(input, std::regex(R"(\b(THEN|IFERR|REPEAT)\b)", rc::icase), "$1\n");
        input = regex_replace(input, std::regex(R"((; *)(THEN|UNTIL)\b)", rc::icase), "$1\n$2");
        input = regex_replace(input, std::regex(R"(; *\b(ELSE)\b)", rc::icase), ";\n$1\n");
        input = regex_replace(input, std::regex(R"(; *(END|UNTIL|ELSE|LOCAL|CONST)\b;)", rc::icase), ";\n$1;");
        input = regex_replace(input, std::regex(R"((.+)\bBEGIN\b)", rc::icase), "$1\nBEGIN");
        
        
        std::istringstream iss;
        iss.str(input);
        std::string str;
        
        while(getline(iss, str)) {
            output.append(translatePPLPlusLine(str, outfile));
        }
        
        Singleton::shared()->incrementLineNumber();
    }
    
    utf::write(output, outfile);
    
    infile.close();
    singleton.popPath();
}


// MARK: - Command Line
void version(void) {
    using namespace std;
    std::cout
    << "Copyright (C) 2023-" << YEAR << " Insoft. All rights reserved.\n"
    << "Insoft "<< NAME << " version, " << VERSION_NUMBER << " (BUILD " << VERSION_CODE << ")\n"
    << "Built on: " << DATE << "\n"
    << "Licence: MIT License\n\n"
    << "For more information, visit: http://www.insoft.uk\n";
}

void error(void) {
    std::cout << COMMAND_NAME << ": try '" << COMMAND_NAME << " --help' for more information\n";
    exit(0);
}

void info(void) {
    using namespace std;
    std::cout
    << "          ***********     \n"
    << "        ************      \n"
    << "      ************        \n"
    << "    ************  **      \n"
    << "  ************  ******    \n"
    << "************  **********  \n"
    << "**********    ************\n"
    << "************    **********\n"
    << "  **********  ************\n"
    << "    ******  ************  \n"
    << "      **  ************    \n"
    << "        ************      \n"
    << "      ************        \n"
    << "    ************          \n\n"
    << "Copyright (C) 2023-" << YEAR << " Insoft. All rights reserved.\n"
    << "Insoft PPL+" << std::string(YEAR).substr(2) << " Pre-Processor for PPL\n\n";
}

void help(void) {
    using namespace std;
    std::cout
    << "Copyright (C) 2023-" << YEAR << " Insoft. All rights reserved.\n"
    << "Insoft "<< NAME << " version, " << VERSION_NUMBER << " (BUILD " << VERSION_CODE << ")\n"
    << "\n"
    << "Usage: " << COMMAND_NAME << " <input-file> [-o <output-file>] [-v <flags>]\n"
    << "\n"
    << "Options:\n"
    << "  -o <output-file>        Specify the filename for generated PPL code.\n"
    << "  -v                      Display detailed processing information.\n"
    << "\n"
    << "  Verbose Flags:\n"
    << "     a                    Aliases\n"
    << "     p                    Preprocessor\n"
    << "     r                    Regular Expression\n"
    << "\n"
    << "Additional Commands:\n"
    << "  " << COMMAND_NAME << " {--version | --help }\n"
    << "    --version              Display the version information.\n"
    << "    --help                 Show this help message.\n";
}


// MARK: - Main
int main(int argc, char **argv) {
    
    std::string in_filename, out_filename;
    
    if (argc == 1) {
        error();
        exit(100);
    }
    
    bool verbose = false;
    bool showpath = false;
    
    
    std::string args(argv[0]);
    
    for (int n = 1; n < argc; n++) {
        args = argv[n];
        
        if (args == "-o") {
            if ( n + 1 >= argc ) {
                error();
                exit(101);
            }
            out_filename = argv[n + 1];
            if (fs::path(out_filename).extension().empty()) {
                out_filename.append(".hpprgm");
            }
            
            n++;
            continue;
        }
        
    
        if ( args == "--help" ) {
            help();
            return 0;
        }
        
        
        if ( strcmp( argv[n], "--version" ) == 0 ) {
            version();
            return 0;
        }
        
        
        if (args.starts_with("-v=")) {
            if (args.find("a") != std::string::npos) Singleton::shared()->aliases.verbose = true;
            if (args.find("p") != std::string::npos) preprocessor.verbose = true;
            if (args.find("r") != std::string::npos) Singleton::shared()->regexp.verbose = true;
            if (args.find("l") != std::string::npos) verbose = true;
                
            continue;
        }
        
        if (args.starts_with("-I")) {
            preprocessor.systemIncludePath.push_front(fs::path(args.substr(2)).has_filename() ? fs::path(args.substr(2)) : fs::path(args.substr(2)).parent_path());
            continue;
        }
        
        if (args == "--path") {
            showpath = true;
            continue;
        }
        
        in_filename = fs::expand_tilde(argv[n]);
        std::regex re(R"(.\w*$)");
    }
    
    
    
    if (!fs::exists(in_filename)) {
        if (fs::exists(in_filename + ".prgm+")) in_filename.append(".prgm+");
    }
    if (!fs::exists(in_filename)) {
        error();
        return 0;
    }
    
    if (out_filename.empty()) {
        out_filename = fs::path(in_filename).stem().string() + ".prgm";
    }
    if (fs::path(out_filename).parent_path().empty()) {
        out_filename.insert(0, fs::path(in_filename).parent_path().string() + "/");
    }
    out_filename = fs::expand_tilde(out_filename);
    
    info();
    
    
    std::ofstream outfile;
    outfile.open(out_filename, std::ios::out | std::ios::binary);
    if(!outfile.is_open())
    {
        error();
        return 0;
    }

    if (fs::path(out_filename).extension() != ".hpprgm") {
        // The ".ppl" file format requires UTF16-LE.
        outfile.put(0xFF);
        outfile.put(0xFE);
    } else {
        outfile.seekp(20, std::ios::beg);
    }
    
      
    
    // Start measuring time
    Timer timer;
    
    std::string str;
    
    str = "#define __pplplus";
    preprocessor.parse(str);
    
    str = R"(#define __LIST_LIMIT 10000)";
    preprocessor.parse(str);
    
    str = R"(#define __VERSION )" + std::to_string(NUMERIC_BUILD / 100);
    preprocessor.parse(str);
    
    str = R"(#define __NUMERIC_BUILD )" + std::to_string(NUMERIC_BUILD);
    preprocessor.parse(str);
    
#ifdef DEBUG
    loadRegexLibs(fs::expand_tilde("~/GitHub/PrimeSDK/Package Installer/package-root/Applications/HP/PrimeSDK/lib"), true);
#else
    loadRegexLibs("/Applications/HP/PrimeSDK/lib", verbose);
#endif
    
    
    translatePPLPlusToPPL(in_filename, outfile);
    
    if (fs::path(out_filename).extension() == ".hpprgm") {
        // Get the current file size.
        std::streampos currentPos = outfile.tellp();
        
        // Code size will be the current filesize - the header size + the two aditional bytes.
        uint32_t codeSize = static_cast<uint32_t>(currentPos) - 20 + 2;

        // Seek to the beginning to write the header.
        outfile.seekp(0, std::ios::beg);

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

        // CODE HEADER
        /**
         0x0000-0x0003: Size of the PPL Code in UTF-16 LE
         */
#ifndef __LITTLE_ENDIAN__
        codeSize = swap_endian(codeSize);
#endif
        outfile.write(reinterpret_cast<const char*>(&codeSize), sizeof(codeSize));
        
        /**
         0x0004-0x----: Code in UTF-16 LE until 00 00
         */
        outfile.seekp(0, std::ios::end);
        outfile.put(0x00);
        outfile.put(0x00);
    }
    outfile.close();
    
    if (hasErrors() == true) {
        std::cout << ANSI::Red << "ERRORS!" << ANSI::Default << "\n";
        remove(out_filename.c_str());
        return 0;
    }
    
    // Stop measuring time and calculate the elapsed time.
    long long elapsed_time = timer.elapsed();
    
    // Display elasps time in secononds.
    if (elapsed_time / 1e9 < 1.0) {
        std::cout << "Completed in " << std::fixed << std::setprecision(2) << elapsed_time / 1e6 << " milliseconds\n";
    } else {
        std::cout << "Completed in " << std::fixed << std::setprecision(2) << elapsed_time / 1e9 << " seconds\n";
    }
    if (fs::path(out_filename).extension() != ".hpprgm")
        std::cout << "UTF-16LE file ";
    else
        std::cout << "File ";
    if (showpath)
        std::cout << "at \"" << out_filename << "\" succefuly created.\n";
    else
        std::cout << fs::path(out_filename).filename() << " succefuly created.\n";
            
    return 0;
}
