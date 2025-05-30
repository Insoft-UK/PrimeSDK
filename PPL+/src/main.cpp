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
#include <unordered_set>

#include "timer.hpp"
#include "singleton.hpp"
#include "common.hpp"

#include "preprocessor.hpp"
#include "dictionary.hpp"
#include "alias.hpp"
#include "strings.hpp"
#include "calc.hpp"
#include "hpprgm.hpp"
#include "utf.hpp"
#include "comments.hpp"

#include "../version_code.h"

#define NAME "PPL+ Pre-Processor for PPL"
#define COMMAND_NAME "ppl+"
#define INDENT_WIDTH indentation

static unsigned int indentation = 1;

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
using std::vector;
using std::stringstream;

namespace fs = std::filesystem;
namespace rc = std::regex_constants;

void translatePPLPlusToPPL(const fs::path &path, ofstream &outfile);

static Preprocessor preprocessor = Preprocessor();
static Strings strings = Strings();
static string assignment = "=";
static vector<string> operators = { ":=", "==", "▶", "≥", "≤", "≠" };

// MARK: - Extensions

namespace std::filesystem {
    std::string expand_tilde(const string &path) {
        if (!path.empty() && path.starts_with("~")) {
#ifdef _WIN32
            const char* home = std::getenv("USERPROFILE");
#else
            const char* home = std::getenv("HOME");
#endif
            
            if (home) {
                return string(home) + path.substr(1);  // Replace '~' with $HOME
            }
        }
        return path;  // return as-is if no tilde or no HOME
    }
}

#if __cplusplus >= 202302L
    #include <bit>
    using std::byteswap;
#elif __cplusplus >= 201103L
    #include <cstdint>
    namespace std {
        template <typename T>
        T bitswap(T u)
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
    }
#else
    #error "C++11 or newer is required"
#endif

// MARK: - Other

void terminator() {
    cout << MessageType::CriticalError << "An internal pre-processing problem occurred. Please review the syntax before this point.\n";
    exit(0);
}
void (*old_terminate)() = std::set_terminate(terminator);


string replace_operators(const string &input)
{
    string output;
    output.reserve(input.size());  // Reserve space to reduce reallocations

    for (std::size_t i = 0; i < input.size(); ++i) {
        if (i + 1 < input.size()) {
            // Lookahead for 2-character operators
            if (input[i] == '>' && input[i + 1] == '=') {
                output += "≥";
                ++i;
                continue;
            }
            if (input[i] == '<' && input[i + 1] == '=') {
                output += "≤";
                ++i;
                continue;
            }
            if (input[i] == '=' && input[i + 1] == '>') {
                output += "▶";
                ++i;
                continue;
            }
            if (input[i] == '<' && input[i + 1] == '>') {
                output += "≠";
                ++i;
                continue;
            }
        }

        // Default: copy character
        output += input[i];
    }

    return output;
}

string expand_assignment_equals(const string &input)
{
    string output;
    output.reserve(input.size() * 2);  // Worst-case growth

    for (size_t i = 0; i < input.size(); ++i) {
        if (input[i] == '=') {
            // Check if preceded by ':' (:=), do not expand
            if (i > 0 && input[i - 1] == ':') {
                output += '=';
            }
            // Check if followed by '=' (already '=='), copy as-is
            else if (i + 1 < input.size() && input[i + 1] == '=') {
                output += "==";
                ++i;  // skip next '='
            }
            else {
                output += "==";
            }
        } else {
            output += input[i];
        }
    }

    return output;
}

string convert_assign_to_colon_equal(const string &input)
{
    string output;
    output.reserve(input.size() * 2);  // Conservative buffer size

    for (size_t i = 0; i < input.size(); ++i) {
        if (input[i] == '=') {
            // Check for '=='
            if (i + 1 < input.size() && input[i + 1] == '=') {
                output += "==";
                ++i;
            }
            // Check for ':=' (don't modify)
            else if (i > 0 && input[i - 1] == ':') {
                output += '=';
            }
            // Replace single '=' with ':='
            else {
                output += ":=";
            }
        } else {
            output += input[i];
        }
    }

    return output;
}


string normalize_operators(const string &input) {
    // List of all operators to normalize
        
        string result;
        size_t i = 0;

        while (i < input.size()) {
            bool matched = false;

            for (const string& op : operators) {
                if (input.compare(i, op.size(), op) == 0) {
                    if (!result.empty() && result.back() != ' ') result += ' ';
                    result += op;
                    i += op.size();
                    if (i < input.size() && input[i] != ' ') result += ' ';
                    matched = true;
                    break;
                }
            }

            if (!matched) {
                result += input[i++];
            }
        }

        // Final cleanup: collapse multiple spaces
        istringstream iss(result);
        string word, cleaned;
        while (iss >> word) {
            if (!cleaned.empty()) cleaned += ' ';
            cleaned += word;
        }

        return cleaned;
}

string fix_unary_minus(const string &input)
{
    istringstream iss(input);
    vector<string> tokens;
    string token;

    // Tokenize input by whitespace
    while (iss >> token) {
        tokens.push_back(token);
    }

    vector<string> corrected;
    for (size_t i = 0; i < tokens.size(); ++i) {
        if (tokens[i] == "-" && i + 1 < tokens.size()) {
            const string& next = tokens[i + 1];

            // Check if next token is number or identifier
            if (isdigit(next[0]) || isalpha(next[0])) {
                // If previous token is an operator or it's the first token
                if (i == 0 || tokens[i - 1] == ":=" || tokens[i - 1] == "=" ||
                    tokens[i - 1] == "*" || tokens[i - 1] == "/" || tokens[i - 1] == "+" ||
                    tokens[i - 1] == "(") {
                    corrected.push_back("-" + next);
                    ++i; // Skip next token
                    continue;
                }
            }
        }

        corrected.push_back(tokens[i]);
    }

    // Rebuild string
    string result;
    for (const auto& t : corrected) {
        if (!result.empty()) result += ' ';
        result += t;
    }

    return result;
}

vector<string> split_commas(const string &input)
{
    vector<string> result;
    stringstream ss(input);
    string token;

    while (getline(ss, token, ',')) {
        result.push_back(token);
    }

    return result;
}

vector<string> split_escaped_commas(const string &input)
{
    vector<string> result;
    string token;
    bool escape = false;

    for (size_t i = 0; i < input.size(); ++i) {
        char c = input[i];

        if (escape) {
            // Handle escaped character
            if (c == ',') {
                token += ',';  // Turn \, into ,
            } else {
                token += '\\'; // Preserve the backslash
                token += c;
            }
            escape = false;
        }
        else if (c == '\\') {
            escape = true;
        }
        else if (c == ',') {
            result.push_back(token);
            token.clear();
        }
        else {
            token += c;
        }
    }

    result.push_back(token); // Add last token
    return result;
}

string process_escapes(const string &input) {
    string result;
    for (size_t i = 0; i < input.length(); ++i) {
        if (input[i] != '\\' || i + 1 == input.length()) {
            result += input[i];
            continue;
        }
        char next = input[i + 1];
        if (next == 'n') {
            result += '\n';
            ++i; // skip the next character
            continue;
        }
        if (next == 's') {
            result += ' ';
            ++i; // skip the next character
            continue;
        }
        if (next == 't') {
            result += (string(INDENT_WIDTH, ' '));
            ++i; // skip the next character
            continue;
        }
        result += input[i]; // add the backslash
        result += next;     // add the next character as is
        ++i; // skip the next character
    }
    return result;
}


string to_lower(const string &s)
{
    std::string result = s;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return result;
}

string to_upper(const string &s)
{
    std::string result = s;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return std::toupper(c); });
    return result;
}

string replace_words(const string& input,
                          const vector<std::string>& words,
                          const string& replacement)
{
    // Create lowercase word set
    std::unordered_set<string> wordSet;
    for (const auto& w : words) {
        wordSet.insert(to_lower(w));
    }

    std::string result;
    size_t i = 0;
    
    while (i < input.size()) {
        if (!isalpha(static_cast<unsigned char>(input[i])) && input[i] != '_') {
            result += input[i];
            ++i;
            continue;
        }
        size_t start = i;
        
        while (i < input.size() && (isalpha(static_cast<unsigned char>(input[i])) || input[i] == '_')) {
            ++i;
        }
        
        string word = input.substr(start, i - start);
        string lowerWord = to_lower(word);
        
        if (wordSet.count(lowerWord)) {
            result += replacement;
            continue;
        }
        
        result += word;
    }
    
    return result;
}

string capitalize_words(const string &input, const std::unordered_set<std::string> &words)
{
    // Create lowercase word set
    std::unordered_set<string> wordset;
    for (const auto& w : words) {
        wordset.insert(to_lower(w));
    }
    
    string result;
    size_t i = 0;
    
    while (i < input.size()) {
        if (!isalpha(static_cast<unsigned char>(input[i])) && input[i] != '_') {
            result += input[i];
            ++i;
            continue;
        }
        size_t start = i;
        
        while (i < input.size() && (isalpha(static_cast<unsigned char>(input[i])) || input[i] == '_')) {
            ++i;
        }
        
        string word = input.substr(start, i - start);
        string lowercase = to_lower(word);
        
        if (wordset.count(lowercase)) {
            result += to_upper(lowercase);
            continue;
        }
        
        result += word;
    }
    
    return result;
}

// MARK: - PPL+ To PPL Translater...
void reformatPPLLine(string &str) {
    regex re;
    
    Strings strings = Strings();
    strings.preserveStrings(str);
    
    str = normalize_operators(str);
    str = fix_unary_minus(str);
    
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
    
    if (ln.substr(0,2) == "//") {
        ln = ln.insert(0, string(singleton->scopeDepth * INDENT_WIDTH, ' '));
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
 
    // Remove any comments.
    singleton->comments.preserveComment(ln);
    singleton->comments.removeComment(ln);
    
    ln = normalize_whitespace(ln);

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
    
    // Keywords
    ln = capitalize_words(ln, {
        "begin", "end", "return", "kill", "if", "then", "else", "xor", "or", "and", "not",
        "case", "default", "iferr", "ifte", "for", "from", "step", "downto", "to", "do",
        "while", "repeat", "until", "break", "continue", "export", "const", "local", "key"
    });
    
    ln = capitalize_words(ln, {"log", "cos", "sin", "tan", "ln", "min", "max"});
    
    ln = regex_replace(ln, regex(R"(\s*([^\w \s])\s*)"), "$1");
    
    ln = replace_operators(ln);
    
    // PPL by default uses := instead of C's = for assignment. Converting all = to PPL style :=
    if (assignment == "=") {
        ln = convert_assign_to_colon_equal(ln);
    } else {
        ln = expand_assignment_equals(ln);
    }
    
    //MARK: alias parsing
    
    re = R"(^alias ([A-Za-z_]\w*(?:::[a-zA-Z]\w*)*):=([a-zA-Z][\w→]*(?:\.[a-zA-Z][\w→]*)*);$)";
//    re = R"(^alias ([A-Za-z_]\w*(?:::[a-zA-Z]\w*)*):=([a-zA-Z\x{7F}-\x{FFFF}][\x{7F}-\x{FFFF}\w]*(?:\.[a-zA-Z\x{7F}-\x{FFFF}][\x{7F}-\x{FFFF}\w]*)*);$)";
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
    
    ln = process_escapes(ln);
   
    strings.restoreStrings(ln);
    singleton->comments.restoreComment(ln);
    
    ln += '\n';
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



void embedPPLCode(const string &filepath, ofstream &os)
{
    ifstream is;
    string str;
    
    fs::path path = filepath;
    is.open(filepath, ios::in);
    if (!is.is_open()) return;
    if (path.extension() == ".hpprgm" || path.extension() == ".ppl") {
        std::wstring wstr = hpprgm::load(filepath);
        
        if (!wstr.empty()) {
            str = utf::to_utf8(wstr);
            str = regex_replace(str, regex(R"(^ *#pragma mode *\(.+\) *\n+)"), "");
            utf::write(str, os);
            is.close();
            return;
        }
    }
    while (getline(is, str)) {
        str += '\n';
        utf::write(str, os);
    }
    is.close();
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
    return str.find("#PYTHON") != string::npos;
}

bool isPPLBlock(const string str) {
    return str.find("#PPL") != string::npos;
}

void processPPLBlock(ifstream &infile, ofstream &outfile) {
    string str;
    
    Singleton::shared()->incrementLineNumber();
    
    while(getline(infile, str)) {
        if (str.find("#END") != string::npos) {
            Singleton::shared()->incrementLineNumber();
            return;
        }
        
        str.append("\n");
        utf::write(str, outfile);
        Singleton::shared()->incrementLineNumber();
    }
}

void processPythonBlock(ifstream &infile, ofstream &outfile, string &input) {
    regex re;
    string str;
    Aliases aliases;
    aliases.verbose = Singleton::shared()->aliases.verbose;
    
    Singleton::shared()->incrementLineNumber();
    
    input = clean_whitespace(input);

    size_t start = input.find('(');
    size_t end = input.find(')', start);
        
    if (start != string::npos && end != string::npos && end > start) {
        vector<string> arguments = split_commas(input.substr(start + 1, end - start - 1));
        input = "#PYTHON (";
        int index = 0;
        
        Aliases::TIdentity identity = {
            .type = Aliases::Type::Argument
        };
        for (const string &argument : arguments) {
            if (index++) input.append(",");
            start = argument.find(':');
            
            if (start != string::npos) {
                input.append(argument.substr(0, start));
                identity.identifier = argument.substr(start + 1, argument.length() - start - 1);
                identity.real = "argv[" + to_string(index) + "]";
                aliases.append(identity);
                continue;
            }
            input.append(argument);
        }
        input.append(")");
    }
        
    
    utf::write(input + '\n', outfile);
    

    while(getline(infile, str)) {
        if (str.find("#END") != string::npos) {
            utf::write("#END\n", outfile);
            Singleton::shared()->incrementLineNumber();
            return;
        }
        
        str = aliases.resolveAllAliasesInText(str);
        str.append("\n");
        utf::write(str, outfile);
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

    bool pragma = false;
    
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
        
        utf8 = ppl_plus::Comments().removeTripleSlashComment(utf8);
        
        if (utf8.find("#EXIT") != string::npos) {
            break;
        }
        
        while (preprocessor.disregard == true) {
            preprocessor.parse(utf8);
            Singleton::shared()->incrementLineNumber();
            getline(infile, utf8);
        }
        
        if (isPythonBlock(utf8)) {
            processPythonBlock(infile, outfile, utf8);
            continue;
        }
        
        if (isPPLBlock(utf8)) {
            processPPLBlock(infile, outfile);
            continue;
        }
        
        // Handle `#pragma mode` for PPL+
        if (utf8.find("#pragma mode") != string::npos) {
            if (pragma) {
                Singleton::shared()->incrementLineNumber();
                continue;
            }
            pragma = true;
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
                if (it->str(1) == "operators") {
                    operators = split_escaped_commas(normalize_whitespace(it->str(2)));
                    continue;
                }
                utf8.append(it->str() + " ");
            }
            utf8.append(")");
            
            utf::write(utf8 + "\n", outfile);
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
    
        utf8 = replace_words(utf8, {"endif", "wend", "next"}, "END");
        utf8 = replace_words(utf8, {"try"}, "IFERR");
        utf8 = replace_words(utf8, {"catch"}, "THEN");
  
        /*
         We first need to perform pre-parsing to ensure that, in lines such
         as if condition then statement/s end;, the statement/s and end; are
         not on the same line. This ensures proper indentation can be applied
         during the reformatting stage of PPL code.
         
         But we must ignore the line if it's a regex and all @
         */
        
        
        
        if (!regex_search(utf8, regex(R"(^ *\b(?:regex|dict) +)"))) {
            utf8 = regex_replace(utf8, regex(R"(\b(THEN|IFERR|REPEAT)\b)", rc::icase), "$1\n");
            utf8 = regex_replace(utf8, regex(R"((; *)(THEN|UNTIL)\b)", rc::icase), "$1\n$2");
            utf8 = regex_replace(utf8, regex(R"(; *\b(ELSE)\b)", rc::icase), ";\n$1\n");
            utf8 = regex_replace(utf8, regex(R"(; *(END|UNTIL|ELSE|LOCAL|CONST)\b;)", rc::icase), ";\n$1;");
            utf8 = regex_replace(utf8, regex(R"((.+)\bBEGIN\b)", rc::icase), "$1\nBEGIN");
        }
            
    
        istringstream iss;
        iss.str(utf8);
        
        while(getline(iss, str)) {
            translatePPLPlusLine(str, outfile);
            utf::write(str, outfile);
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
    << "Insoft PPL+" << string(YEAR).substr(2) << " Pre-Processor for PPL\n\n";
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
        
        in_filename = fs::expand_tilde(argv[n]);
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
    loadRegexLibs(fs::expand_tilde("~/GitHub/PrimeSDK/Package Installer/package-root/Applications/HP/PrimeSDK/lib"), true);
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
    if (elapsed_time / 1e9 < 1.0) {
        cout << "Completed in " << fixed << setprecision(2) << elapsed_time / 1e6 << " milliseconds\n";
    } else {
        cout << "Completed in " << fixed << setprecision(2) << elapsed_time / 1e9 << " seconds\n";
    }
    if (showpath)
        cout << "UTF-16LE file at \"" << out_filename << "\" succefuly created.\n";
    else
        cout << "UTF-16LE file " << fs::path(out_filename).filename() << " succefuly created.\n";
    
            
    return 0;
}
