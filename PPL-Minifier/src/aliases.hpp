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


#ifndef ALIASES_HPP
#define ALIASES_HPP

#include <iostream>
#include <list>
#include <vector>
#include <stdint.h>

namespace ppl {
    class Aliases {
    public:
        enum class Type {
            Unknown,
            GlobalVariable,
            Variable,
            Function,
            Property
        };
        
        enum class Scope {
            Auto   = 0,
            Global = 1,
            Local  = 2
        };
        
        typedef struct TIdentity {
            std::string identifier;
            std::string real;
            Type type;
            Scope scope;
            long line;              // line that definition accoured;
            std::string pathname;   // path and filename that definition accoured
        } TIdentity;
        std::vector<TIdentity> identities;
        
        bool descendingOrder = true;
        bool verbose = false;
        
        bool append(const TIdentity &identity);
        void removeAllLocalAliases();
        void removeAllAliasesOfType(const Type type);
        std::string resolveAliasesInText(const std::string& str);
        void remove(const std::string& identifier);
        bool exists(const TIdentity &identity);
        bool identifierExists(const std::string& identifier);
        bool realExists(const std::string& real);
        void dumpIdentities();
    };
}
#endif // ALIASES_HPP
