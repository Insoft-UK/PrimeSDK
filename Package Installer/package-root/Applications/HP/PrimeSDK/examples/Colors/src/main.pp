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
#include <pplang>
#include <clang>
#include <hp>
#include <dictionary>

#include "defines.pp"
local channel:Colors.channel = 0;

local colorsH:Colors.h = 0, colorsS:Colors.s = 100, colorsV:Colors.v = 100;
local colorsR:Colors.r = 255, colorsG:Colors.g = 0, colorsB:Colors.b = 0;
local colorsUT:Colors.unitType = UnitType.Decimal;
local colorsCS:Colors.colorSpace = ColorSpace.HSV;

#include "common.pp"
Colors::Setup()
begin
    local n, data;
    DIMGROB_P(G9, 320, 240, 0);
    
    RECT(Color.Black);
    BLIT_P(G9, __SCREEN);
    
    data = emptylist;
    for n from 0 to 359 step 2 do
        data[n / 2 + 1] = R→B(HSV(n, 100, 100), 64, Base.Hex) + BITSL(R→B(HSV(n + 1, 100, 100), 64, Base.Hex), 32);
    end;
    DIMGROB_P(HUE, 360, 1, data);
    
    data := emptylist;
    for n from 0 to 255 step 2 do
        data[n / 2 + 1] = R→B(RGB(0, 0, 0, 255 - n), 64, Base.Hex) + BITSL(R→B(RGB(0, 0, 0, 255 - (n + 1)), 64, Base.Hex), 32);
    end;
    DIMGROB_P(BLACK_TO_TRANSPARENT_OVERLAY, 1, 256, data);
    
    data := emptylist;
    for n from 0 to 255 step 2 do
        local rgb = {RGB(255, 255, 255, n), RGB(255, 255, 255, n + 1)};
        data[n / 2 + 1] = R→B(RGB(255, 255, 255, n), 64, Base.Hex) + BITSL(R→B(RGB(255, 255, 255, n + 1), 64, Base.Hex), 32);
    end;
    DIMGROB_P(TRANSPARENT_TO_WHITE_OVERLAY, 256, 1, data);
end;

Colors::DrawMenu()
begin
    /// 0-51, 53-104, (106-157, 159-210), 212-263, 265-319
    RECT_P(G9, 0, 220, 51, 239, #666666:32h);
    RECT_P(G9, 53, 220, 104, 239, #666666:32h);
    RECT_P(G9, 106, 220, 210, 239, #666666:32h);
    RECT_P(G9, 212, 220, 263, 239, #666666:32h);
    RECT_P(G9, 265, 220, 319, 239, #666666:32h);
   
    drawTextCentered("%",25,225,2);
    drawTextCentered("d",78,225,2);
    drawTextCentered("-         HUE         +",158,225,2);
    drawTextCentered("HSV",237,225,2);
    drawTextCentered("HSL",290,225,2);
    
    if Colors.unitType == UnitType.Percentage then
        TEXTOUT_P("•", G9, 4, 225, 2, Color.White);
    else
        TEXTOUT_P("•", G9, 57, 225, 2, Color.White);
    end;
    
    if Colors.colorSpace == ColorSpace.HSV then
        TEXTOUT_P("•", G9, 216, 225, 2, Color.White);
    else
        TEXTOUT_P("•", G9, 269, 225, 2, Color.White);
    end;
end;

Colors::DrawHue(h)
begin
    BLIT_P(G9, 10, 148, 310, 168, HUE);
    
    X = IP(h / 360 * 300) + 10;
    Y = 148;
    RECT_P(G9, X-3, Y-3, X+2, Y+22, Color.Black, Color.Clear);
    RECT_P(G9, X-2, Y-2, X+2, Y+21, Color.White, Color.Clear);
end;

Colors::DrawSV(h,s,v)
begin
    RECT_P(G9, 10, 10, 137, 137, HSV(h, 100, 100));
    BLIT_P(G9, 10, 10, 138, 138, G4);
    BLIT_P(G9, 10, 10, 138, 138, G3);

    X := s / 100 * 127 + 10;
    Y := (1 - v / 100) * 127 + 10;
    RECT_P(G9, X-3, Y-3, X+2, Y+2, Color.Black, Color.Clear);
    RECT_P(G9, X-2, Y-2, X+1, Y+1, Color.White, Color.Clear);
end;

Colors::Update()
begin
    RECT(G9,#222222:32h);

    Colors::DrawMenu;
    Colors::DrawHue(Colors.h);
    Colors::DrawSV(Colors.h, Colors.s, Colors.v);
    
    // Color Picked
    RECT_P(G9, 154, 10, 309, 42, HSV(Colors.h, Colors.s, Colors.v));
    
    local hsl, rgb, cmyk, hex;
    
    rgb = {Colors.r, Colors.g, Colors.b};
    hsl = HSVtoHSL(Colors.h, Colors.s, Colors.v);
    cmyk = RGBtoCMYK(Colors.r, Colors.g, Colors.b);
    hex = "#" + HEX(Colors.r) + HEX(Colors.g) + HEX(Colors.b);
    
    L0 := TEXTSIZE(hex,7);
    TEXTOUT_P(hex,G9,160-L0(1)/2,180,7,Color.White);
    
    
    local n, ts, info;
    
    /// RGB Info
    info = {"R:","G:","B:"};
    X = 138;
    Y = 51;
    for n = 1 to 3 step 1 do
        ts = TEXTSIZE(info[n],3);
        TEXTOUT_P(info[n],G9,X+15-ts[1],Y+3,3,Color.White);
        RECT_P(G9,X+16,Y,X+55,Y+20,#333333:32h);
        
        if Colors.channel == 1 && n == 1 then
            RECT_P(G9,X+54,Y,X+55,Y+20,RGB(255,0,0));
        end;
        
        if Colors.channel == 2 && n == 2 then
            RECT_P(G9,X+54,Y,X+55,Y+20,RGB(0,255,0));
        end;
        
        if Colors.channel == 3 && n == 3 then
            RECT_P(G9,X+54,Y,X+55,Y+20,RGB(0,0,255));
        end;
        
        if Colors.unitType == UnitType.Decimal then
            TEXTOUT_P(rgb[n],G9,X+22,Y+5,2,Color.White);
        else
            TEXTOUT_P(IP(rgb[n] / 255 * 100)+"%",G9,X+20,Y+5,2,Color.White);
        end;
        Y += 24;
    end;
    
    /// HSV or HSL Info
    if Colors.colorSpace == ColorSpace.HSV then info = {"H:","S:","V:"}; end;
    if Colors.colorSpace == ColorSpace.HSL then info = {"H:","S:","L:"}; end;
    
    X += 58;
    Y = 51;
    
    for n from 1 to 3 step 1 do
        ts = TEXTSIZE(info[n],3);
        TEXTOUT_P(info[n],G9,X+15-ts[1],Y+3,3,Color.White);
        RECT_P(G9,X+16,Y,X+55,Y+20,#333333:32h);
        case
            if Colors.colorSpace == ColorSpace.HSV then
                if n == 1 then
                    TEXTOUT_P(IP(Colors.h)+"°",G9,X+20,Y+5,2,Color.White);
                end;
                if n == 2 then
                    TEXTOUT_P(IP(Colors.s)+"%",G9,X+20,Y+5,2,Color.White);
                end;
                if n == 3 then
                    TEXTOUT_P(IP(Colors.v)+"%",G9,X+20,Y+5,2,Color.White);
                end;
            end;
            if Colors.colorSpace == ColorSpace.HSL then
                if n == 1 then
                    TEXTOUT_P(IP(hsl[n])+"°",G9,X+20,Y+5,2,Color.White);
                else
                    TEXTOUT_P(IP(hsl[n])+"%",G9,X+20,Y+5,2,Color.White);
                end;
            end;
        end;
        Y += 24;
    end;
    
    /// CMYK Info
    info = {"C:","M:","Y:","K:"};
    X += 58;
    Y = 51;
    for n from 1 to 4 step 1 do
        ts = TEXTSIZE(info[n],3);
        TEXTOUT_P(info[n],G9,X+15-ts[1],Y+3,3,Color.White);
        RECT_P(G9,X+16,Y,X+55,Y+20,#333333:32h);
        TEXTOUT_P(IP(cmyk[n])+"%",G9,X+20,Y+5,2,Color.White);
        Y += 24;
    end;
    
    BLIT_P(__SCREEN, G9);
end;

Colors::UpdateRGB()
begin
    local rgb;
    rgb = HSVtoRGB(Colors.h, Colors.s, Colors.v);
    Colors.r = rgb[1];
    Colors.g = rgb[2];
    Colors.b = rgb[3];
end;

Colors::DoMenu()
begin
    local menu = (Y<220? 0: IP(X / (320/6)+0.025) + 1);
    case
        if menu == 1 then
            Colors.unitType = UnitType.Percentage;
        end;
            
        if menu == 2 then
            Colors.unitType = UnitType.Decimal;
        end;
            
        if menu == 3 then
            Colors.h = floor((Colors.h - 29) / 30) * 30;
            Colors::UpdateRGB;
            HP.MouseClr;
        end;
            
        if menu == 4 then
            Colors.h = floor(Colors.h / 30) * 30 + 30;
            Colors::UpdateRGB;
            HP.MouseClr;
        end;
            
        if menu == 5 then
            Colors.colorSpace = ColorSpace.HSV;
        end;
            
        if menu == 6 then
            Colors.colorSpace = ColorSpace.HSL;
        end;
    end;
end;

Colors::AdjRGB(delta)
begin
    case
        if Colors.channel == 1 then
            Colors.r = Colors.r + delta;
            Colors.r = IP(MIN(MAX(Colors.r, 0), 255));
        end;
        
        if Colors.channel == 2 then
            Colors.g = Colors.g + delta;
            Colors.g = IP(MIN(MAX(Colors.g, 0), 255));
        end;
        
        if Colors.channel == 3 then
            Colors.b = Colors.b + delta;
            Colors.b = IP(MIN(MAX(Colors.b, 0), 255));
        end;
    end;
    
    // Update HSV values.
    local hsv;
    hsv = RGBtoHSV(Colors.r, Colors.g, Colors.b);
    Colors.h = hsv[1];
    Colors.s = hsv[2];
    Colors.v = hsv[3];
end;

Colors::AdjHue(d:delta)
begin
    Colors.h = Colors.h + delta;
    Colors.h = IP(MIN(MAX(Colors.h, 0), 360));
    
    Colors::UpdateRGB;
end;

Colors::AdjSaturation(d:delta)
begin
    Colors.s = Colors.s + delta;
    Colors.s = IP(MIN(MAX(Colors.s, 0), 100));
    
    Colors::UpdateRGB;
end;

Colors::AdjBrightness(d:delta)
begin
    Colors.v = Colors.v + delta;
    Colors.v = IP(MIN(MAX(Colors.v, 0), 100));
    
    Colors::UpdateRGB;
end;

Colors::Loop()
begin
    local focus = "";

    while 1 do
        Colors::Update();

        local event = WAIT(-1);
        dict Event event;
        if hp::isKeyPressed(event) then
            focus = "";
            iferr
                if event.key == KeyCode.Esc then
                    return;
                end;
                
                if event.key == KeyCode.Enter then
                    return HSV(Colors.h, Colors.s, Colors.v);
                end;
                
                if event.key == KeyCode.Minus then
                    if Colors.channel == 0 then
                        Colors::AdjHue(-1);
                    else
                        Colors::AdjRGB(-1);
                    end;
                end;
                
                if event.key == KeyCode.Plus then
                    if Colors.channel == 0 then
                        Colors::AdjHue(1);
                    else
                        Colors::AdjRGB(1);
                    end;
                end;
                
                if event.key == KeyCode.Up then
                    if Colors.channel == 0 then
                        Colors::AdjBrightness(1);
                    else
                        Colors.channel += (Colors.channel == 1 ? 2 : -1);
                    end;
                end;
                
                if event.key == KeyCode.Down then
                    if Colors.channel == 0 then
                        Colors::AdjBrightness(-1);
                    else
                        Colors.channel += (Colors.channel == 3 ? -2 : 1);
                    end;
                end;
                
                if event.key == KeyCode.Left && Colors.channel == 0 then
                    Colors::AdjSaturation(-1);
                end;
                
                if event.key == KeyCode.Right && Colors.channel == 0 then
                    Colors::AdjSaturation(1);
                end;
            then
            end;
            
        end;
        
        if hp::isMouseEvent(event) then
            X = event.x;
            Y = event.y;
            
            Colors::DoMenu();
            
            case
                if event.type == EventType.MouseDown then
                    if X>=5 && X<315 && Y>=148 && Y<188 then
                        Colors.h := (X - 5) / 310 * 360;
                        Colors::UpdateRGB();
                        Colors.channel = 0;
                        focus = "Hue";
                    end;
                    
                    if X>=5 && X<143 && Y≥5 && Y<143 then
                        Colors.s := IP((X - 5) / 137 * 100);
                        Colors.v := IP((1 - (Y - 5) / 137) * 100);
                        Colors::UpdateRGB();
                        Colors.channel = 0;
                        focus = "Saturation & Brightness";
                    end;
                    
                    if X>=138 && X <=193 && Y>=51 && Y<=71 then
                        Colors.channel = 1;
                    end;
                    
                    if X>=138 && X <=193 && Y>=75 && Y<=95 then
                        Colors.channel = 2;
                    end;
                    
                    if X>=138 && X <=193 && Y>=99 && Y<=119 then
                        Colors.channel = 3;
                    end;
                end;
                
                if event.type == EventType.MouseMove then
                    if focus == "Hue" then
                        Colors.h = (X - 5) / 310 * 360;
                        Colors::UpdateRGB();
                    end;
                    
                    if focus == "Saturation & Brightness" then
                        Colors.s = IP((X - 5) / 137 * 100);
                        Colors.v = IP((1 - (Y - 5) / 137) * 100);
                        Colors::UpdateRGB();
                    end;
                end;
                
                if event.type == EventType.MouseUp then
                    focus = "";
                end;
            end;
            
            Colors.h = IP(MIN(MAX(Colors.h, 0), 360));
            Colors.s = IP(MIN(MAX(Colors.s, 0), 100));
            Colors.v = IP(MIN(MAX(Colors.v, 0), 100));
        end;
    end;
end;

export Colors()
begin
    Colors::Setup;
    Colors::Loop;
end;
