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

using namespace ppl_plus;

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
    
    if (name.at(0) == '_') return false;
    
    re = R"(^[A-Za-z]\w*(?:(::)|\.))";
    if (std::regex_search(name, re)) {
        return false;
    }
    
    return true;
}

// This function will examine any variable name thats not a valid PPL variable name and asign an auto: prefix to it.
static void inferredAutoForVariableName(std::string &ln) {
    std::regex re;
    
    re = std::regex(R"(\b((?:LOCAL|CONST) +)(.*)(?=;))", std::regex_constants::icase);
    std::sregex_token_iterator it = std::sregex_token_iterator {
        ln.begin(), ln.end(), re, {1, 2}
    };
    if (it != std::sregex_token_iterator()) {
        std::string code = *it++;
        std::string str = *it;
        
        re =  R"([^,;]+)";
        for (auto it = std::sregex_iterator(str.begin(), str.end(), re);;) {
            std::string s = trim_copy(it->str());
            if (regex_search(s, std::regex(R"(^[a-zA-Z]\w*:[a-zA-Z])"))) {
                code.append(s);
            }
            else {
                if (!isValidPPLName(s)) {
                    s.insert(0, "auto:");
                }
                code.append(s);
            }
            
            if (++it == std::sregex_iterator()) break;
            code.append(",");
        }
        ln = regex_replace(ln, std::regex(R"(\b((?:LOCAL|CONST) +)(.*)(?=;))", std::regex_constants::icase), code);
    }
}

bool Auto::parse(std::string &str) {
    std::smatch match;
    std::regex re;
    size_t pos;
    Singleton *singleton = Singleton::shared();
    
    inferredAutoForVariableName(str);
    
    
    re = R"(^(auto +)([^:=]*)(?=;))";
    if (regex_search(str, match, re)) {
        str.replace(match.position(1), match.str(1).length(), "LOCAL ");
        
        re = R"(([a-zA-Z_]\w*) *(?=;|,))";
        str = regex_replace(str, re, "auto:$1");
    }
    
    if (singleton->scopeDepth == 0) {
        re = std::regex(R"(\b(LOCAL|CONST) +)", std::regex_constants::icase);
        if (regex_search(str, match, re)) {
            while ((pos = str.find("auto:")) != std::string::npos) {
                str.erase(pos, 4);
                while (singleton->aliases.realExists("g" + base10ToBase32(++_globalCount)));
                str.insert(pos, "g" + base10ToBase32(_globalCount));
            }
        }

        re = R"(\bauto *(?=: *(?:[A-Za-z_][\w:.]*) *(?=\()))";
        if (regex_search(str, match, re)) {
            while (singleton->aliases.realExists("fn" + base10ToBase32(++_fnCount)));
            str.replace(match.position(), match.str().length(), "fn" + base10ToBase32(_fnCount));
        }
        
        _paramCount = 0;
        _varCount = 0;
        while ((pos = str.find("auto:")) != std::string::npos) {
            while (singleton->aliases.realExists("p" + base10ToBase32(++_paramCount)));
            str.replace(pos, 4, "p" + base10ToBase32(_paramCount));
        }
    }
    
    
    // Variables/Constants
    re = std::regex(R"(\b(LOCAL|CONST) +)", std::regex_constants::icase);
    if (regex_search(str, match, re)) {
        while ((pos = str.find("auto:")) != std::string::npos) {
            str.erase(pos, 4);
            while (singleton->aliases.realExists("v" + base10ToBase32(++_varCount)));
            str.insert(pos, "v" + base10ToBase32(_varCount));
        }
    }
    
    return true;
}




