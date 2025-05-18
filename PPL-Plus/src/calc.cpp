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


#include "calc.hpp"
#include "common.hpp"

#include <regex>
#include <vector>
#include <stack>
#include <sstream>
#include <iomanip>
#include <cmath>

using ppl_plus::Calc;

using std::cout;
using std::string;
using std::regex;
using std::smatch;
using std::sregex_iterator;
using std::sregex_token_iterator;
using std::to_string;
using std::fixed;
using std::setprecision;
using std::stringstream;
using std::invalid_argument;
using std::vector;
using std::stack;

static bool _verbose = false;

// Function to check if a character is an operator
static bool isOperator(char c) {
    return (c == '+' || c == '-' || c == '*' || c == '/' || c == '%' || c == '^');
}

static bool isExpresionValid(const string &expression) {
    regex re;
    
    re = R"([\d+\-*\/ πe%&|()]+)";
    return regex_match(expression, re);
}

// Function to get the precedence of an operator
static int precedence(char op) {
    if (op == '+' || op == '-') return 1;
    if (op == '*' || op == '/' || op == '%') return 2;
    if (op == '^') return 3;
    return 0;
}

// Function to perform arithmetic operations
static double applyOperator(const double a, const double b, const char op) {
    switch (op) {
        case '+': return a + b;
        case '-': return a - b;
        case '*': return a * b;
        case '/':
            if (b == 0) {
                cout << MessageType::Error << "#[]: division by zero\n";
                return 0;
            }
            return a / b;
        case '%': return fmod(a,b) < 0 ? b + fmod(a,b) : fmod(a,b);
        case '^': return pow(a, b);
            
        default:
            cout << MessageType::Error << "#[]: unknown '" << op << "' operator\n";
            return 0;
    }
}

static string separateExpression(const string &expression) {
    stringstream separated;
    for (size_t i = 0; i < expression.length(); ++i) {
        if (expression[i] == '(' || expression[i] == ')' || expression[i] == '+' || expression[i] == '-' || expression[i] == '*' || expression[i] == '/' || expression[i] == '%' || expression[i] == '^') {
            // Handle negative numbers (only if '-' is not preceded by a digit)
            if (expression[i] == '-' && (i == 0 || !isdigit(expression[i - 1]))) {
                separated << " " << expression[i];
            } else {
                separated << " " << expression[i] << " ";
            }
        } else {
            separated << expression[i];
        }
    }
    return separated.str();
}

// Function to convert infix expression to postfix (RPN)
static vector<string> infixToPostfix(const string &expression) {
    vector<string> output;
    stack<char> operators;
    
    regex re(R"([^ ]+)");
    for(auto it = sregex_iterator(expression.begin(), expression.end(), re); it != sregex_iterator(); ++it ) {
        string result = it->str();
        
        if (isdigit(result[0]) || (result.length() > 1 && result[0] == '-')) {
            output.push_back(it->str());
            continue;
        }
        
        if (isOperator(result.c_str()[0])) {
            while (!operators.empty() && precedence(operators.top()) >= precedence(result.c_str()[0])) {
                output.push_back(string(1, operators.top()));
                operators.pop();
            }
            operators.push(result.c_str()[0]);
            continue;
        }
        
        if (result.c_str()[0] == '(') {
            operators.push('(');
            continue;
        }
        
        if (result.c_str()[0] == ')') {
            while (!operators.empty() && operators.top() != '(') {
                output.push_back(string(1, operators.top()));
                operators.pop();
            }
            if (operators.empty()) {
                cout << MessageType::Error << "#[]: missing '(' in expression '" << expression << "'\n";
                continue;
            }
                
            operators.pop();  // Remove the '(' from stack
            continue;
        }
        
        cout << MessageType::Error << "#[]: uknown '" << result << "' in expression '" << expression << "'\n";
    }

    while (!operators.empty()) {
        output.push_back(string(1, operators.top()));
        operators.pop();
    }
    
    if (_verbose) {
        cout << MessageType::Verbose << "calc: RPN: ";
        for (auto it = output.begin(); it != output.end(); ) {
            cout << *it;
            if (++it != output.end()) {
                cout << ",";
            }
        }
        cout << "\n";
    }

    return output;
}

// Function to evaluate a postfix expression
static double evaluatePostfix(const vector<string>& postfix) {
    stack<double> values;

    for (const string &token : postfix) {
        if (isdigit(token[0]) || (token.length() > 1 && token[0] == '-')) {
            values.push(stod(token));
            continue;
        }
        if (isOperator(token[0])) {
            double b = values.top(); values.pop();
            double a = values.top(); values.pop();
            values.push(applyOperator(a, b, token[0]));
        }
    }

    return values.top();
}

// Function to evaluate an infix expression
static double evaluateExpression(const string &expression) {
    vector<string> postfix = infixToPostfix(expression);
    return evaluatePostfix(postfix);
}


// MARK: -
// MARK: PPL Integer Number Handerling

// Function to convert a 64-bit number to a specified bit width
static uint64_t convertToBitWidth(uint64_t num, int bitWidth) {
    if (bitWidth <= 0) {
        // If bit width is 0 or negative, return 0 (no bits to retain)
        return 0;
    }
    
    if (bitWidth >= 64) {
        // If bit width is 64 or more, return the number as-is
        return num;
    }

    // Create a mask with the desired number of bits
    uint64_t mask = (1ULL << bitWidth) - 1;

    // Apply the mask to the number and return the result
    return num & mask;
}

// Function to perform two's complement on a number with a given bit width
static int64_t twosComplement(int64_t num, int bitWidth) {
    if (bitWidth <= 0 || bitWidth > 64) {
        throw invalid_argument("Bit width must be between 1 and 64.");
    }

    // Create a mask to fit within the specified bit width
    int64_t mask = (1LL << bitWidth) - 1;

    // Mask the number to limit it to the bit width
    num &= mask;

    // Check if the number is negative in two's complement by checking the most significant bit
    int64_t signBit = 1LL << (bitWidth - 1);
    if (num & signBit) {
        // The number is negative, so we apply two's complement
        num = num - (1LL << bitWidth);  // Subtract 2^bitWidth to get the negative value
    }

    return num;
}

static string decimalSignedNumber(int64_t num, int bitWidth) {
    if (bitWidth < 1 || bitWidth > 64) bitWidth = 64;
    
    num = convertToBitWidth(num, bitWidth);
    num = twosComplement(num, bitWidth);

    return to_string(num);
}

static string decimalUnsignedNumber(int64_t num, int bitWidth) {
    if (bitWidth < 1 || bitWidth > 64) bitWidth = 64;
    
    num = convertToBitWidth(num, bitWidth);

    return to_string(num);
}

// Function to convert a string with PPL-style integer number to return a base 10 number
static string convertPPLIntegerNumberToBase10(const string &str) {
    regex re;
    smatch match;
    
    re = R"(#([\dA-F]+)(?::(-)?(6[0-4]|[1-5][0-9]|[1-9]))?([odh])?)";
    if (!regex_search(str, match, re)) return str;
    
    /*
     Group 1 The hex part of the string.
     Group 2 The signed part if given!.
     Group 3 The bit width of the integer number if also given!
     Group 4 The base type, should always be `h` in this case.
     */
    
    int base = 10;
    if (match.str(4) == "h") base = 16;
    if (match.str(4) == "o") base = 8;
    
    int64_t num = stol(match.str(1), nullptr, base);
    int bitWidth = match.str(3).empty() ? 64 : atoi(match.str(3).c_str());
    
    if (match.str(2).empty())
        return decimalUnsignedNumber(num, bitWidth);
    
    return decimalSignedNumber(num, bitWidth);
}

// Function to convert a string with PPL-style integer number to a plain base 10 number
static void convertPPLStyleNumbersToBase10(string &str) {
    regex re;
    smatch match;
    string s;
    
    re = R"(#([\dA-F])+(?::-?\d+)?([odh])?)";
    while (regex_search(str, match, re)) {
        /*
         Group 1 The number part of the string.
         Group 2 The base type, should be `h`, `d` or `o` if given!.
         */
        
        s = convertPPLIntegerNumberToBase10(match.str());
        str = str.replace(match.position(), match.length(), s);
    }
}


// MARK: - Public Methods

bool Calc::evaluateMathExpression(string &str)
{
    regex re;
    smatch match;
    
    if (!isExpresionValid(str)) return false;
    
    string expression = str;
    convertPPLStyleNumbersToBase10(expression);
    
    expression = regex_replace(expression, regex(R"(e)"), "2.71828182845904523536028747135266250");
    expression = regex_replace(expression, regex(R"(π|pi)"), "3.14159265358979323846264338327950288");
    
    strip(expression);
    
    expression = separateExpression(expression);
    double result = evaluateExpression(expression);
    
    stringstream ss;
    ss << fixed << setprecision(10) << result;
    expression = ss.str();
    expression.erase ( expression.find_last_not_of('0') + 1, string::npos );
    if (expression.at(expression.length() - 1) == '.') {
        expression.resize(expression.length() - 1);
    }
    str = expression;
    
    return true;
}

bool Calc::parse(string &str)
{
    regex re;
    smatch match;
    
    
    
    re = R"(\\( *\d{1,2}|F|C|R)?(?:\[|`)(.*)(?:\]|`))";
    while (regex_search(str, match, re)) {
        
        string matched = match.str();
        
        convertPPLStyleNumbersToBase10(matched);
        
        string expression;
        int scale = 0;
        
        matched = regex_replace(matched, regex(R"(e)"), "2.71828182845904523536028747135266250");
        matched = regex_replace(matched, regex(R"(π)"), "3.14159265358979323846264338327950288");
        
        strip(matched);
        
        char output = 0;
        
        auto it = sregex_token_iterator {
            matched.begin(), matched.end(), re, {1, 2}
        };
        if (it != sregex_token_iterator()) {
            if (it->matched) {
                if (isdigit(it->str().c_str()[0])) {
                    scale = atoi(it->str().c_str());
                } else {
                    output = it->str().c_str()[0];
                }
            }
            else {
                scale = -1; // -1 means auto scale
            }
            it++;
            expression = *it;
        }
        
        parse(expression);
        
        
        expression = separateExpression(expression);
        double result = evaluateExpression(expression);
        
        switch (output) {
            case 'F':
                result = floor(result);
                break;
                
            case 'C':
                result = ceil(result);
                break;
                
            case 'R':
                result = round(result);
                break;
                
            default:
                break;
        }
        
        stringstream ss;
        ss << fixed << setprecision(scale > -1 ? scale : 10) << result;
        string s = ss.str();
        
        if (scale < 0) {
            s.erase ( s.find_last_not_of('0') + 1, string::npos );
            s.erase ( s.find_last_not_of('.') + 1, string::npos );
        }
        
        
        str = str.replace(match.position(), match.length(), s);
        return true;
    }

    return false;
}




