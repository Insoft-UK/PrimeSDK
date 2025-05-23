// The MIT License (MIT)
//
// Copyright (c) 2023 Insoft. All rights reserved.
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

#include <iostream>
#include <fstream>
#include <sstream>
#include <regex>
#include <cstring>
#include <iomanip>
#include <cmath>
#include <sys/time.h>
#include <ctime>
#include <vector>
#include <iterator>
#include <cstdlib>

#include <iconv.h>
//#include <stdexcept>

#include "timer.hpp"
#include "singleton.hpp"
#include "common.hpp"

#include "preprocessor.hpp"
#include "dictionary.hpp"
#include "alias.hpp"
#include "strings.hpp"
#include "calc.hpp"
#include "hpprgm.hpp"

#include "../version_code.h"

#define NAME "PPL+ Pre-Processor for PPL"
#define COMMAND_NAME "ppl+"
#define INDENT_WIDTH indentation

static unsigned int indentation = 2;

using ppl_plus::Singleton;
using ppl_plus::Strings;
using ppl_plus::Aliases;
using ppl_plus::Alias;
using ppl_plus::Calc;
using ppl_plus::Dictionary;
using ppl_plus::Preprocessor;

using std::cout;
using std::string;
using std::ofstream;
using std::ifstream;
using std::regex;
using std::smatch;
using std::regex_replace;
using std::sregex_iterator;
using std::sregex_token_iterator;
using std::to_string;
using std::fixed;
using std::ios;
using std::setprecision;
using std::istringstream;
using std::streampos;
using std::getenv;

namespace fs = std::filesystem;
namespace rc = std::regex_constants;

void translatePPLPlusToPPL(const fs::path &path, ofstream &outfile);

static Preprocessor preprocessor = Preprocessor();
static Strings strings = Strings();
static string assignment = "=";



std::string expandTilde(const string &path) {
    if (path.starts_with("~/")) {
        const char* home = getenv("HOME");
        if (home) {
            return string(home) + path.substr(1);  // Replace '~' with $HOME
        }
    }
    return path;
}

template <typename T>
T swap_endian(T u)
{
    static_assert (CHAR_BIT == 8, "CHAR_BIT != 8");

    union
    {
        T u;
        unsigned char u8[sizeof(T)];
    } source, dest;

    source.u = u;

    for (size_t k = 0; k < sizeof(T); k++)
        dest.u8[k] = source.u8[sizeof(T) - k - 1];

    return dest.u;
}

void terminator() {
    cout << MessageType::CriticalError << "An internal pre-processing problem occurred. Please review the syntax before this point.\n";
    exit(0);
}
void (*old_terminate)() = std::set_terminate(terminator);

// MARK: - Utills

std::string utf16_to_utf8(const std::wstring &wstr) {
    std::string utf8;
    uint16_t utf16 = 0;

    for (size_t i = 0; i < wstr.size(); i++) {
        utf16 = static_cast<uint16_t>(wstr[i]);

        if (utf16 <= 0x007F) {
            // 1-byte UTF-8: 0xxxxxxx
            utf8 += static_cast<char>(utf16 & 0x7F);
        } else if (utf16 <= 0x07FF) {
            // 2-byte UTF-8: 110xxxxx 10xxxxxx
            utf8 += static_cast<char>(0b11000000 | ((utf16 >> 6) & 0b00011111));
            utf8 += static_cast<char>(0b10000000 | (utf16 & 0b00111111));
        } else {
            // 3-byte UTF-8: 1110xxxx 10xxxxxx 10xxxxxx
            utf8 += static_cast<char>(0b11100000 | ((utf16 >> 12) & 0b00001111));
            utf8 += static_cast<char>(0b10000000 | ((utf16 >> 6) & 0b00111111));
            utf8 += static_cast<char>(0b10000000 | (utf16 & 0b00111111));
        }
    }

    return utf8;
}


uint16_t utf8_to_utf16(const char *str) {
    uint8_t *utf8 = (uint8_t *)str;
    uint16_t utf16 = *utf8;
    
    if ((utf8[0] & 0b11110000) == 0b11100000) {
        utf16 = utf8[0] & 0b11111;
        utf16 <<= 6;
        utf16 |= utf8[1] & 0b111111;
        utf16 <<= 6;
        utf16 |= utf8[2] & 0b111111;
        return utf16;
    }
    
    // 110xxxxx 10xxxxxx
    if ((utf8[0] & 0b11100000) == 0b11000000) {
        utf16 = utf8[0] & 0b11111;
        utf16 <<= 6;
        utf16 |= utf8[1] & 0b111111;
        return utf16;
    }
    
    return utf16;
}

// Function to remove whitespaces around specific operators using regular expressions
string removeWhitespaceAroundOperators(const string &str) {
    // Regular expression pattern to match spaces around the specified operators
    // Operators: {}[]()≤≥≠<>=*/+-▶.,;:!^
    regex re(R"(\s*([{}[\]()≤≥≠<>=*\/+\-▶.,;:!^&|%`])\s*)");
    
    // Replace matches with the operator and no surrounding spaces
    string result = regex_replace(str, re, "$1");
    
    return result;
}

// MARK: - PPL+ To PPL Translater...
void reformatPPLLine(string &str) {
    regex re;
    
    Strings strings = Strings();
    strings.preserveStrings(str);
    
    str = removeWhitespaceAroundOperators(str);
    
    str = regex_replace(str, regex(R"(,)"), ", ");
    str = regex_replace(str, regex(R"(\{)"), "{ ");
    str = regex_replace(str, regex(R"(\})"), " }");
    str = regex_replace(str, regex(R"(^ +(\} *;))"), "$1\n");
    str = regex_replace(str, regex(R"(\{ +\})"), "{}");
    
    /*
     To prevent correcting over-modifications, first replace all double `==` with a single `=`.
     
     Converting standalone `=` to `==` initially can lead to unintended changes like `<=`, `>=`,
     `:=`, and `==` turning into `<==`, `>==`, `:==`, and `===`.
     
     By first reverting all double `==` back to a single `=`, and ensuring that only standalone
     `=` or `:=` with surrounding whitespace are targeted, we can then safely convert `=` to `==`
     without affecting other operators.
     */
    str = regex_replace(str, regex(R"(==)"), "=");
    
    // Ensuring that standalone `≥`, `≤`, `≠`, `=`, `:=`, `+`, `-`, `*` and `/` have surrounding whitespace.
    re = R"(≥|≤|≠|=|:=|\+|-|\*|\/|▶)";
    str = regex_replace(str, re, " $0 ");
    
    // We now hand the issue of Unary Minus/Operator
    
    // Ensuring that `≥`, `≤`, `≠`, `=`, `+`, `-`, `*` and `/` have a whitespace befor `-`.
    re = R"(([≥≤≠=\+|\-|\*|\/]) +- +)";
    str = regex_replace(str, re, "$1 -");
    
    // Ensuring that `-` in  `{ - `, `( - ` and `[ - ` situations have no surrounding whitespace.
    re = R"(([({[,]) +- +)";
    str = regex_replace(str, re, "$1-");
    
    // We can now safely convert `=` to `==` without affecting other operators.
    if (!regex_search(str, regex(R"(LOCAL [A-Za-z]\w* = )"))) {
        str = regex_replace(str, regex(R"( = )"), " == ");
    }
    
    
    if (Singleton::shared()->scopeDepth > 0) {
        try {
            if (!regex_search(str, regex(R"(\b(BEGIN|IF|CASE|REPEAT|WHILE|FOR|ELSE|IFERR)\b)"))) {
                str.insert(0, string(Singleton::shared()->scopeDepth * INDENT_WIDTH, ' '));
            } else {
                str.insert(0, string((Singleton::shared()->scopeDepth - 1) * INDENT_WIDTH, ' '));
            }
        }
        catch (...) {
            cout << MessageType::CriticalError << "'" << str << "'\n";
            exit(0);
        }
        
        
        re = regex(R"(^ *(THEN)\b)", rc::icase);
        str = regex_replace(str, re, string((Singleton::shared()->scopeDepth - 1) * INDENT_WIDTH, ' ') + "$1");
        
      
        if (regex_search(str, regex(R"(\bEND;$)"))) {
            str = regex_replace(str, regex(R"(;(.+))"), ";\n" + string((Singleton::shared()->scopeDepth - 1) * INDENT_WIDTH, ' ') + "$1");
        } else {
            str = regex_replace(str, regex(R"(; *(.+))"), "; $1");
        }
    }
    
    if (Singleton::shared()->scopeDepth == 0) {
        str = regex_replace(str, regex(R"(END;)"), "$0\n");
        str = regex_replace(str, regex(R"(LOCAL )"), "");
    }
    
    
    str = regex_replace(str, regex(R"(([)};])([A-Z]))"), "$1 $2");
    
    re = R"(([^a-zA-Z ])(BEGIN|END|RETURN|KILL|IF|THEN|ELSE|XOR|OR|AND|NOT|CASE|DEFAULT|IFERR|IFTE|FOR|FROM|STEP|DOWNTO|TO|DO|WHILE|REPEAT|UNTIL|BREAK|CONTINUE|EXPORT|CONST|LOCAL|KEY))";
    str = regex_replace(str, re, "$1 $2");
    
    re = R"((BEGIN|END|RETURN|KILL|IF|THEN|ELSE|XOR|OR|AND|NOT|CASE|DEFAULT|IFERR|IFTE|FOR|FROM|STEP|DOWNTO|TO|DO|WHILE|REPEAT|UNTIL|BREAK|CONTINUE|EXPORT|CONST|LOCAL|KEY)([^a-zA-Z ;]))";
    str = regex_replace(str, re, "$1 $2");
    
    re = R"(([a-zA-Z]) +([{(]))";
    str = regex_replace(str, re, "$1$2");
    
    
    strings.restoreStrings(str);
}

void capitalizeKeywords(string &str) {
    string result = str;
    regex re;
    
    // We turn any keywords that are in lowercase to uppercase
    re = R"(\b(begin|end|return|kill|if|then|else|xor|or|and|not|case|default|iferr|ifte|for|from|step|downto|to|do|while|repeat|until|break|continue|export|const|local|key)\b)";
    for(sregex_iterator it = sregex_iterator(str.begin(), str.end(), re); it != sregex_iterator(); ++it) {
        string result = it->str();
        transform(result.begin(), result.end(), result.begin(), ::toupper);
        str = str.replace(it->position(), it->length(), result);
    }
}


void translatePPLPlusLine(string &ln, ofstream &outfile) {
    regex re;
    smatch match;
    ifstream infile;
    

    Singleton *singleton = Singleton::shared();
    
    // Remove any leading white spaces before or after.
    trim(ln);
    
    if (ln.empty()) {
        ln = "";
        return;
    }
    
    if (singleton->regexp.parse(ln)) return;
    
    
    if (ln.substr(0,2) == "//") {
        ln = ln.insert(0, string(singleton->scopeDepth * INDENT_WIDTH, ' '));
        ln += '\n';
        return;
    }

    /*
     While parsing the contents, strings may inadvertently undergo parsing, leading
     to potential disruptions in the string's content.
     
     To address this issue, we prioritize the preservation of any existing strings.
     After we prioritize the preservation of any existing strings, we blank out the
     string/s.
     
     Subsequently, after parsing, any strings that have been blanked out can be
     restored to their original state.
     */
    strings.preserveStrings(ln);
    strings.blankOutStrings(ln);
    
    ln = regex_replace(ln, regex(R"(\s+)"), " "); // All multiple whitespaces in succesion to a single space, future reg-ex will not require to deal with '\t', only spaces.
    
    // Remove any comments.
    singleton->comments.preserveComment(ln);
    singleton->comments.removeComment(ln);

    
    singleton->regexp.resolveAllRegularExpression(ln);
    if (preprocessor.parse(ln)) {
        ln = "";
        return;
    }
    ln = singleton->aliases.resolveAllAliasesInText(ln);
    
    /*
     A code stack provides a convenient way to store code snippets
     that can be retrieved and used later.
     */
    singleton->codeStack.parse(ln);

    
    if (Dictionary::isDictionaryDefinition(ln)) {
        Dictionary::proccessDictionaryDefinition(ln);
        Dictionary::removeDictionaryDefinition(ln);
    }
    if (ln.empty()) return;
    capitalizeKeywords(ln);
    ln = removeWhitespaceAroundOperators(ln);
    
    /*
     In C++, the standard library provides support for regular expressions
     through the <regex> library, but it does not support lookbehind
     assertions (such as (?<!...)) directly, as they are not part of the
     regular expressions supported by the C++ Standard Library.
     
     However, we can work around this limitation by adjusting your regular
     expression to achieve the same result using alternative techniques.
     
     This approach doesn’t fully replicate lookbehind functionality, but
     it can be effective for simpler cases where a limited lookbehind is
     required.
     */
    
    re = R"((?:[^<>=]|^)(>=|<>|<=|=>)(?!=[<>=]))";
    string::const_iterator it = ln.cbegin();
    while (regex_search(it, ln.cend(), match, re)) {
        // We will convert any >= != <= or => to PPLs ≥ ≠ ≤ and ▶
        string s = match.str(1);
        
        // Replace the operator with the appropriate PPL symbol.
        if (s == ">=") s = "≥";
        if (s == "<>") s = "≠";
        if (s == "<=") s = "≤";
        if (s == "=>") s = "▶";
       
        ln = ln.replace(match.position(1), match.length(1), s);
        it = ln.cbegin();
    }
    
    // PPL by default uses := instead of C's = for assignment. Converting all = to PPL style :=
    if (assignment == "=") {
        re = R"(([^:=]|^)(?:=)(?!=))";
        ln = regex_replace(ln, re, "$1 := ");
    }
    
    
    
    //MARK: alias parsing
    
    re = R"(^alias ([A-Za-z_]\w*(?:::[a-zA-Z]\w*)*):=([a-zA-Z]\w*(?:\.[a-zA-Z]\w*)*);$)";
    if (regex_search(ln, match, re)) {
        Aliases::TIdentity identity;
        identity.identifier = match[1].str();
        identity.real = match[2].str();
        identity.type = Aliases::Type::Alias;
        identity.scope = Aliases::Scope::Auto;
        
        singleton->aliases.append(identity);
        ln = "";
        return;
    }
    
    //MARK: -
    
    re = R"(\b(log|cos|sin|tan|ln|min|max)\b)";
    for(sregex_iterator it = sregex_iterator(ln.begin(), ln.end(), re); it != sregex_iterator(); ++it) {
        string result = it->str();
        transform(result.begin(), result.end(), result.begin(), ::toupper);
        ln = ln.replace(it->position(), it->length(), result);
    }
    
    re = R"(\b(BEGIN|IF|FOR|CASE|REPEAT|WHILE|IFERR)\b)";
    for(auto it = sregex_iterator(ln.begin(), ln.end(), re); it != sregex_iterator(); ++it) {
        singleton->increaseScopeDepth();
    }
    
    re = R"(\b(END|UNTIL)\b)";
    for(auto it = sregex_iterator(ln.begin(), ln.end(), re); it != sregex_iterator(); ++it) {
        singleton->decreaseScopeDepth();
        if (singleton->scopeDepth == 0) {
            singleton->aliases.removeAllOutOfScopeAliases();
            ln += '\n';
        }
       
        singleton->regexp.removeAllOutOfScopeRegexps();
    }
    
    
    if (singleton->scopeDepth == 0) {
        re = R"(^ *(KS?A?_[A-Z\d][a-z]*) *$)";
        sregex_token_iterator it = sregex_token_iterator {
            ln.begin(), ln.end(), re, {1}
        };
        if (it != sregex_token_iterator()) {
            string s = *it;
            ln = "KEY " + s + "()";
        }
    }
    
    singleton->autoname.parse(ln);
    Alias::parse(ln);
    Calc::parse(ln);
    
    
    reformatPPLLine(ln);
    
    ln = regex_replace(ln, regex(R"(__NL__)"), "\n");
    ln = regex_replace(ln, regex(R"(__CR__)"), "\r");
    ln = regex_replace(ln, regex(R"(__INDENT__)"), string(INDENT_WIDTH, ' '));
    ln = regex_replace(ln, regex(R"(__SPACE__)"), " ");
   
    strings.restoreStrings(ln);
    singleton->comments.restoreComment(ln);
    
    ln += '\n';
}

void writeUTF16Line(const string &ln, ofstream &outfile) {
    if (ln.empty()) return;
    
    for ( int n = 0; n < ln.length(); n++) {
        uint8_t *ascii = (uint8_t *)&ln.at(n);
        if (ln.at(n) == '\r') continue;
        
        // Output as UTF-16LE
        if (*ascii >= 0x80) {
            uint16_t utf16 = utf8_to_utf16(&ln.at(n));
            
#ifndef __LITTLE_ENDIAN__
            utf16 = utf16 >> 8 | utf16 << 8;
#endif
            outfile.write((const char *)&utf16, 2);
            if ((*ascii & 0b11100000) == 0b11000000) n++;
            if ((*ascii & 0b11110000) == 0b11100000) n+=2;
            if ((*ascii & 0b11111000) == 0b11110000) n+=3;
        } else {
            outfile.put(ln.at(n));
            outfile.put('\0');
        }
    }
}

void loadRegexLib(const fs::path path, const bool verbose)
{
    string utf8;
    ifstream infile;
    
    infile.open(path, std::ios::in);
    if (!infile.is_open()) {
        return;
    }
    
    if (verbose) cout << "Library " << (path.filename() == ".base.re" ? "base" : path.stem()) << " successfully loaded.\n";
    
    while (getline(infile, utf8)) {
        utf8.insert(0, "regex ");
        Singleton::shared()->regexp.parse(utf8);
    }
    
    infile.close();
}

void loadRegexLibs(const string path, const bool verbose)
{
    loadRegexLib(path + "/.base.re", verbose);
    
    try {
        for (const auto& entry : fs::directory_iterator(path)) {
            if (fs::path(entry.path()).extension() != ".re" || fs::path(entry.path()).filename() == ".base.re") {
                continue;
            }
            loadRegexLib(entry.path(), verbose);
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << "error: " << e.what() << '\n';
    }
}



void embedPPLCode(const string filename, ofstream &os)
{
    ifstream is;
    string s;
    
    is.open(filename, ios::in);
    if (is.is_open()) {
        if (fs::path(filename).extension() == ".hpprgm" || hpprgm::is_utf16le(filename)) {
            std::wstring ws = hpprgm::load(filename);
            s.append(utf16_to_utf8(ws));
            writeUTF16Line(s, os);
        } else {
            while (getline(is, s)) {
                s.append("\n");
                writeUTF16Line(s, os);
            }
        }
    }
}

bool verbose(void)
{
    if (Singleton::shared()->aliases.verbose) return true;
    if (preprocessor.verbose) return true;
    
    return false;
}

enum BlockType
{
    BlockType_Python, BlockType_PPL, BlockType_PrimePlus
};

bool isPythonBlock(const string str) {
    regex re(R"(^ *# *PYTHON( .*)?$)");
    return regex_search(str, re);
}

bool isPPLBlock(const string str) {
    regex re(R"(^ *# *PPL *(\/\/.*)?$)");
    return regex_search(str, re);
}

void writePPLBlock(ifstream &infile, ofstream &outfile) {
    regex re(R"(^ *# *(END) *(?:\/\/.*)?$)");
    string str;
    
    Singleton::shared()->incrementLineNumber();
    
    while(getline(infile, str)) {
        if (regex_search(str, re)) {
            Singleton::shared()->incrementLineNumber();
            return;
        }
        
        str.append("\n");
        writeUTF16Line(str, outfile);
        Singleton::shared()->incrementLineNumber();
    }
}

void writePythonBlock(ifstream &infile, ofstream &outfile) {
    regex re(R"(^ *# *(END) *(?:\/\/.*)?$)");
    string str;
    
    writeUTF16Line("#PYTHON\n", outfile);
    Singleton::shared()->incrementLineNumber();
    
    while(getline(infile, str)) {
        if (regex_search(str, re)) {
            writeUTF16Line("#END\n", outfile);
            Singleton::shared()->incrementLineNumber();
            return;
        }
        
        str.append("\n");
        writeUTF16Line(str, outfile);
        Singleton::shared()->incrementLineNumber();
    }
}

void translatePPLPlusToPPL(const fs::path &path, ofstream &outfile) {
    Singleton &singleton = *Singleton::shared();
    ifstream infile;
    regex re;
    string utf8;
    string str;
    smatch match;

    
    singleton.pushPath(path);
    
    infile.open(path,ios::in);
    if (!infile.is_open()) {
        return;
    }
    
    while (getline(infile, utf8)) {
        /*
         Handle any escape lines `\` by continuing to read line joining them all up as one long line.
         */
        
        if (!utf8.empty()) {
            while (utf8.at(utf8.length() - 1) == '\\' && !utf8.empty()) {
                utf8.resize(utf8.length() - 1);
                string s;
                getline(infile, s);
                utf8.append(s);
                Singleton::shared()->incrementLineNumber();
                if (s.empty()) break;
            }
        }
        
        if (regex_match(utf8, regex(R"(^ *#EXIT *$)", rc::icase))) {
            break;
        }
        
        while (preprocessor.disregard == true) {
            preprocessor.parse(utf8);
            Singleton::shared()->incrementLineNumber();
            getline(infile, utf8);
        }
        
        if (isPythonBlock(utf8)) {
            writePythonBlock(infile, outfile);
            continue;
        }
        
        if (isPPLBlock(utf8)) {
            writePPLBlock(infile, outfile);
            continue;
        }
        
        // Handle `#pragma mode` for PPL+
        re = R"(^ *\#pragma mode *\(.*\) *$)";
        if (regex_match(utf8, re)) {
            re = R"(([a-zA-Z]\w*)\(([^()]*)\))";
            string s = utf8;
            utf8 = "#pragma mode( ";
            for(auto it = sregex_iterator(s.begin(), s.end(), re); it != sregex_iterator(); ++it) {
                if (it->str(1) == "assignment") {
                    if (it->str(2) != ":=" && it->str(2) != "=") {
                        cout << MessageType::Warning << "#pragma mode: for '" << it->str() << "' invalid.\n";
                    }
                    if (it->str(2) == ":=") assignment = ":=";
                    if (it->str(2) == "=") assignment = "=";
                    continue;
                }
                if (it->str(1) == "indentation") {
                    indentation = atoi(it->str(2).c_str());
                    continue;
                }
                utf8.append(it->str() + " ");
            }
            utf8.append(")");
            
            writeUTF16Line(utf8 + "\n", outfile);
            Singleton::shared()->incrementLineNumber();
            continue;
        }
        
        
        if (preprocessor.isQuotedInclude(utf8)) {
            Singleton::shared()->incrementLineNumber();
            
            string filename = preprocessor.extractIncludeFilename(utf8);
            if (fs::path(filename).parent_path().empty() && fs::exists(filename) == false) {
                filename = singleton.getMainSourceDir().string() + "/" + filename;
            }
            if (fs::path(filename).extension() == ".hpprgm" || fs::path(filename).extension() == ".ppl") {
                embedPPLCode(filename, outfile);
                continue;
            }
            if (!(fs::exists(filename))) {
                cout << MessageType::Verbose << fs::path(filename).filename() << " file not found\n";
            } else {
                translatePPLPlusToPPL(filename, outfile);
            }
            continue;
        }
        
        if (preprocessor.isAngleInclude(utf8)) {
            Singleton::shared()->incrementLineNumber();
            string filename = preprocessor.extractIncludeFilename(utf8);
            if (fs::path(filename).extension().empty()) {
                filename.append(".ppl+");
            }
            for (fs::path systemIncludePath : preprocessor.systemIncludePath) {
                if (fs::exists(systemIncludePath.string() + "/" + filename)) {
                    filename = systemIncludePath.string() + "/" + filename;
                    break;
                }
            }
            if (!(fs::exists(filename))) {
                cout << MessageType::Verbose << fs::path(filename).filename() << " file not found\n";
            } else {
                translatePPLPlusToPPL(filename, outfile);
            }
            continue;
        }
    
        if (!Singleton::shared()->scopeDepth) {
            // Local Scope
            
            /*
             We first need to perform pre-parsing to ensure that, in lines such
             as if condition then statement/s end;, the statement/s and end; are
             not on the same line. This ensures proper indentation can be applied
             during the reformatting stage of PPL code.
             
             But we must ignore the line if it's a regex and all @
             */
            
            if (!regex_search(utf8, regex(R"(^ *(@[a-z]+ )? *regex )"))) {
                utf8 = regex_replace(utf8, regex(R"(\b(THEN)\b)", rc::icase), "$1\n");
                utf8 = regex_replace(utf8, regex(R"(; *\b(ELSE)\b)", rc::icase), ";\n$1\n");
                utf8 = regex_replace(utf8, regex(R"(; *(END|UNTIL|ELSE|LOCAL|CONST)?;)", rc::icase), ";\n$1;");
                utf8 = regex_replace(utf8, regex(R"((.+)\bBEGIN\b)", rc::icase), "$1\nBEGIN");
            }
            
        } else {
            // Global Scope
        }
        
        
        istringstream iss;
        iss.str(utf8);
        
        while(getline(iss, str)) {
            translatePPLPlusLine(str, outfile);
            writeUTF16Line(str, outfile);
        }
        
        
        Singleton::shared()->incrementLineNumber();
    }
    
    
    infile.close();
    singleton.popPath();
}


// MARK: - Command Line
void version(void) {
    using namespace std;
    cout
    << "Copyright (C) 2023-" << YEAR << " Insoft. All rights reserved.\n"
    << "Insoft "<< NAME << " version, " << VERSION_NUMBER << " (BUILD " << VERSION_CODE << ")\n"
    << "Built on: " << DATE << "\n"
    << "Licence: MIT License\n\n"
    << "For more information, visit: http://www.insoft.uk\n";
}

void error(void) {
    cout << COMMAND_NAME << ": try '" << COMMAND_NAME << " --help' for more information\n";
    exit(0);
}

void info(void) {
    using namespace std;
    cout
    << "          ***********     \n"
    << "        ************      \n"
    << "      ************        \n"
    << "    ************  **      \n"
    << "  ************  ******    \n"
    << "************  **********  \n"
    << "**********    ************\n"
    << "************    **********\n"
    << "  **********  ************\n"
    << "    ******  ************  \n"
    << "      **  ************    \n"
    << "        ************      \n"
    << "      ************        \n"
    << "    ************          \n\n"
    << "Copyright (C) 2023-" << YEAR << " Insoft. All rights reserved.\n"
    << "Insoft "<< NAME << " version, " << VERSION_NUMBER << "\n\n";
}

void help(void) {
    using namespace std;
    cout
    << "Copyright (C) 2023-" << YEAR << " Insoft. All rights reserved.\n"
    << "Insoft "<< NAME << " version, " << VERSION_NUMBER << " (BUILD " << VERSION_CODE << ")\n"
    << "\n"
    << "Usage: " << COMMAND_NAME << " <input-file> [-o <output-file>] [-v <flags>]\n"
    << "\n"
    << "Options:\n"
    << "  -o <output-file>        Specify the filename for generated PPL code.\n"
    << "  -v                      Display detailed processing information.\n"
    << "  --utf16-le              UTF16-LE for .hpprgm file format.\n"
    << "\n"
    << "  Verbose Flags:\n"
    << "     a                    Aliases\n"
    << "     p                    Preprocessor\n"
    << "     r                    Regular Expression\n"
    << "\n"
    << "Additional Commands:\n"
    << "  ansiart {--version | --help}\n"
    << "    --version              Display the version information.\n"
    << "    --help                 Show this help message.\n";
}


// MARK: - Main
int main(int argc, char **argv) {
    
    string in_filename, out_filename;
    
    if (argc == 1) {
        error();
        exit(100);
    }
    
    bool verbose = false;
    bool showpath = false;
    
    bool utf16 = false;
    
    string args(argv[0]);
    
    for (int n = 1; n < argc; n++) {
        args = argv[n];
        
        if (args == "-o") {
            if ( n + 1 >= argc ) {
                error();
                exit(101);
            }
            out_filename = argv[n + 1];
            if (fs::path(out_filename).extension().empty()) {
                out_filename.append(".hpprgm");
            }
            
            n++;
            continue;
        }
        
        if ( args == "--utf16-le" ) {
            utf16 = true;
            continue;
        }
        
        if ( args == "--help" ) {
            help();
            return 0;
        }
        
        
        if ( strcmp( argv[n], "--version" ) == 0 ) {
            version();
            return 0;
        }
        
        
        if (args.starts_with("-v=")) {
            if (args.find("a") != string::npos) Singleton::shared()->aliases.verbose = true;
            if (args.find("p") != string::npos) preprocessor.verbose = true;
            if (args.find("r") != string::npos) Singleton::shared()->regexp.verbose = true;
            if (args.find("l") != string::npos) verbose = true;
                
            continue;
        }
        
        if (args.starts_with("-I")) {
            preprocessor.systemIncludePath.push_front(fs::path(args.substr(2)).has_filename() ? fs::path(args.substr(2)) : fs::path(args.substr(2)).parent_path());
            continue;
        }
        
        if (args == "--path") {
            showpath = true;
            continue;
        }
        
        in_filename = expandTilde(argv[n]);
        regex re(R"(.\w*$)");
    }
    
    
    
    if (!fs::exists(in_filename)) {
        if (fs::exists(in_filename + ".pp")) in_filename.append(".pp");
        if (fs::exists(in_filename + ".ppl+")) in_filename.append(".ppl+");
    }
    if (!fs::exists(in_filename)) {
        error();
        return 0;
    }
    
    if (out_filename.empty()) {
        out_filename = fs::path(in_filename).stem().string() + ".ppl";
    }
    if (fs::path(out_filename).parent_path().empty()) {
        out_filename.insert(0, fs::path(in_filename).parent_path().string() + "/");
    }
    
    info();
    
    
    ofstream outfile;
    outfile.open(out_filename, ios::out | ios::binary);
    if(!outfile.is_open())
    {
        error();
        return 0;
    }

    if (fs::path(out_filename).extension() != ".hpprgm" || utf16) {
        // The ".ppl" file format requires UTF16-LE.
        outfile.put(0xFF);
        outfile.put(0xFE);
    } else {
        outfile.seekp(20, ios::beg);
    }
    
      
    
    // Start measuring time
    Timer timer;
    
    string str;
    
    str = "#define __pplplus";
    preprocessor.parse(str);
    
    str = R"(#define __LIST_LIMIT 10000)";
    preprocessor.parse(str);
    
    str = R"(#define __VERSION )" + to_string(NUMERIC_BUILD / 100);
    preprocessor.parse(str);
    
    str = R"(#define __NUMERIC_BUILD )" + to_string(NUMERIC_BUILD);
    preprocessor.parse(str);
    
#ifdef DEBUG
    loadRegexLibs(expandTilde("~/GitHub/PrimeSDK/Package Installer/package-root/Applications/HP/PrimeSDK/lib"), true);
#else
    loadRegexLibs("/Applications/HP/PrimeSDK/lib", verbose);
#endif
    
    
    translatePPLPlusToPPL(in_filename, outfile);
    
    if (fs::path(out_filename).extension() == ".hpprgm" && !utf16) {
        // Get the current file size.
        streampos currentPos = outfile.tellp();
        
        // Code size will be the current filesize - the header size + the two aditional bytes.
        uint32_t codeSize = static_cast<uint32_t>(currentPos) - 20 + 2;

        // Seek to the beginning to write the header.
        outfile.seekp(0, ios::beg);

        // HEADER
        /**
         0x0000-0x0003: Header Size, excludes itself (so the header begins at offset 4)
         */
        outfile.put(0x0C); // 12
        outfile.put(0x00);
        outfile.put(0x00);
        outfile.put(0x00);
        
        // Write the 12-byte UTF-16LE header.
        /**
         0x0004-0x0005: Number of variables in table.
         0x0006-0x0007: Number of uknown?
         0x0008-0x0009: Number of exported functions in table.
         0x000A-0x000F: Conn. kit generates 7F 01 00 00 00 00 but all zeros seems to work too.
         */
        for (int i = 0; i < 12; ++i) {
            outfile.put(0x00);
        }

        // CODE HEADER
        /**
         0x0000-0x0003: Size of the PPL Code in UTF-16 LE
         */
#ifndef __LITTLE_ENDIAN__
        codeSize = swap_endian(codeSize);
#endif
        outfile.write(reinterpret_cast<const char*>(&codeSize), sizeof(codeSize));
        
        /**
         0x0004-0x----: Code in UTF-16 LE until 00 00
         */
        outfile.seekp(0, ios::end);
        outfile.put(0x00);
        outfile.put(0x00);
    }
    outfile.close();
    
    if (hasErrors() == true) {
        cout << ANSI::Red << "ERRORS!" << ANSI::Default << "\n";
        remove(out_filename.c_str());
        return 0;
    }
    
    // Stop measuring time and calculate the elapsed time.
    long long elapsed_time = timer.elapsed();
    
    // Display elasps time in secononds.
    cout << "Completed in " << fixed << setprecision(2) << elapsed_time / 1e9 << " seconds\n";
    if (showpath)
        cout << "UTF-16LE file at \"" << out_filename << "\" succefuly created.\n";
    else
        cout << "UTF-16LE file " << fs::path(out_filename).filename() << " succefuly created.\n";
    
            
    return 0;
}
