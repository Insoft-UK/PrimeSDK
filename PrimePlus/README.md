# P+ for HP Prime

<br />
<img src="https://raw.githubusercontent.com/Insoft-UK/PrimeSDK/main/assets/P+_Logo.svg" style="width: 128px" />
P+ is a proprietary programming language designed to mitigate the challenges associated with code maintenance and comprehensibility in the HP Programming Language (PPL). P+ serves as an intermediary language that addresses these issues by introducing support for substitutions and facilitating code organization. The final output is a PPL program file with a minimal footprint, optimized to conserve the limited storage space available on the HP Prime.
<br/><br/>
P+ is essentially the PPL language with additional features, such as substitution capabilities, and requires all keywords to be in lowercase.
<br/><br/>

<b><a href="https://github.com/Insoft-UK/PrimeSDK/tree/main/GROB">GROB</a> to be intergrated in the future.</b>

```
#pragma mode( separator(.,;) integer(h64) )
#include <pplang>

auto:displayCopyright()
begin
  TEXTOUT_P("Copyright (c) 2023-2025 Insoft. All rights reserved.", 0, 0);
end;

#PYTHON
#END

EXPORT START()
BEGIN
  displayCopyright();
#PPL
  // In P+ `=` is treated as `:=` were in PPL `=` is treated as `==`
  // So only PPL code in this section.
#END
  WAIT;
  var a:alpha = 0;
  alpha += 10;
  RETURN a;
END;
```

>PPL and P+ code can co-exist as P+ is just an extension of PPL.

`p+ project.pp -l pplib`

```
#pragma mode( separator(.,;) integer(h64) )
fn1()
BEGIN
  TEXTOUT_P("Copyright (c) 2023-2025 Insoft. All rights reserved.", 0, 0);
END;

#PYTHON
#END
EXPORT START()
BEGIN
  fn1;
  // In P+ `=` is treated as `:=` were in PPL `=` is treated as `==`
  // So only PPL code in this section.
  WAIT;
  LOCAL a := 0;
  a := a + 10;
  RETURN a;
END;
```

## Regular Expressions
P+ v3.1 and lator added support for regular expressions.

**switch**
eg.
```
regex `\bswitch +([a-zA-Z_]\w*)` LOCAL sw__SCOPE__ := $1;CASE
regex `\bcase +(\-?\d+) +do *$` IF sw\`__SCOPE__-1` == $1 THEN
```

P+
```
switch X
    case 0 do
    end;
end;
```

PPL
```
LOCAL sw0 := X;
CASE
  IF sw0 == 0 THEN
  END;
END;
```

## Code Stack

A code stack provides a convenient way to store code snippets that can be retrieved and used later.
**P+**
```
__PUSH__`i := i + 1;`
local i := 8;
__POP__
```
**PPL**
```
LOCAL i := 8;
i := i + 1;
```

Intended for `regex`

## Substitution
`down ...` for `DOWNTO` included.
`…` for `...` supported, `...` and `…` can now be used instead of `to`

>[!IMPORTANT]
In P+ `=` is treated as `:=` were in PPL `=` is treated as `==`

>[!NOTE]
The P+ proprietary programming language is susceptible to change, while also maintaining some compatibility with previous versions.
