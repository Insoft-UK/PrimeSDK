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


#ifndef SINGLETON_HPP
#define SINGLETON_HPP

#include <iostream>
#include <vector>
#include "aliases.hpp"
#include "common.hpp"

using namespace ppl;

class Singleton {
    std::vector<std::string> _pathnames;
    std::vector<long> _lines;
    static Singleton* _shared;
public:
    enum class Scope {
        Global = 1,
        Local  = 2
    };
    Scope scope = Scope::Global;

    Aliases aliases;
    
    int nestingLevel = 0;
    
    static Singleton *shared();
    
    void incrementLineNumber(void);
    long currentLineNumber(void);
    
    std::string currentPathname(void);
    
    // returns the pathname of
    std::string getPath(void);
    
    void pushPathname(const std::string& pathname);
    void popPathname(void);
    
    
    
private:
    Singleton() {
        _currentline = 1;
    }
    Singleton(const Singleton &);
    Singleton& operator=(const Singleton &);
    
protected:
    long _currentline;
};


#endif /* SINGLETON_HPP */
