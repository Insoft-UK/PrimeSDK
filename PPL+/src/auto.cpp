// The MIT License (MIT)
//
// Copyright (c) 2023-2025 Insoft. All rights reserved.
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


#include "auto.hpp"

#include <sstream>
#include <regex>
#include "singleton.hpp"

using pplplus::Auto;

static std::string base10ToBase32(unsigned int num) {
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
    reverse(result.begin(), result.end());
    
    return result;
}

static bool isValidPPLName(const std::string name) {
    std::regex re;
    std::string s = name;
    
    if (s.at(0) == '@') s.erase(remove(s.begin(), s.end(), '@'), s.end());
    if (s.at(0) == '_') return false;
    
    re = R"(^[A-Za-z]\w*(?:(::)|\.))";
    if (std::regex_search(s, re)) {
        return false;
    }
    
    return true;
}

std::string Auto::parse(const std::string& str) {
    std::smatch matches;
    std::regex re;
    
    if (Singleton::shared()->scopeDepth == 0) {
        re = std::regex(R"((?:EXPORT )?(?:([a-zA-Z]\w*):)?(@?[a-zA-Z_]\w*(?:::[a-zA-Z_]\w*)*)\(((?:[a-zA-Z]\w*(?::[a-zA-Z_]\w*)?)(?:,(?:[a-zA-Z]\w*(?::[a-zA-Z_]\w*)?))*)?\))", std::regex_constants::icase);
        if (regex_search(str, matches, re)) {
            if (matches.str(1).empty()) {
                if (isValidPPLName(matches.str(2))) return str;
                
                while (Singleton::shared()->aliases.realExists("fn" + base10ToBase32(++_count)));
                std::string output = str;
                output.insert(matches.position(2),  "fn" + base10ToBase32(_count) + ":");
                return output;
            }
        }
    }
    
    return str;
}




