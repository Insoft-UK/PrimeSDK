`\bVAR\b`i LOCAL
`\b(?:true|yes)\b`i 1
`\b(?:false|no)\b`i 0
`\bPYTHON +([a-z]\w*) *:?= *([a-z]\w*)\b`i LOCAL $1:="\""+STRING($2)+"\""
