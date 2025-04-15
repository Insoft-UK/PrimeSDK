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


#include "singleton.hpp"

Singleton* Singleton::_shared = NULL;

Singleton* Singleton::shared() {
    if (_shared == NULL) {
        _shared = new Singleton();
    }
    return(_shared);
}

void Singleton::incrementLineNumber(void) {
    ++_currentline;
}

long Singleton::currentLineNumber(void) {
    return _currentline;
}

std::string Singleton::currentPathname(void) {
    if (_pathnames.empty()) return "";
    return _pathnames.back();
}

std::string Singleton::getPath(void) {
    std::string pathname = _pathnames.front();
    if (pathname.empty()) return "";
    pathname.resize(pathname.rfind('/') + 1);
    return pathname;
}

void Singleton::pushPathname(const std::string& pathname) {
    _pathnames.push_back(pathname);
    _lines.push_back(_currentline);
    _currentline = 1;
}
void Singleton::popPathname(void) {
    _currentline = _lines.back();
    _pathnames.pop_back();
    _lines.pop_back();
}
