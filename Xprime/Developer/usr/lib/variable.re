`\bVAR\b`i LOCAL
`\b(?:TRUE|YES)\b`i 1
`\b(?:FALSE|NO)\b`i 0
`\bPYTHON +([a-z]\w*) *:?= *([a-z]\w*)\b`i LOCAL $1:="\""+STRING($2)+"\""
