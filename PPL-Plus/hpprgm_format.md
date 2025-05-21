# The .hpprgm format
>[!WARNING]
>This is an older version of the .hpprgm format, originally used by the HP Prime G1 (not the G2). It is accepted by the HP Prime Emulator on macOS, where it is automatically converted for compatibility. However, on Windows 11 (NT 10.1), which uses a more recent version of the emulator, compatibility with this older format has not yet been verified.

**0x0000-0x0003**: Header Size, excludes itself (so the header begins at offset 4)

**0x0004-0x0005**: Number of variables in table
  
**0x0006-0x0007**: Number of **uknown**?

**0x0008-0x0009**: Number of exported functions in table
 
**0x000A-0x000F**: Conn. kit generates **7F 01 00 00 00 00** but all zeros seems to work too.

**0x0010-0x----**:

- Entry format is as follows:
    - Type of item:
        **30 00** for variable,
        **31 00** for exported function
    - Name of item:
        UTF-16, until **00 00 00 00**


## VARIABLE VALUES
>[!NOTE]
>There are as many blocks of this type as you have exported variables and the blocks are in the same order as the exported variables

**0x0000-0x0003**: Size of the value, **excludes itself**

**0x0004-0x0005**:

- **01 00** for detecting that this is a list
- **02 00** for single value entry
  
  #### If single value entry

  **0x0006-0x0007**:

  - **type**:
    - **10 01** for base-10 integer or float
    - **11 20** for base-16 integer
    - **12 02** for string

  #### IF base-10 integer or float:


    **0x0008-0x000B**: Exponent (signed little endian 32-bit integer)
    
    **0x000C-0x0013**: Mantissa
  
    little endian weird stuff. Hexadecimal to be interpreted as decimal...
  
    **00 00 00 00 00 00 00 25 01** is supposed to be 1.25 in decimal,
    **00 00 00 00 00 00 00 28 06** -> 6.28 and so on
      
    The value is mantissa*10^exponent
  
  #### IF base-16 integer:
  
    **0x0008-0x000B**: **02 00 00 00** (why?)

    **0x000C-0x0013**: BCD **55 63 62 00 00 00 00 00** becomes #626355 **25 06 00 00 00 00 00 00** becomes #625
  
  #### IF string:
  
    **0x0008-0x0009**: Length of string in characters, excludes the tailing **00 00**

    **0x000A-**: string itself, ends in **00 00**

  
  #### IF list:

    **0x0006-0x0007**: ??? (Tends to be **16 00**, **16 01** or **16 02**)

    **0x0008-0x0009**: 32-bit LE Amount of members in list, let's call this N

    **0x000A-0x000B**: ??? (Probably reserved for something, **7F 01** or **00 00**)
 
    **N 4-byte values**: ??? Actual list of values follows, they are in reverse order compared to what they are in the source.
    
    **An entry in the list follows this formula**:
  
    Stuff gets clever and recursive, every entry in here is handled like a "VARIABLE VALUE" itself minus the size integer at the beginning


## CODE HEADER

**0x0000-0x0003**: Size of the **PPL Code** in UTF-16 LE

**0x0004-0x----**: Code in UTF-16 LE until **00 00**


