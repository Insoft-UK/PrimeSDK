#pragma mode( separator(.,;) integer(h64) )

@global dict White = #FFFFFF:32h, Silver = #C0C0C0:32h, Gray = #808080:32h, Black = #000000:32h, Red = #FF0000:32h, Magenta = #FF00FF:32h, Cardinal = #D03020:32h, Coral = #F08070:32h, Pink = #FFC0C0:32h, Indigo = #400080:32h, Purple = #800080:32h, Violet = #8000FF:32h, Lavender = #C0C0FF:32h, NavyBlue = #000080:32h, Blue = #0000FF:32h, Denim = #1060C0:32h, Lochmara = #0070C0:32h, SkyBlue = #80D0FF:32h, PacificBlue = #00A0C0:32h, Teal = #008080:32h, Cyan = #00FFFF:32h, Aqua = #00FFFF:32h, FirGreen = #003000:32h, OfficeGreen = #008000:32h, Shamrock = #00A060:32h, Apple = #50B040:32h, LimeGreen = #00FF00:32h, Chocolate = #400808:32h, Brown = #905000:32h, Orange = #FF8000:32h, Olive = #808000:32h, Tangerine = #FFD000:32h, Yellow = #FFFF00:32h, Khaki = #F0E090:32h, Tan = #D0B090:32h, Maroon = #800000:32h, SnowWhite = #FFF0F0:32h, Clear = #FF000000:32h Color;

#define COPYRIGHT   "COPYRIGHT 1985 E.C. PUBLICATIONS"
#define END_OF_DATA 999

#define SCREEN_MIDDLE       120
#define SCREEN_CENTRE       160

#include "data.txt"

def DATA MAD.data;
local LnD:MAD.line;
local Scl:MAD.scale = 1.0;

Setup:MAD::Init()
begin
    MAD.line = {0, 0, 0, 0};
    MAD.scale = 1.0;
    
    RECT(Color.White);
    #include "mad.txt";
    BLIT_P(SCREEN_CENTRE - 32, SCREEN_MIDDLE - 32, G1);
    TEXTOUT_P("PRESS ANY KEY", 108, 208);
    TEXTOUT_P(COPYRIGHT, 76, 224, 1);
end;

Read:MAD::Read()
begin
    MAD.line[1] = MAD.data[I];
    MAD.line[2] = MAD.data[I+1];
    MAD.line[3] = MAD.data[I+2];
    MAD.line[4] = MAD.data[I+3];
    I = I + 4;
end;

Draw:MAD::Draw()
begin
    RECT(#00FF00h);
    local C:color = Color.White;
    local FX:x1, FY:y1, LX:x2, LY:y2;

    I:=1;
    while MAD.data[I] != END_OF_DATA do
        if I==1561 then
            color := #006600h;
        end;
        MAD::Read();
    
        x1 = SCREEN_CENTRE + MAD.scale * MAD.line[1];
        y1 = SCREEN_MIDDLE - MAD.scale * MAD.line[2];
        x2 = SCREEN_CENTRE + MAD.scale * MAD.line[3];
        y2 = SCREEN_MIDDLE - MAD.scale * MAD.line[4];
    
        LINE_P(x1, y1, x2, y2, color);
        LINE_P(x1+1, y1, x2+1, y2, color);
    end;
    TEXTOUT_P("WHAT, ME WORRY?", 100, 208);
    TEXTOUT_P(COPYRIGHT, 76, 224, 1);
end;

export RUN()
begin
    MAD::Init;
    WAIT;
    MAD::Draw;
    WAIT;
end;
