>`\bswitch +([a-zA-Z_][\w.:(),]*)`i LOCAL sw__SCOPE__ := $1;\nCASE
>`\bcase +(\-?\d+) +do *$`i IF sw\`__SCOPE__-1` == $1 THEN
