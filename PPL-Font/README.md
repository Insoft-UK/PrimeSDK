# Adafruit GFX Font Converter
This is a handy utility that goes well with the original [fontconvert](https://github.com/adafruit/Adafruit-GFX-Library/tree/master/fontconvert) tool provided by [Adafruit](https://www.adafruit.com/) for converting Adafruit_GFX `.h` to Adafruit_GFX `.hpprgm` format.

This utility tool provides the ability to generate an Adafruit_GFX `.hpprgm` format from an Adafruit_GFX `.h` file.

e.g.
### HD44780.h
<img src="https://github.com/Insoft-UK/PrimeSDK/blob/main/assets/HD44780.png" width="20%" >

```
pplfont HD44780.h
```

The HP Prime stores its glyph data as a list of 64-bit unsigned integers. The bitmap, however, is stored in a specific bit order (little-endian) and where each byte of the 64-bit value is mirror-flipped.

e.g.
<img src="https://github.com/Insoft-UK/PrimeSDK/blob/main/assets/Hart.png" width="128" >
```
01101100 #6Ch to 00110110 #36h
11111110 #FEh to 01111111 #7Fh
11111110 #FEh to 01111111 #7Fh
11111110 #FEh to 01111111 #7Fh
01111110 #7Eh to 01111110 #7Eh
00111000 #38h to 00011100 #1Ch
00010000 #10h to 00001000 #08h
00000000 #00h to 00000000 #00h := #00081C7E7F7F7F36:64h
```

### Little-Endian
`#00081C7E7F7F7F36:64h`
#### Glyphs
`#--YYXXAAHHWWOOOO:64h = #--:8h #00:8h #00:8h #00:8h #00:8h #00:8h #0000:16h`

> [!NOTE]
pplfont is a streamlined version of the piXfont tool. While both can perform the same core font-related tasks, piXfont offers additional features, including creating fonts from images, converting .hpprgm files back into .h files, and generating texture atlases.

