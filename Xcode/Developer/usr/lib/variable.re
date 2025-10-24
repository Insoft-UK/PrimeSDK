`\bVAR\b`i LOCAL
`\b(?:TRUE|YES)\b`i 1
`\b(?:FALSE|NO)\b`i 0
`\bPYTHON +([a-z]\w*) *:?= *([a-z]\w*)\b`i LOCAL $1:="\""+STRING($2)+"\""
>`\bauto\b`i v__ADVANCE____COUNT__
`\b([a-zA-Z_]\w*) *\: *([a-zA-Z]\w*(?:::[a-zA-Z]\w*)*)` alias $2:=$1;$1
