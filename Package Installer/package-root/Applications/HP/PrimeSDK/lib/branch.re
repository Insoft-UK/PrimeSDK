`\bswitch +([a-zA-Z_][\w.:(),]*)` LOCAL sw__SCOPE__ := $1;CASE
`\bcase +(\-?\d+) +do *$` IF sw\` __SCOPE__-1` == $1 THEN
