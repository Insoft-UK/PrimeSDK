@disregard
 Copyright © 2024 Insoft. All rights reserved.
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
@end

auto:HEX(d:decimal)
begin
    local h,l;
    h = decimal >> 4;
    l = decimal & 15;
    return CHAR(ifte(h<10,48,55) + h) + CHAR(ifte(l<10,48,55) + l);
end;

auto:drawTextCentered(auto:string, auto:atX, auto:ofY, auto:fontSize)
begin
    local auto:size;
    dict Size size;
 
    size = TEXTSIZE(string, fontSize);
    TEXTOUT_P(string, G9, atX - size.width / 2, ofY, fontSize, Color.White);
end;
