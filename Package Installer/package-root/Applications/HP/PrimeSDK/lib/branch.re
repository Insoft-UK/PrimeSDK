>`\bendif\b`i END
>`\bswitch +([a-zA-Z_][\w.:(),]*)`i LOCAL sw__SCOPE__ := $1; CASE
>`\bcase +(\-?\d+) +do *$`i IF sw\` __SCOPE__-1` == $1 THEN
>`\btry\b`i IFERR
