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


#include "comments.hpp"
#include "common.hpp"

#include <iostream>
#include <regex>

using namespace pp;

void Comments::preserveComment(const std::string &str)
{
    size_t pos;
    
    pos = str.find("///");
    if (pos != std::string::npos) {
        return;
    }
    
    pos = str.find("//");
    if (pos != std::string::npos) {
        _preservedComment = str.substr(pos, str.length() - pos);
        if (verbose) std::cout
            << MessageType::Verbose
            << "comment"
            << ": '" << _preservedComment << "' preserved\n";
    }
}

std::string &Comments::removeComment(std::string &str)
{
    size_t pos = str.find("//");
    if (pos != std::string::npos) {
        str.resize(pos);
        if (verbose) std::cout
            << MessageType::Verbose
            << "comment: removed!\n";
    }
    
    return str;
}
std::string &Comments::restoreComment(std::string &str)
{
    if (!_preservedComment.empty()) {
        /*
         If line is not empty then we can't just append the reserved comment back at the
         end of this line as they must be a space between the start of the comment and code
        */
        if (!str.empty()) str.append(" ");
        str.append(_preservedComment);
        if (verbose) std::cout
            << MessageType::Verbose
            << "comment"
            << ": '" << _preservedComment << "' restored\n";
        _preservedComment.clear();
    }
    return str;
}
