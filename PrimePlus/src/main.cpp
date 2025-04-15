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

#include "timer.hpp"
#include "singleton.hpp"
#include "common.hpp"

#include "preprocessor.hpp"
#include "dictionary.hpp"
#include "alias.hpp"
#include "strings.hpp"
#include "calc.hpp"

#include "version_code.h"

#define NAME "P+ Pre-Processor"
#define COMMAND_NAME "p+"

using namespace pp;

static Preprocessor preprocessor = Preprocessor();
static Strings strings = Strings();

static std::string _basename;


void terminator() {
    std::cout << MessageType::CriticalError << "An internal pre-processing problem occurred. Please review the syntax before this point.\n";
    exit(0);
}

void (*old_terminate)() = std::set_terminate(terminator);


void translatePPlusToPPL(const std::string &pathname, std::ofstream &outfile);

// MARK: - Utills

uint32_t utf8_to_utf16(const char *str) {
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
std::string removeWhitespaceAroundOperators(const std::string &str) {
    // Regular expression pattern to match spaces around the specified operators
    // Operators: {}[]()≤≥≠<>=*/+-▶.,;:!^
    std::regex re(R"(\s*([{}[\]()≤≥≠<>=*\/+\-▶.,;:!^&|%`])\s*)");
    
    // Replace matches with the operator and no surrounding spaces
    std::string result = std::regex_replace(str, re, "$1");
    
    return result;
}

// MARK: - P+ To PPL Translater...
void reformatPPLLine(std::string &str) {
    std::regex re;
    
    Strings strings = Strings();
    strings.preserveStrings(str);
    
    str = removeWhitespaceAroundOperators(str);
    
    str = regex_replace(str, std::regex(R"(,)"), ", ");
    str = regex_replace(str, std::regex(R"(\{)"), "{ ");
    str = regex_replace(str, std::regex(R"(\})"), " }");
    str = regex_replace(str, std::regex(R"(^ +(\} *;))"), "$1\n");
    str = regex_replace(str, std::regex(R"(\{ +\})"), "{}");
    
    /*
     To prevent correcting over-modifications, first replace all double `==` with a single `=`.
     
     Converting standalone `=` to `==` initially can lead to unintended changes like `<=`, `>=`,
     `:=`, and `==` turning into `<==`, `>==`, `:==`, and `===`.
     
     By first reverting all double `==` back to a single `=`, and ensuring that only standalone
     `=` or `:=` with surrounding whitespace are targeted, we can then safely convert `=` to `==`
     without affecting other operators.
     */
    str = regex_replace(str, std::regex(R"(==)"), "=");
    
    // Ensuring that standalone `≥`, `≤`, `≠`, `=`, `:=`, `+`, `-`, `*` and `/` have surrounding whitespace.
    re = R"(≥|≤|≠|=|:=|\+|-|\*|\/|▶)";
    str = regex_replace(str, re, " $0 ");
    
    // We now hand the issue of Unary Minus/Operator
    
    // Ensuring that `≥`, `≤`, `≠`, `=`, `+`, `-`, `*` and `/` have a whitespace befor `-`.
    re = R"(([≥≤≠=\+|\-|\*|\/]) +- +)";
    str = regex_replace(str, re, "$1 -");
    
    // Ensuring that `-` in  `{ - `, `( - ` and `[ - ` situations have no surrounding whitespace.
    re = R"(([({[]) +- +)";
    str = regex_replace(str, re, "$1-");
    
    if (!regex_search(str, std::regex(R"(LOCAL [A-Za-z]\w* = )"))) {
        // We can now safely convert `=` to `==` without affecting other operators.
        str = regex_replace(str, std::regex(R"( = )"), " == ");
    }
    
    
    if (Singleton::shared()->scopeDepth > 0) {
        try {
            if (!regex_search(str, std::regex(R"(\b(BEGIN|IF|CASE|REPEAT|WHILE|FOR|ELSE|IFERR)\b)"))) {
                str.insert(0, std::string(Singleton::shared()->scopeDepth * INDENT_WIDTH, ' '));
            } else {
                str.insert(0, std::string((Singleton::shared()->scopeDepth - 1) * INDENT_WIDTH, ' '));
            }
        }
        catch (...) {
            std::cout << MessageType::CriticalError << "'" << str << "'\n";
            exit(0);
        }
        
        
        re = std::regex(R"(^ *(THEN)\b)", std::regex_constants::icase);
        str = regex_replace(str, re, std::string((Singleton::shared()->scopeDepth - 1) * INDENT_WIDTH, ' ') + "$1");
        
      
        if (regex_search(str, std::regex(R"(\bEND;$)"))) {
            str = regex_replace(str, std::regex(R"(;(.+))"), ";\n" + std::string(Singleton::shared()->scopeDepth * INDENT_WIDTH, ' ') + "$1");
        } else {
            str = regex_replace(str, std::regex(R"(;(.+))"), ";\n" + std::string((Singleton::shared()->scopeDepth - 1) * INDENT_WIDTH, ' ') + "$1");
        }
    }
    
    if (Singleton::shared()->scopeDepth == 0) {
        str = regex_replace(str, std::regex(R"(END;)"), "$0\n");
        str = regex_replace(str, std::regex(R"(LOCAL )"), "");
    }
    
    str = regex_replace(str, std::regex(R"( +(:|-))"), " $1");
    str = regex_replace(str, std::regex(R"(([^\d]) +- +)"), "$1 -");
    str = regex_replace(str, std::regex(R"(([^-]) *-(?!= ))"), "$1 - ");
    str = regex_replace(str, std::regex(R"(([)};])([A-Z]))"), "$1 $2");
    
    re = R"(([^a-zA-Z ])(BEGIN|END|RETURN|KILL|IF|THEN|ELSE|XOR|OR|AND|NOT|CASE|DEFAULT|IFERR|IFTE|FOR|FROM|STEP|DOWNTO|TO|DO|WHILE|REPEAT|UNTIL|BREAK|CONTINUE|EXPORT|CONST|LOCAL|KEY))";
    str = regex_replace(str, re, "$1 $2");
    
    re = R"((BEGIN|END|RETURN|KILL|IF|THEN|ELSE|XOR|OR|AND|NOT|CASE|DEFAULT|IFERR|IFTE|FOR|FROM|STEP|DOWNTO|TO|DO|WHILE|REPEAT|UNTIL|BREAK|CONTINUE|EXPORT|CONST|LOCAL|KEY)([^a-zA-Z ;]))";
    str = regex_replace(str, re, "$1 $2");
    
    re = R"(([a-zA-Z]) +([{(]))";
    str = regex_replace(str, re, "$1$2");
    
    
    strings.restoreStrings(str);
}

void capitalizeKeywords(std::string &str) {
    std::string result = str;
    std::regex re;
    
    // We turn any keywords that are in lowercase to uppercase
    re = R"(\b(begin|end|return|kill|if|then|else|xor|or|and|not|case|default|iferr|ifte|for|from|step|downto|to|do|while|repeat|until|break|continue|export|const|local|key)\b)";
    for(std::sregex_iterator it = std::sregex_iterator(str.begin(), str.end(), re); it != std::sregex_iterator(); ++it) {
        std::string result = it->str();
        std::transform(result.begin(), result.end(), result.begin(), ::toupper);
        str = str.replace(it->position(), it->length(), result);
    }
}


void translatePPlusLine(std::string &ln, std::ofstream &outfile) {
    std::regex re;
    std::smatch match;
    std::ifstream infile;
    

    Singleton *singleton = Singleton::shared();
    
    static int consecutiveBlankLines = 0;
    
    
    // Remove any leading white spaces before or after.
    trim(ln);
    
    if (ln.empty()) {
        ln = "";
        if (!consecutiveBlankLines++) {
            ln += '\n';
        }
        return;
    }
    consecutiveBlankLines = 0;
    
    
    if (ln.substr(0,2) == "//") {
        ln = ln.insert(0, std::string(singleton->scopeDepth * INDENT_WIDTH, ' '));
        ln += '\n';
        return;
    }
    
    if (singleton->regexp.parse(ln)) return;
    


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
    
    ln = regex_replace(ln, std::regex(R"(\s+)"), " "); // All multiple whitespaces in succesion to a single space, future reg-ex will not require to deal with '\t', only spaces.
    
    
    // Remove any comments.
    singleton->comments.preserveComment(ln);
    singleton->comments.removeComment(ln);

    
    singleton->regexp.resolveAllRegularExpression(ln);
    ln = singleton->aliases.resolveAllAliasesInText(ln);
    
    
    
    
    /*
     A code stack provides a convenient way to store code snippets
     that can be retrieved and used later.
     */
    singleton->codeStack.parse(ln);

    
    if (Dictionary::isDictionary(ln)) {
        Dictionary::proccessDictionary(ln);
        ln = "";
        return;
    }
    
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
    
    re = R"((?:[^<>=]|^)(>=|!=|<>|<=|=>)(?!=[<>=]))";
    std::string::const_iterator it = ln.cbegin();
    while (std::regex_search(it, ln.cend(), match, re)) {
        // We will convert any >= != <= or => to PPLs ≥ ≠ ≤ and ▶
        std::string s = match.str(1);
        
        // Replace the operator with the appropriate PPL symbol.
        if (s == ">=") s = "≥";
        if (s == "!=") s = "≠";
        if (s == "<>") s = "≠";
        if (s == "<=") s = "≤";
        if (s == "=>") s = "▶";
       
        ln = ln.replace(match.position(1), match.length(1), s);
        it = ln.cbegin();
    }
    
    // PPL uses := instead of C's = for assignment. Converting all = to PPL style :=
    re = R"(([^:=]|^)(?:=)(?!=))";
    ln = std::regex_replace(ln, re, "$1 := ");
    
    
    
    //MARK: - namespace parsing
    
    re = R"(^namespace ([A-Za-z](?:\w+|::[A-Za-z]+)*):=([A-Za-z](?:\w+|::[A-Za-z]+)*);$)";
    if (regex_search(ln, match, re) && singleton->scopeDepth == 0) {
        Aliases::TIdentity identity;
        identity.identifier = match[1].str();
        identity.real = match[2].str();
        identity.type = Aliases::Type::Def;
        identity.scope = Aliases::Scope::Auto;
        
        singleton->aliases.append(identity);
        ln = "";
        return;
    }
    
    re = R"(^using ([A-Za-z](?:\w+|::[A-Za-z]+)*);$)";
    if (regex_search(ln, match, re) && singleton->scopeDepth > 0) {
        singleton->aliases.addNamespace(match[1].str());
        ln = "";
        return;
    }
    
    //MARK: -
    
    re = R"(\b(log|cos|sin|tan|ln|min|max)\b)";
    for(std::sregex_iterator it = std::sregex_iterator(ln.begin(), ln.end(), re); it != std::sregex_iterator(); ++it) {
        std::string result = it->str();
        std::transform(result.begin(), result.end(), result.begin(), ::toupper);
        ln = ln.replace(it->position(), it->length(), result);
    }
    
    re = R"(\b(BEGIN|IF|FOR|CASE|REPEAT|WHILE|IFERR|switch)\b)";
    for(auto it = std::sregex_iterator(ln.begin(), ln.end(), re); it != std::sregex_iterator(); ++it) {
        singleton->increaseScopeDepth();
    }
    
    re = R"(\b(END|UNTIL)\b)";
    for(auto it = std::sregex_iterator(ln.begin(), ln.end(), re); it != std::sregex_iterator(); ++it) {
        singleton->decreaseScopeDepth();
        if (singleton->scopeDepth == 0) {
            singleton->aliases.removeAllLocalAliases();
            ln += '\n';
        }
       
        singleton->regexp.removeAllOutOfScopeRegexps();
    }
    
    
    if (singleton->scopeDepth == 0) {
        re = R"(^ *(KS?A?_[A-Z\d][a-z]*) *$)";
        std::sregex_token_iterator it = std::sregex_token_iterator {
            ln.begin(), ln.end(), re, {1}
        };
        if (it != std::sregex_token_iterator()) {
            std::string s = *it;
            ln = "KEY " + s + "()";
        }
        
        re = R"(^ *(?:export +)?([_:.\w]+) *(?=\())";
        if (regex_search(ln, match, re)) {
            if (match[1].str().at(0) == '_') {
                std::string pattern;
                pattern = match[1].str();
                re = pattern;
                ln = regex_replace(ln, re, "auto:" + match[1].str());
            }
        }
        re = R"(^ *(export +)?([a-zA-Z_]\w*((::)|\.))+[a-zA-Z_]\w* *(?=\())";
        ln = regex_replace(ln, re, "auto:$0");
    }
    
    
    singleton->autoname.parse(ln);
    Alias::parse(ln);
    
    Calc::parse(ln);
    
    
    reformatPPLLine(ln);
    
    ln = regex_replace(ln, std::regex(R"(__NL__)"), "\n");
    ln = regex_replace(ln, std::regex(R"(__CR__)"), "\r");
    ln = regex_replace(ln, std::regex(R"(__INDENT__)"), std::string(INDENT_WIDTH, ' '));
    ln = regex_replace(ln, std::regex(R"(__SPACE__)"), " ");
   
    strings.restoreStrings(ln);
    singleton->comments.restoreComment(ln);
    
    ln += '\n';
}

void writeUTF16Line(const std::string &ln, std::ofstream &outfile) {
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

bool verbose(void) {
    if (Singleton::shared()->aliases.verbose) return true;
    if (preprocessor.verbose) return true;
    
    return false;
}

enum BlockType {
    BlockType_Python, BlockType_PPL, BlockType_PrimePlus
};

bool isPythonBlock(const std::string str) {
    std::regex re(R"(^ *# *PYTHON *(\/\/.*)?$)");
    return std::regex_search(str, re);
}

bool isPPLBlock(const std::string str) {
    std::regex re(R"(^ *# *PPL *(\/\/.*)?$)");
    return std::regex_search(str, re);
}

void writePPLBlock(std::ifstream &infile, std::ofstream &outfile) {
    std::regex re(R"(^ *# *(END) *(?:\/\/.*)?$)");
    std::string str;
    
    Singleton::shared()->incrementLineNumber();
    
    while(getline(infile, str)) {
        if (std::regex_search(str, re)) {
            Singleton::shared()->incrementLineNumber();
            return;
        }
        
        str.append("\n");
        writeUTF16Line(str, outfile);
        Singleton::shared()->incrementLineNumber();
    }
}

void writePythonBlock(std::ifstream &infile, std::ofstream &outfile) {
    std::regex re(R"(^ *# *(END) *(?:\/\/.*)?$)");
    std::string str;
    
    writeUTF16Line("#PYTHON\n", outfile);
    Singleton::shared()->incrementLineNumber();
    
    while(getline(infile, str)) {
        if (std::regex_search(str, re)) {
            writeUTF16Line("#END\n", outfile);
            Singleton::shared()->incrementLineNumber();
            return;
        }
        
        str.append("\n");
        writeUTF16Line(str, outfile);
        Singleton::shared()->incrementLineNumber();
    }
}

void translatePPlusToPPL(const std::string &path, std::ofstream &outfile) {
    using namespace std;
    
    Singleton &singleton = *Singleton::shared();
    ifstream infile;
    regex re;
    string utf8;
    string str;
    smatch match;
    
    singleton.pushPath(path);
    
    infile.open(path,ios::in);
    if (!infile.is_open()) {
        cout << "File not found.\n";
        exit(0);
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
        
        while (preprocessor.disregard == true) {
            preprocessor.parse(utf8);
            Singleton::shared()->incrementLineNumber();
            getline(infile, utf8);
        }
        
        if (isPythonBlock(utf8)) {
            writePythonBlock(infile, outfile);
            Singleton::shared()->incrementLineNumber();
            continue;
        }
        
        if (isPPLBlock(utf8)) {
            writePPLBlock(infile, outfile);
            Singleton::shared()->incrementLineNumber();
            continue;
        }
        
        re = R"(\#pragma mode *\(.*\)$)";
        if (regex_match(utf8, re)) {
            writeUTF16Line(utf8 + "\n", outfile);
            Singleton::shared()->incrementLineNumber();
            continue;
        }
        
        
        
        if (preprocessor.parse(utf8)) {
            if (!preprocessor.filename.empty()) {
                // Flagged with #include preprocessor for file inclusion, we process it before continuing.
                translatePPlusToPPL(preprocessor.filename, outfile);
            }
            Singleton::shared()->incrementLineNumber();
            continue;
        }
        
        
        
    
        /*
         We first need to perform pre-parsing to ensure that, in lines such
         as if condition then statement/s end;, the statement/s and end; are
         not on the same line. This ensures proper indentation can be applied
         during the reformatting stage of PPL code.
         
         But we must ignore the line if it's a def, undef or regex and all @
        */
        
        if (!regex_search(utf8, regex(R"(^ *(@[a-z]+ )? *(def|undef|regex) )"))) {
            re = regex(R"(\b(THEN)\b)", regex_constants::icase);
            utf8 = regex_replace(utf8, re, "$1\n");
            
            re = regex(R"(; *\b(ELSE)\b)", regex_constants::icase);
            utf8 = regex_replace(utf8, re, ";\n$1\n");
            
            re = regex(R"(; *(END|UNTIL|ELSE|LOCAL|CONST)?;)", regex_constants::icase);
            utf8 = regex_replace(utf8, re, ";\n$1;");
        }
        
        
        istringstream iss;
        iss.str(utf8);
        
        while(getline(iss, str)) {
            translatePPlusLine(str, outfile);
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
    std::cout << COMMAND_NAME << ": try '" << COMMAND_NAME << " --help' for more information\n";
    exit(0);
}

void info(void) {
    using namespace std;
    cout
    << "Copyright (C) 2023-" << YEAR << " Insoft. All rights reserved.\n"
    << "Insoft "<< NAME << " version, " << VERSION_NUMBER << "\n\n";
}

void help(void) {
    using namespace std;
    cout
    << "Copyright (C) 2023-" << YEAR << " Insoft. All rights reserved.\n"
    << "Insoft "<< NAME << " version, " << VERSION_NUMBER << " (BUILD " << VERSION_CODE << ")\n"
    << "\n"
    << "Usage: " << _basename << " <input-file> [-o <output-file>] [-b <flags>] [-l <pathname>]\n"
    << "\n"
    << "Options:\n"
    << "  -o <output-file>        Specify the filename for generated PPL code.\n"
    << "  -v                      Display detailed processing information.\n"
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
    std::string in_filename, out_filename;
    
    if (argc == 1) {
        error();
        exit(100);
    }
    
    std::string args(argv[0]);
    _basename = basename(args);
    
    for (int n = 1; n < argc; n++) {
        args = argv[n];
        
        if (args == "-o") {
            if ( n + 1 >= argc ) {
                error();
                exit(101);
            }
            out_filename = argv[n + 1];
            if (out_filename.substr(out_filename.length() - 7).compare(".hpprgm") != 0) {
                out_filename += ".hpprgm";
            }
            
            n++;
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
        
        
        if ( args == "-v" ) {
            if ( n + 1 >= argc ) {
                error();
                exit(103);
            }
            args = argv[++n];
            
            if (args.find("a") != std::string::npos) Singleton::shared()->aliases.verbose = true;
            if (args.find("p") != std::string::npos) preprocessor.verbose = true;
            if (args.find("r") != std::string::npos) Singleton::shared()->regexp.verbose = true;
            
            continue;
        }
        
        if (args == "-l") {
            if (++n >= argc) {
                error();
                return 0;
            }
            preprocessor.path = std::string(argv[n]);
            if (preprocessor.path.at(preprocessor.path.length() - 1) == '/') preprocessor.path.resize(preprocessor.path.length() - 1);
            continue;
        }
        
        in_filename = argv[n];
        std::regex re(R"(.\w*$)");
    }
    
    if (!out_filename.length()) {
        out_filename = in_filename;
        if (out_filename.rfind(".")) {
            out_filename.replace(out_filename.rfind("."), out_filename.length() - out_filename.rfind("."), ".hpprgm");
        }
    }
    
    info();
    
    
    std::ofstream outfile;
    outfile.open(out_filename, std::ios::out | std::ios::binary);
    if(!outfile.is_open())
    {
        error();
        return 0;
    }
    
    // The "hpprgm" file format requires UTF-16LE.
    outfile.put(0xFF);
    outfile.put(0xFE);
    
    // Start measuring time
    Timer timer;
    
    std::string str;
    
    str = "#define __pplus";
    preprocessor.parse(str);
    
    str = R"(#define __LIST_LIMIT 10000)";
    preprocessor.parse(str);
    
    str = R"(#define __VERSION )" + std::to_string(NUMERIC_BUILD / 100);
    preprocessor.parse(str);
    
    str = R"(#define __NUMERIC_BUILD )" + std::to_string(NUMERIC_BUILD);
    preprocessor.parse(str);
    
    translatePPlusToPPL(in_filename, outfile);
    
    // Stop measuring time and calculate the elapsed time.
    long long elapsed_time = timer.elapsed();
    
    
    outfile.close();
    
    if (hasErrors() == true) {
        std::cout << ANSI::Red << "ERRORS" << ANSI::Default << "❗\n";
        remove(out_filename.c_str());
        return 0;
    }
    
    // Display elasps time in secononds.
    std::cout << "Completed in " << std::fixed << std::setprecision(2) << elapsed_time / 1e9 << " seconds\n";
    std::cout << "UTF-16LE File '" << out_filename << "' Succefuly Created.\n";
    
    return 0;
}
