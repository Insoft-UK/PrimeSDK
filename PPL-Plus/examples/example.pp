@disregard
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
@end
#pragma mode( separator(.,,) integer(h64) )


#include <pplang>
#include <cplang>

@disregard
 When defining a macro with parameters, issues can arise if an argument
 name is adjacent to a letter. To resolve this, you can use numbered
 placeholders like `1$` for the first argument, `2$` for the second, `3$`
 for the third, and so on. This ensures the macro arguments remain uniquely
 identifiable and avoid issues, note that $0 is the identifier.
@end

#define MacroList(i)      L$1
#define COPYWRITE "Copyright (c) 2023-2025 Insoft. All rights reserved."

#PYTHON
#END


Example:AVeryLongName(p:first, q:second)
begin
    // Local Variables with aliases
    local a: alpha, b: beta;
    
    
    local e: myEvt;
    dict Event myEvt;
    
    
    myEvt.type;
    
    
    
    // Macro
    alpha := MacroList(1);
    
    // b
    beta := first + second;
    
    if a <= b and b > 10 then a = a + 1; end;
    if a != b and b > 10 then a = a + 1; end;
    if a <> b and b > 10 then a = a + 1; end;
    if a >= b or b < 20 then a = a + 1; end;
    if a == b or b < 20 then a = a + 1; end;
    
    // Pre-Calculate
    #define VALUE 5
    local pre_calculated := \2`10.0 + VALUE + #Ah`;
    local hex := \`#A:2h * 2`;
    local bin := \`#1111:3b * 2`;
    local oct := \`#1111:3o * 2`;
    local dec := \`#15:3d * 2`;
    
    // LOCAL auto variable name
    auto iAmVerryLong;

#
end;

auto:myFunction()
begin
    AVeryLongName(2,5);
end;

myFunction();

void clang()
{
    var i;
    
    for (var A=0; A <= 10; A = A + 1) {
        B = B + A;
        if (a == 9) {
            break;
        }
    }
    
    while (A > 0) {
        A = 0;
    }
    
    if ((A == B && B != 0) || A <= -1) {
    } else {
    }
    
    do {
        A = A - 1;
    } while (A == 1);
    
    switch (A) {
    case 1:
        switch (B) {
        case 2:
            end;
        }
        end;
    }
}

pplang()
begin
    var v: variableName = 0;
    
    for variableName in 0...7
    end;
    
    switch variableName
    case 1 do
        end;
    end;
end;


