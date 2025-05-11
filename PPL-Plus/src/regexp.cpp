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

using namespace ppl_plus;

bool Regexp::parse(std::string &str) {
    std::regex re;
    std::smatch match;
    
    re = R"(^ *\bregex +(@)?`([^`]*)` *(.*)$)";
    if (regex_search(str, match, re)) {
        TRegexp regexp = {
            .pattern = match[2].str(),
            .replacement = match[3].str(),
            .scopeLevel = match[1].matched ? 0 : static_cast<size_t>(Singleton::shared()->scopeDepth),
            .line = Singleton::shared()->currentLineNumber(),
            .pathname = Singleton::shared()->currentPath()
        };
    
        str = std::string("");
        if (regularExpressionExists(regexp.pattern)) return true;
        
        _regexps.push_back(regexp);
        if (verbose) std::cout
            << MessageType::Verbose
            << "Defined " << (regexp.scopeLevel ? "local " : "") << "regular expresion "
            << "`" << ANSI::Green << regexp.pattern << ANSI::Default << "`\n";
        return true;
    }
    
    return false;
}

void Regexp::removeAllOutOfScopeRegexps() {
    for (auto it = _regexps.begin(); it != _regexps.end(); ++it) {
        if (it->scopeLevel > Singleton::shared()->scopeDepth) {
            if (verbose) std::cout
                << MessageType::Verbose
                << "Removed " << (it->scopeLevel ? "local " : "") <<"regular expresion `" << ANSI::Green << it->pattern << ANSI::Default << "`\n";
            
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
            << "Regular expresion already defined. Previous definition at " << basename(it->pathname) << ":" << it->line << "\n";
            return true;
        }
    }
    
    return false;
}



