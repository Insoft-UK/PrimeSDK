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

#include "aliases.hpp"
#include "common.hpp"

#include "singleton.hpp"
#include <regex>
#include <sstream>

using namespace ppl;

bool compareInterval(Aliases::TIdentity i1, Aliases::TIdentity i2) {
    return (i1.identifier.length() > i2.identifier.length());
}

bool Aliases::append(const TIdentity& idty) {
    TIdentity identity = idty;
    Singleton *singleton = Singleton::shared();
    
    if (identity.identifier.empty()) return false;
    
    trim(identity.identifier);
    trim(identity.real);
    identity.pathname = singleton->currentPathname();
    identity.line = singleton->currentLineNumber();
    
    
    if (Scope::Auto == identity.scope) {
        identity.scope = singleton->scope == Singleton::Scope::Global ? Aliases::Scope::Global : Aliases::Scope::Local;
    }
    
    if ('_' == identity.identifier.at(0) && '_' != identity.identifier.at(1)) {
        identity.identifier = identity.identifier.substr(1, identity.identifier.length() - 1);
        identity.type = Type::Property;
    }
    
    if (exists(identity) == true) {
        for (const auto &it : identities) {
            if (it.identifier == identity.identifier && identity.type != Type::Function) {
                std::cout
                << MessageType::Warning
                << "redefinition of: "
                << "'" << identity.identifier << "' as "
                << (Scope::Local == identity.scope ? "local: " : "")
                << (Scope::Global == identity.scope ? "global: " : "")
                << "type was previous definition at " << it.pathname << ":" << it.line << '\n';
                break;
            }
        }
        return false;
    }
    
    identities.push_back(identity);
    
    if (descendingOrder) std::sort(identities.begin(), identities.end(), compareInterval);
    
    if (verbose) std::cout
        << MessageType::Verbose
        << (Scope::Local == identity.scope ? "local:" : "")
        << (Scope::Global == identity.scope ? "global:" : "")
        << (Type::Unknown == identity.type ? " identifier" : "")
        << " '" << identity.identifier << "' for '" << identity.real << "' defined\n";
    return true;
}

void Aliases::removeAllLocalAliases() {
    for (auto it = identities.begin(); it != identities.end(); ++it) {
        if (it->scope == Scope::Local) {
            if (verbose) std::cout
                << MessageType::Verbose
                << "local:"
                << (Type::Unknown == it->type ? " identifier" : "")
                << " '" << it->identifier << "' removed!\n";
            identities.erase(it);
            removeAllLocalAliases();
            break;
        }
    }
}

void Aliases::removeAllAliasesOfType(const Type type) {
    for (auto it = identities.begin(); it != identities.end(); ++it) {
        if (it->type == type) {
            if (verbose) std::cout
                << MessageType::Verbose
                << (Scope::Local == it->scope ? "local: " : "")
                << (Scope::Global == it->scope ? "global: " : "")
                << (Type::Unknown == it->type ? "identifier" : "")
                << " '" << it->identifier << "' removed!\n";
            identities.erase(it);
            removeAllLocalAliases();
            break;
        }
    }
}


std::string Aliases::resolveAliasesInText(const std::string& str) {
    std::string s = str;
    std::regex re;
    
    if (s.empty()) return s;
        
    for (auto it = identities.begin(); it != identities.end(); ++it) {
        if ('`' == it->identifier.at(0) && '`' == it->identifier.at(it->identifier.length() - 1)) {
            re = it->identifier;
        } else {
            re = R"(\b)" + it->identifier + R"(\b)";
        }

        s = regex_replace(s, re, it->real);
    }
    
    return s;
}

void Aliases::remove(const std::string& identifier) {
    for (auto it = identities.begin(); it != identities.end(); ++it) {
        if (it->identifier == identifier) {
            if (verbose) std::cout
                << MessageType::Verbose
                << (Scope::Local == it->scope ? "local: " : "")
                << (Scope::Global == it->scope ? "global: " : "")
                << (Type::Unknown == it->type ? "identifier" : "")
                << " '" << it->identifier << "' removed!\n";
            
            identities.erase(it);
            return;
        }
    }
}

bool Aliases::exists(const TIdentity &identity) {
    for (auto it = identities.begin(); it != identities.end(); ++it) {
        if (it->identifier == identity.identifier) {
            return true;
        }
    }
    return false;
}

bool Aliases::identifierExists(const std::string& identifier) {
    for (auto it = identities.begin(); it != identities.end(); ++it) {
        if (it->identifier == identifier) {
            return true;
        }
    }
    return false;
}

bool Aliases::realExists(const std::string& real) {
    for (auto it = identities.begin(); it != identities.end(); ++it) {
        if (it->real == real) {
            return true;
        }
    }
    return false;
}

void Aliases::dumpIdentities() {
    for (auto it = identities.begin(); it != identities.end(); ++it) {
        if (verbose) std::cout << "identities : " << it->identifier << " = " << it->real << "\n";
    }
}
