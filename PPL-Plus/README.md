# PPL+ for HP Prime

<br />
<img src="https://raw.githubusercontent.com/Insoft-UK/PrimeSDK/main/assets/PPL+_Logo.png" style="width: 128px" />
<b>PPL+</b> is a pre-processor utility designed to improve code maintainability and readability in the HP Programming Language (PPL). PPL+ also allows one to define regular expressions to assist in the preprocessing workflow. The final output is a compact, optimized PPL program file tailored to the HP Prime’s limited storage capacity.
<br/><br/>

<b><a href="https://github.com/Insoft-UK/PrimeSDK/tree/main/GROB">GROB</a> to be intergrated in the future.</b>

**PPL+**

```
#pragma mode( separator(.,;) integer(h64) )
auto: displayCopyright()
begin
  TEXTOUT_P("Copyright (c) 2023-2025 Insoft. All rights reserved.", 0, 0);
end;

#PYTHON
#END

EXPORT START()
BEGIN
  displayCopyright();
#PPL
  // In PPL+ `=` is treated as `:=` were in PPL `=` is treated as `==`
  // So only PPL code in this section.
  A := B;
#END
  WAIT;
  LOCAL a: alpha = 0;
  LOCAL _beta = 1;
  alpha = alpha + _beta;
  RETURN a;
END;
```


**PPL**

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
  // In PPL+ `=` is treated as `:=` were in PPL `=` is treated as `==`
  // So only PPL code in this section.
  A := B;
  WAIT;
  LOCAL a := 0;
  LOCAL v1 := 1;
  a := a + v1;
  RETURN a;
END;
```

## Regular Expressions

### switch


**PPL+**
```
regex `\bswitch +([a-zA-Z_]\w*)` LOCAL sw__SCOPE__ := $1;CASE
regex `\bcase +(\-?\d+) +do *$` IF sw\`__SCOPE__-1` == $1 THEN

switch X
    case 0 do
    end;
end;
```

**PPL**
```
LOCAL sw0 := X;
CASE
  IF sw0 == 0 THEN
  END;
END;
```

## Code Stack

A code stack provides a convenient way to store code snippets that can be retrieved and used later.

**PPL+**
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

## Assignment Style

In <b>PPL+</b>, the = operator is treated as := (assignment) by default, whereas in standard <b>PPL</b>, = is interpreted as == (equality). This behavior in PPL+ can be explicitly controlled using the directive:

```#pragma mode( assignment(:=) )```

This allows you to toggle between assignment-style and comparison-style interpretation of =.

>[!IMPORTANT]
In PPL+ by default `=` is treated as `:=` were in PPL `=` is treated as `==`

>[!NOTE]
The PPL+ pre-processor is susceptible to change, while also maintaining some compatibility with previous versions.
