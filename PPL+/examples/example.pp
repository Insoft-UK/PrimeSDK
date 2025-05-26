#ifndef __pplplus
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
#endif

#pragma mode( separator(.,,) integer(h64) assignment(=) indentation(2) )
#include <hp>
#include <dictionary>

regex `\bInt8\(([^()]*)\)` SETBITS($1,-7)



#ifndef __pplplus
 When defining a macro with parameters, issues can arise if an argument
 name is adjacent to a letter. To resolve this, you can use numbered
 placeholders like `1$` for the first argument, `2$` for the second, `3$`
 for the third, and so on. This ensures the macro arguments remain uniquely
 identifiable and avoid issues, note that $0 is the identifier.
#endif

#define MacroList(i)      L$1
#define COPYWRITE "Copyright (c) 2023-2025 Insoft. All rights reserved."

//#include "example.ppl"

#PYTHON
#END


#PPL
#END



//#include "g1.hpprgm"
//#include "g2.hpprgm"
//#include "utf16-le.hpprgm"

#include <pplang>


pplang()
begin
    hp::screen::clear(0);
local y;
    var v: variableName = K_Apps;
    10▶variableName;
    
    
    for variableName in 0...7
    end;
    
    switch variableName
    case 1 do
        end;
    end;
end;
