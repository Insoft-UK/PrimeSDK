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

#include "strings.hpp"
#include <regex>

using namespace ppl;

void Strings::preserveStrings(const std::string& str) {
    std::regex re;
    
    re = R"("[^"]*")";
    for (auto it = std::sregex_iterator(str.begin(), str.end(), re); it != std::sregex_iterator(); ++it ) {
        _preservedStrings.push_back(it->str());
    }
}

void Strings::blankOutStrings(std::string &str) {
    std::regex re;
    
    re = R"("[^"]*")";
    str = regex_replace(str, re, R"("")");
}

void Strings::restoreStrings(std::string& str) {
    const std::regex re(R"("[^"]*")");

    // If there are no preserved strings, return early
    if (_preservedStrings.empty()) return;

    std::string result;
    auto inserter = std::back_inserter(result);

    auto it = std::sregex_iterator(str.begin(), str.end(), re);
    auto end = std::sregex_iterator();

    // Track the position after the last match
    std::sregex_iterator last_iter = it;

    for (; it != end && !_preservedStrings.empty(); ++it) {
        // Append text before the match
        inserter = std::copy(it->prefix().first, it->prefix().second, inserter);
        
        // Insert the next preserved string
        inserter = std::copy(_preservedStrings.front().begin(), _preservedStrings.front().end(), inserter);
        _preservedStrings.pop_front();  // Remove the used preserved string

        last_iter = it;
    }

    // Handle the text after the last match
    if (last_iter != end) {
        inserter = std::copy(last_iter->suffix().first, last_iter->suffix().second, inserter);
    } else {
        // If no matches were found, copy the entire string
        result = str;
    }

    // Update the input string with the modified result
    str = result;
}
