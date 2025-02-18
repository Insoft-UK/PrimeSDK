#pragma mode( separator(.,;) integer(h64) )

def BITAND    bit::and;
def BITSR     bit::shift::right;

regex `\bfont.bitmap\b` font[1]
regex `\bfont.glyphs\[([\w\d.:]+)\]` font[2,$1+1]
regex `\bfont.first\b` font[3]
regex `\bfont.last\b` font[4]
regex `\bfont.yAdvance\b` font[5]
regex `\bglyph.xAdvance\b` BITAND(BITSR(glyph, 32), 255)
regex `\bglyph.width\b` BITAND(BITSR(glyph, 16), 255)
regex `\bglyph.height\b` BITAND(BITSR(glyph, 24), 255)
regex `\bglyph.dX\b` BITAND(BITSR(glyph, 40), 255)
regex `\bglyph.dY\b` BITAND(BITSR(glyph, 48), 255) - 256
regex `\bglyph.bitmapOffset\b` BITAND(glyph, 65535)

GLYPH_P:drawGlyph(trgt, g:glyph, x, y, font, color, sX:sizeX, sY:sizeY)
begin
    auto bitmapOffset;
    local auto:glyph = font.glyphs[glyph];
    
    local h = glyph.height;
    local w = glyph.width;
    
    x = x + glyph.dX;
    y = y + (glyph.dY + font.yAdvance) * sizeY;
    
    local bitmap = font.bitmap;
    local bitmapOffset = bit::shift::right(glyph.bitmapOffset, 3) + 1
    
    local bits = bitmap[bitmapOffset];
    local bitPosition = glyph.bitmapOffset & 7 * 8;
    
    bits = bit::shift::right(bits, (64 - bitPosition));
    while h do
        auto xx;
        for xx = 0; xx < w; xx += 1 do
            if bitPosition & 63 == 0 then
                bits = bitmap[bitmapOffset];
                bitmapOffset = bitmapOffset + 1;
            end;

            if bits & 1 == 1 then
                if sizeX == 1 and sizeY == 1 then
                    PIXON_P(trgt, x + xx, y, color);
                else
                    RECT_P(trgt, x + xx * sizeX, y, x + xx * sizeX + sizeX, y + sizeY, color);
                end;
            end;

            bits = bit::shift::right(bits, 1);
        end;

        y = y + sizeY;
        h = h - 1;
    end;
  
    return glyph.xAdvance;
end;

export FONT_P(trgt, text, x, y, auto:font, color)
begin
    local i, auto:asciiList := ASC(text);
 
    for i := 1 ... SIZE(l) do
        if x >= 320 then
            break;
        end;
        
        if asciiList[i] < font.first or asciiList[i] > font.last then
            continue;
        end;
        
        x := x + drawGlyph(trgt, l[i] - font.first + 1, x, y, font, color, 1, 1);
    end;
end;
