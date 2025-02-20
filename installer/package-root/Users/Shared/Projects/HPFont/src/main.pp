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
#pragma mode( separator(.,;) integer(h64) )

def BITAND bitwise::and;
def BITSR  bitwise::shift_right;

regex `\.{3}` to

GLYPH_P(trgt, ascii, x, y, fnt, color, sizeX, sizeY)
BEGIN
 local g:glyph := fnt[2, ascii];
 def bitwise::and(glyph, 65535) glyph.bitmapOffset;
 def bitwise::and(bitwise::shift_right(glyph, 16), 255) glyph.width;
 def bitwise::and(bitwise::shift_right(glyph, 24), 255) glyph.height;
 def bitwise::and(bitwise::shift_right(glyph, 32), 255) glyph.xAdvance;
 def bitwise::and(bitwise::shift_right(glyph, 40), 255) glyph.dX;
 def bitwise::and(bitwise::shift_right(glyph, 48), 255) glyph.dY;
 
 local xAdvance := glyph.xAdvance;
 
  if bitwise::and(glyph, #FFFFFFFF) == 0 then
    return xAdvance * sizeX;
  end;
  
  local w, h, dX, dY, xx;
  
  local yAdvance := fnt[5];

  w := glyph.width;
  h := glyph.height;
 
  dX := glyph.dX;
  dY := glyph.dY;
 
  x := x + dX * sizeX;
  y := y + (yAdvance + dY * sizeY);
  
  local bitmap := fnt[1];

  local offset := glyph.bitmapOffset;
  local bitPosition := bitwise::and(offset, 7) * 8;
  offset := bitwise::shift_right(offset, 3) + 1;
  local bits := bitwise::shift_right(bitmap[offset], bitPosition);
  
  repeat
    for xx := 1 ... w do
      if bitPosition == 64 then
        bitPosition := 0;
        offset := offset + 1;
        bits := bitmap[offset];
      end;
     
      if bitwise::and(bits, 1) == 1 then
        if sizeX == 1 AND sizeY == 1 then
          PIXON_P(trgt, x + xx,y, color);
        ELSE
          RECT_P(trgt, x + xx * sizeX, y, x + xx * sizeX + sizeX - 1, y + sizeY - 1, color);
        end;
      end;
      
      bitPosition := bitPosition + 1;
      bits := bitwise::shift_right(bits, 1);
    end;
   
    y + sizeY ▶ y;
    h - 1 ▶ h;
  until h == 0;
  
  return xAdvance * sizeX;
end;

export FONT_P(trgt, text, x, y, fnt, color, sizeX, sizeY)
BEGIN
  local i, l := ASC(text);
 
  for i := 1 ... SIZE(l) do
    if x >= 320 then
      break;
    end;
    if l[i] < fnt[3] OR l[i] > fnt[4] then
      continue;
    end;
    x := x + GLYPH_P(trgt, l[i] - fnt[3] + 1, x, y, fnt, color, sizeX, sizeY);
  end;
end;
