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


#include "preprocessor.hpp"
#include "singleton.hpp"
#include "common.hpp"
#include "calc.hpp"

#include <regex>
#include <sstream>
#include <fstream>
#include <cctype>

using namespace ppl_plus;

static Singleton *_singleton  = Singleton::shared();


bool Preprocessor::parse(std::string &str) {
    std::string s;
    std::regex re;
    std::smatch match;
    std::sregex_token_iterator it;
    std::sregex_token_iterator end;
    Aliases::TIdentity  identity;
    filename = std::string("");

    
    
    if (disregard == false) {
        if (regex_search(str, std::regex(R"(^ *@disregard *$)"))) {
            disregard = true;
            return true;
        }
        
        re = R"(^ *#include +)";
        if (regex_search(str, re)) {
            std::sregex_token_iterator it;
            const std::sregex_token_iterator end;
            
            re = R"(^ *#include +<([^<>:"\|\?\*]*)>)";
            it = std::sregex_token_iterator {
                str.begin(), str.end(), re, {1}
            };
            if (it != end) {
                filename = *it++;
                
                if (std::filesystem::path(filename).parent_path().empty()) {
                    filename.insert(0, path + "/");
                }
                
                if (std::filesystem::path(filename).extension().empty()) {
                    filename.append(".pplib");
                }
                
                if (verbose) std::cout << MessageType::Verbose << "#include: file named '" << std::filesystem::path(filename).filename() << "'\n";
                return true;
            }
            
            re = R"(^ *#include +"([^<>:"\|\?\*]*)\")";
            it = std::sregex_token_iterator {
                str.begin(), str.end(), re, {1}
            };
            if (it != end) {
                filename = *it++;
                
                if (!file_exists(filename)) {
                    filename.insert(0, _singleton->getProjectDir() + "/");
                    if (!file_exists(filename)) {
                        std::cout << MessageType::Error << "file " << std::filesystem::path(filename).filename() << " not found.\n";
                        return false;
                    }
                }
                if (verbose) std::cout << MessageType::Verbose << "#include: file named '" << std::filesystem::path(filename).filename() << "'\n";
                return true;
            }
            return false;
        }
        
        /*
         eg. #define NAME(a,b,c) c := a+b
         Group  0 #define NAME(a,b,c) c := a+b
                1 NAME
                2 a,b,c
                3 c := a+b
         */
        re = R"(^ *#define +([A-Za-z_]\w*)(?:\(([A-Za-z_ ,]+)\))? *(.*))";
        if (std::regex_search(str, match, re)) {
            identity.identifier = match[1].str();
            identity.parameters = match[2].str();
            strip(identity.parameters);
            identity.real = match[3].str();
            
            identity.scope = Aliases::Scope::Global;
            identity.type = Aliases::Type::Macro;
            
            identity.real = _singleton->aliases.resolveAllAliasesInText(identity.real);
            Calc::evaluateMathExpression(identity.real);
            
            _singleton->aliases.append(identity);
            if (verbose) std::cout << MessageType::Verbose << "#define: " << identity.identifier << '\n';
            return true;
        }
 
        /*
         eg. #undef NAME
         Group  0 #undef NAME
                1 NAME
         */
        re = R"(^ *#undef +([A-Za-z_][\w]*) *$)";
        if (std::regex_search(str, match, re)) {
            _singleton->aliases.remove(match[1].str());
            if (verbose) std::cout << MessageType::Verbose << "#undef: " << *it << '\n';
            return true;
        }
        


        /*
         eg. #ifdef NAME
         Group  0 #ifdef NAME
                1 NAME
         */
        re = R"(^ *#ifdef +([A-Za-z_]\w*) *$)";
        if (std::regex_search(str, match, re)) {
            identity.identifier = match[1].str();
            disregard = !_singleton->aliases.identifierExists(identity.identifier);
            if (verbose) std::cout << MessageType::Verbose << "#ifdef: " << identity.identifier << " is " << (!disregard ? "true" : "false") << '\n';
            return true;
        }
        
        /*
         eg. #ifndef NAME
         Group  0 #ifndef NAME
                1 NAME
         */
        re = R"(^\ *#ifndef +([A-Za-z_]\w*) *$)";
        if (std::regex_search(str, match, re)) {
            identity.identifier = match[1].str();
            
            disregard = _singleton->aliases.identifierExists(identity.identifier);
            if (verbose) std::cout << MessageType::Verbose << "#ifndef: " << identity.identifier << " is " << (!disregard ? "true" : "false") << '\n';
            return true;
        }
        
        
        re = R"(^ *#if +([A-Za-z_]\w*) *(==|!=|>=|<=|>|<) *(.+)$)";
        if (std::regex_search(str, match, re)) {
            identity = _singleton->aliases.getIdentity(match[1].str());
            if (identity.identifier.empty()) return true;
            std::string op = match[2].str();
            std::string real = match[3].str();
            
            disregard = true;
            if (op == "==" && identity.real == real) disregard = false;
            if (op == "!=" && identity.real != real) disregard = false;
            if (op == ">=" && op >= identity.real) disregard = false;
            if (op == "<=" && op <= identity.real) disregard = false;
            if (op == ">" && op >= identity.real) disregard = false;
            if (op == "<" && op <= identity.real) disregard = false;
            
            return true;
        }
    }
    
    if (regex_search(str, std::regex(R"(^ *#else\b *((\/\/.*)|)$)"))) {
        disregard = !disregard;
        if (verbose) std::cout << MessageType::Verbose << "#else: " << disregard << '\n';
        return true;
    }
    
    if (regex_search(str, std::regex(R"(^ *#endif\b *((\/\/.*)|)$)"))) {
        disregard = false;
        if (verbose) std::cout << MessageType::Verbose << "#endif: " << disregard << '\n';
        return true;
    }
    
    if (regex_search(str, std::regex(R"(^ *@end|END *$)"))) {
        disregard = false;
        if (verbose) std::cout << MessageType::Verbose << "@end: " << disregard << '\n';
        return true;
    }
    
    if (regex_search(str, std::regex(R"(^ *#)"))) {
        str = "";
//        std::cout << MessageType::Error << "invalid preprocessor\n";
    }

    return false;
}


