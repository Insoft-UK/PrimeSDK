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

#include "regexp.hpp"
#include "common.hpp"
#include "singleton.hpp"
#include "calc.hpp"

using namespace pp;

bool Regexp::parse(std::string &str) {
    std::regex re;
    std::smatch match;
    
    re = R"(^ *(?:@(global|local) )?\bregex +`([^`]*)` *(.*)$)";
    if (regex_search(str, match, re)) {
        TRegexp regexp = {
            .pattern = match[2].str(),
            .replacement = match[3].str(),
            .scopeLevel = static_cast<size_t>(Singleton::shared()->scopeDepth),
            .line = Singleton::shared()->currentLineNumber(),
            .pathname = Singleton::shared()->currentPath()
        };
        
        if (match[1].matched) {
            regexp.scopeLevel = match[1].str() == "global" ? 0 : 1;
        }
    
        if (regularExpressionExists(regexp.pattern)) return true;
        
        _regexps.push_back(regexp);
        str = std::string("");
        
        if (verbose) std::cout
            << MessageType::Verbose
            << "scope-level " << regexp.scopeLevel << ": "
            << "regex"
            << " '" << ANSI::Green << regexp.pattern << ANSI::Default << "' for '" << ANSI::Green << regexp.replacement << ANSI::Default << "' defined\n";
        return true;
    }
    
    return false;
}

void Regexp::removeAllOutOfScopeRegexps() {
    for (auto it = _regexps.begin(); it != _regexps.end(); ++it) {
        if (it->scopeLevel > Singleton::shared()->scopeDepth) {
            if (verbose) std::cout
                << MessageType::Verbose
                << "regex"
                << " '" << ANSI::Green << it->pattern << ANSI::Default << "' removed❗\n";
            
            _regexps.erase(it);
            removeAllOutOfScopeRegexps();
            break;
        }
    }
}

void Regexp::resolveAllRegularExpression(std::string &str) {
    std::smatch match;
    
    for (auto it = _regexps.begin(); it != _regexps.end(); ++it) {
        if (std::regex_search(str, match, std::regex(it->pattern))) {
            str = regex_replace(str, std::regex(it->pattern), it->replacement);
            str = std::regex_replace(str, std::regex("__SCOPE__"), std::to_string(Singleton::shared()->scopeDepth));
            Calc::evaluateMathExpression(str);
            resolveAllRegularExpression(str);
        }
    }
}

bool Regexp::regularExpressionExists(const std::string &pattern) {
    for (auto it = _regexps.begin(); it != _regexps.end(); ++it) {
        if (it->pattern == pattern) {
            std::cout
            << MessageType::Warning
            << "redefinition of: " << ANSI::Bold << pattern << ANSI::Default << ", ";
            if (basename(Singleton::shared()->currentPath()) == basename(it->pathname)) {
                std::cout << "previous definition on line " << it->line << "\n";
            }
            else {
                std::cout << "previous definition in '" << ANSI::Green << basename(it->pathname) << ANSI::Default << "' on line " << it->line << "\n";
            }
            return true;
        }
    }
    
    return false;
}



