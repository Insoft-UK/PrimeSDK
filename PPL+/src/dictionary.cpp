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

using pplplus::Dictionary;

bool Dictionary::isDictionaryDefinition(const std::string &str) {
    return regex_search(str, std::regex(R"(\bdict +([\w[\],:=#-]+) *@?([A-Za-z_]\w*(?:::[A-Za-z_]\w*)*);)"));
}

std::string Dictionary::removeDictionaryDefinition(const std::string& str) {
    return std::regex_replace(str, std::regex(R"(\bdict +([\w[\],:=#-]+) *@?([A-Za-z_]\w*(?:::[A-Za-z_]\w*)*);)"), "");
}

bool Dictionary::proccessDictionaryDefinition(const std::string &str) {
    std::regex re;
    std::smatch match;
    std::string code;
    
    code = str;
    
    Aliases::TIdentity identity;
    identity.scope = Singleton::shared()->scopeDepth;
    identity.type = Aliases::Type::Alias;
    
    re = R"(\bdict +([\w[\],:=#-]+) *(@)?([A-Za-z_]\w*(?:::[A-Za-z_]\w*)*);)";
    if (regex_search(code, match, re)) {
        
        identity.scope = match[2].matched ? 0 : Singleton::shared()->scopeDepth;

        re = R"(([a-zA-Z]\w*(?:(?:[a-zA-Z_]\w*)|(?:::)|\.)*)(?:(\[#?[\dA-F]+(?::-?\d{0,2}[bodh])?(?:,#?[\dA-F]+(?::-?\d{0,2}[bodh])?)*\])|(?::=(#?[\dA-F]+(?::-?\d{0,2}[bodh])?)))?)";
        std::string s = match[1].str();
        for (auto it = std::sregex_iterator(s.begin(), s.end(), re); it != std::sregex_iterator(); it++) {
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
