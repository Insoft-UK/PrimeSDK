LE

0x0000-0x0003:
- Header Size, excludes itself (so the header begins at offset 4)
0x0004-0x0005:
    Number of variables in table
0x0006-0x0007:
    Number of **uknown**?
0x0008-0x0009:
    Number of exported functions in table
0x000A-0x000F:
    unused?
Conn. kit generates
    7F 01 00 00 00 00
    but all zeros seems to work too.
0x0010-0x----:
Entry format is as follows:
    - Type of item:
        30 00 for variable,
        31 00 for exported function
    Name of item:
        UTF-16, until 00 00 00 00

CODE HEADER:
0x0000-0x0003:
    size of the header, excludes itself
0x0004-:
    Code in UTF-16 LE until 00 00


