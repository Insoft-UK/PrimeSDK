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

#include "dictionary.hpp"

using namespace ppl_plus;

static void removeUnnecessaryWhitespace(std::string &str) {
    // Regular expression pattern to match spaces around the specified operators
    // Operators: {}[]()≤≥≠<>=*/+-▶.,;:!^
    std::regex re(R"(\s*([{}[\]()≤≥≠<>=*\/+\-▶.,;:!^&|%])\s*)");
    
    // Replace matches with the operator and no surrounding spaces
    str = std::regex_replace(str, re, "$1");
    
    auto pos = str.find_last_of("]");
    if (pos != std::string::npos) str.insert(pos + 1, " ");
    
    return;
}

static bool isGlobalDictionary(const std::string &str) {
    return regex_search(str, std::regex("^@global +dict (.*); *$"));
}

bool Dictionary::isDictionary(const std::string &str) {
    return regex_search(str, std::regex("^(@global )? *dict (.*); *$"));
}

bool Dictionary::proccessDictionary(const std::string &str) {
    std::regex re;
    std::smatch match;
    std::string s1, s2;
    
    
    
    s1 = str;
    removeUnnecessaryWhitespace(s1);
    
    Aliases::TIdentity identity;
    identity.scope = Aliases::Scope::Auto;
    identity.type = Aliases::Type::Alias;
    
    
    re = R"(^(@global )?dict +(.+) ((?:`[\w.:]+`)|[\w.:]+)(?:\(([A-Za-z_ ,]+)\))?;$)";
    if (regex_search(s1, match, re)) {
        identity.scope = match[1].matched ? Aliases::Scope::Global : Aliases::Scope::Auto;
        
        re = R"(([a-zA-Z]\w*(?:(?:[a-zA-Z_]\w*)|(?:::)|\.)*)(?:(\[#?[\dA-F]+(?::-?\d{0,2}[bodh])?(?:,#?[\dA-F]+(?::-?\d{0,2}[bodh])?)*\])|(?:=(#?[\dA-F]+(?::-?\d{0,2}[bodh])?)))?)";
        s2 = match[2].str();
        for (auto it = std::sregex_iterator(s2.begin(), s2.end(), re); it != std::sregex_iterator(); it++) {
            identity.identifier = match[3].str() + "." + it->str(1);
            
        
            if (!it->str(2).empty()) {
                // List
                identity.real = match[3].str() + it->str(2);
            }
            if (!it->str(3).empty()) {
                identity.real = it->str(3);
            }
            
            if (it->str(2).empty() && it->str(3).empty()) {
                identity.real = match[3].str();
            }
            
            Singleton::shared()->aliases.append(identity);
        }
        return true;
    }
    return false;
}
