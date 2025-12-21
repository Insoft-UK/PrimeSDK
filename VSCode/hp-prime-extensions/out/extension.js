const vscode = require('vscode');

function activate(context) {
    let disposable = vscode.languages.registerHoverProvider('hp-prime', {
        provideHover(document, position, _token) {
            const range = document.getWordRangeAtPosition(position);
            const word = document.getText(range);
            if (word === 'MAKEMAT') {
                const markdownString = new vscode.MarkdownString();
                markdownString.appendMarkdown('### Sintaxis\n\n');
                markdownString.appendMarkdown('```hp-prime\n');
                markdownString.appendMarkdown('MAKEMAT(expression, rows, columns)\n');
                markdownString.appendMarkdown('```\n\n');
                markdownString.appendMarkdown('---\n\n');
                markdownString.appendMarkdown('### Example of use\n\n');
                markdownString.appendMarkdown('```hp-prime\n');
                markdownString.appendMarkdown('MAKEMAT(0,3,3)\n');
                markdownString.appendMarkdown('```\n\n');
                markdownString.appendMarkdown('Returns a matrix of zeros 3 × 3 → [[0,0,0],[0,0,0],[0,0,0]]\n\n');
                markdownString.appendMarkdown('---\n\n');
                markdownString.appendMarkdown('### MAKEMAT\n\n');
                markdownString.appendMarkdown('Create a matrix with rows x columns dimensions, using expressions to calculate each element. If the expression contains the variables I and J, the calculation of each element substitutes the current row number for I and the current column number for J. You can also create a vector by specifying the number of elements (e) instead of the number of rows and columns.\n\n');
                return new vscode.Hover(markdownString);
            }
            if (word === 'MAKELIST') {
                const markdownString = new vscode.MarkdownString();
                markdownString.appendMarkdown('### Sintaxis\n\n');
                markdownString.appendMarkdown('```hp-prime\n');
                markdownString.appendMarkdown('MAKELIST(expression, variable, start, end, [increment])\n');
                markdownString.appendMarkdown('```\n\n');
                markdownString.appendMarkdown('---\n\n');
                markdownString.appendMarkdown('### Example of use\n\n');
                markdownString.appendMarkdown('```hp-prime\n');
                markdownString.appendMarkdown('MAKELIST(2*X-1, X, 1, 5, 1) returns {1, 3, 5, 7, 9}\n');
                markdownString.appendMarkdown('```\n\n');
                markdownString.appendMarkdown('---\n\n');
                markdownString.appendMarkdown('### MAKELIST\n\n');
                markdownString.appendMarkdown('Make list. Calculates a sequence of elements for a new list. Evaluates the expression, incrementing the variable from the start to the end of the values, using increment steps (default is 1). The function MAKELIST generates a series by automatically producing a list from the repeated evaluation of an expression.\n\n');
                return new vscode.Hover(markdownString);
            }
            if (word === 'BEGIN') {
                const markdownString = new vscode.MarkdownString();
                markdownString.appendMarkdown('```hp-prime\n');
                markdownString.appendMarkdown('BEGIN command1; command2; ...; commandN; END;\n');
                markdownString.appendMarkdown('```\n\n');
                markdownString.appendMarkdown('---\n\n');
                markdownString.appendMarkdown('Define a command or a set of commands that must be executed together in a simple program.\n\n');
                return new vscode.Hover(markdownString);
            }
            if (word === 'RETURN') {
                const markdownString = new vscode.MarkdownString();
                markdownString.appendMarkdown('```hp-prime\n');
                markdownString.appendMarkdown('RETURN expression;\n');
                markdownString.appendMarkdown('```\n\n');
                markdownString.appendMarkdown('---\n\n');
                markdownString.appendMarkdown('Returns the current value of the expression\n\n');
                return new vscode.Hover(markdownString);
            }
            if (word === 'KILL') {
                const markdownString = new vscode.MarkdownString();
                markdownString.appendMarkdown('```hp-prime\n');
                markdownString.appendMarkdown('KILL;\n');
                markdownString.appendMarkdown('```\n\n');
                markdownString.appendMarkdown('---\n\n');
                markdownString.appendMarkdown('Stops the execution of the program\n\n');
                return new vscode.Hover(markdownString);
            }
            if (word === 'IF') {
                const markdownString = new vscode.MarkdownString();
                markdownString.appendMarkdown('```hp-prime\n');
                markdownString.appendMarkdown('IF test THEN commands 1 ELSE commands 2 END;\n');
                markdownString.appendMarkdown('```\n\n');
                markdownString.appendMarkdown('---\n\n');
                markdownString.appendMarkdown('Evaluates test. If test is true (not equal to 0), executes commands 1; otherwise, executes commands 2.\n\n');
                return new vscode.Hover(markdownString);
            }
            if (word === 'CASE') {
                const markdownString = new vscode.MarkdownString();
                markdownString.appendMarkdown('### Syntax\n\n');
                markdownString.appendMarkdown('```hp-prime\n');
                markdownString.appendMarkdown('CASE\n');
                markdownString.appendMarkdown(' IF test1 THEN commands1 END;\n');
                markdownString.appendMarkdown(' IF test1 THEN commands1 END;\n');
                markdownString.appendMarkdown(' ⁝\n');
                markdownString.appendMarkdown(' IF testN THEN commandsN END;\n');
                markdownString.appendMarkdown('END;\n');
                markdownString.appendMarkdown('```\n\n');
                markdownString.appendMarkdown('---\n\n');
                markdownString.appendMarkdown('### CASE\n\n');
                markdownString.appendMarkdown('Evaluates test1. If it is true, executes commands1 and closes the CASE. Otherwise, evaluates test1. If it is true, executes commands2 and closes the CASE. Continues evaluating tests until it finds a true one. If no true test is found, executes the default commands, if provided. The CASE command is limited to 127 branches.\n\n');
                return new vscode.Hover(markdownString);
            }
            if (word === 'IFERR') {
                const markdownString = new vscode.MarkdownString();
                markdownString.appendMarkdown('```hp-prime\n');
                markdownString.appendMarkdown('•  IFERR commands1 THEN commands2 END;\n');
                markdownString.appendMarkdown('•  IFERR commands1 THEN commands2 ELSE commands3 END;\n');
                markdownString.appendMarkdown('```\n\n');
                markdownString.appendMarkdown('---\n\n');
                markdownString.appendMarkdown('Executes the sequence of commands1. If an error occurs during the execution of commands1, it executes the sequence of commands2. Otherwise, it executes the sequence of commands3.\n\n');
                return new vscode.Hover(markdownString);
            }
            
            if (word === 'FOR') {
                const markdownString = new vscode.MarkdownString();
                markdownString.appendMarkdown('```hp-prime\n');
                markdownString.appendMarkdown('FOR var FROM start TO finish DO commands END;\n');
                markdownString.appendMarkdown('```\n\n');
                markdownString.appendMarkdown('---\n\n');
                markdownString.appendMarkdown('Defines the variable var with the value start, and as long as the value of this variable is less than or equal to finish, it executes the sequence of commands, and then adds 1 (increment) to var.\n\n');
                return new vscode.Hover(markdownString);
            }
            
            if (word === 'STEP') {
                const markdownString = new vscode.MarkdownString();
                markdownString.appendMarkdown('```hp-prime\n');
                markdownString.appendMarkdown('FOR var FROM start TO (or DOWNTO) finish [STEP increment] DO command(s) END;\n');
                markdownString.appendMarkdown('```\n\n');
                markdownString.appendMarkdown('---\n\n');
                markdownString.appendMarkdown('Sets the variable var to start; then, while the value of this variable is less than or equal to (or greater than for DOWNTO) finish, it executes the commands and adds (or subtracts DOWNTO) 1 (or increments) to var.\n\n');
                return new vscode.Hover(markdownString);
            }
            
            if (word === 'DOWNTO') {
                const markdownString = new vscode.MarkdownString();
                markdownString.appendMarkdown('```hp-prime\n');
                markdownString.appendMarkdown('FOR var FROM start DOWNTO finish DO commands END;\n');
                markdownString.appendMarkdown('```\n\n');
                markdownString.appendMarkdown('---\n\n');
                markdownString.appendMarkdown('Defines the variable var with the value start, and as long as the value of this variable is greater than or equal to finish, it executes the sequence of commands, and then subtracts 1 (decrement) from var.\n\n');
                return new vscode.Hover(markdownString);
            }
            if (word === 'LOCAL') {
                const markdownString = new vscode.MarkdownString();
                markdownString.appendMarkdown('```hp-prime\n');
                markdownString.appendMarkdown('•  LOCAL var1,var2,…var8;\n');
                markdownString.appendMarkdown('•  LOCAL value1:=10, value2:={};\n');
                markdownString.appendMarkdown('```\n\n');
                markdownString.appendMarkdown('---\n\n');
                markdownString.appendMarkdown('Makes variables var1, var2, etc., local to the program in which they are found.\n\n');
                markdownString.appendMarkdown('The maximum number that a LOCAL can store is 8 variables; to create more variables, you will have to declare another LOCAL with its variables.\n\n');
                return new vscode.Hover(markdownString);
            }
            if (word === 'EXPORT') {
                const markdownString = new vscode.MarkdownString();
                markdownString.appendMarkdown('### Syntax\n\n');
                markdownString.appendMarkdown('```hp-prime\n');
                markdownString.appendMarkdown('•  EXPORT FunctionName()\n');
                markdownString.appendMarkdown('•  EXPORT FunctionName(Parameters)\n');
                markdownString.appendMarkdown('```\n\n');
                markdownString.appendMarkdown('---\n\n');
                markdownString.appendMarkdown('### Example of use\n\n');
                markdownString.appendMarkdown('```hp-prime\n');
                markdownString.appendMarkdown('EXPORT FunctionName()\n');
                markdownString.appendMarkdown('BEGIN\n');
                markdownString.appendMarkdown(' PRINT("Hello world");\n');
                markdownString.appendMarkdown('END;\n');
                markdownString.appendMarkdown('```\n\n');
                markdownString.appendMarkdown('---\n\n');
                markdownString.appendMarkdown('### EXPORT\n\n');
                markdownString.appendMarkdown('In a program, declares a list of exported variables or an exported function.\n\n');
                return new vscode.Hover(markdownString);
            }
            
            if (word === 'VIEW') {
                const markdownString = new vscode.MarkdownString();
                markdownString.appendMarkdown('```hp-prime\n');
                markdownString.appendMarkdown('VIEW “text”, FunctionName();\n');
                markdownString.appendMarkdown('```\n\n');
                markdownString.appendMarkdown('---\n\n');
                markdownString.appendMarkdown('Allows a programmer to customize the Views menu. It makes "text" appear when the VIEW key is pressed and executes the function when the menu key `OK` (or the `ENTER` key) is pressed.\n\n');
                return new vscode.Hover(markdownString);
            }
            
            if (word === 'ASC') {
                const markdownString = new vscode.MarkdownString();
                markdownString.appendMarkdown('```hp-prime\n');
                markdownString.appendMarkdown('ASC(string)\n\n');
                markdownString.appendMarkdown('```\n\n');
                markdownString.appendMarkdown('---\n\n'); // First separator
                markdownString.appendMarkdown('`Example:`\n\n');
                markdownString.appendMarkdown('```hp-prime\n');
                markdownString.appendMarkdown('ASC("AB") returns [65,66]\n');
                markdownString.appendMarkdown('```\n\n');
                markdownString.appendMarkdown('---\n\n'); // Second separator
                markdownString.appendMarkdown('`Description:`\n\n');
                markdownString.appendMarkdown('The function `ASC(string)` returns a list containing the ASCII codes of each character in the provided string.\n\n');
                return new vscode.Hover(markdownString);
            }
            if (word === 'LOWER') {
                const markdownString = new vscode.MarkdownString();
                markdownString.appendMarkdown('```hp-prime\n');
                markdownString.appendMarkdown('LOWER(string)\n\n');
                markdownString.appendMarkdown('```\n\n');
                markdownString.appendMarkdown('---\n\n'); // First separator
                markdownString.appendMarkdown('`Example:`\n\n');
                markdownString.appendMarkdown('```hp-prime\n');
                markdownString.appendMarkdown('•  LOWER("ABC") returns "abc"\n');
                markdownString.appendMarkdown('•  LOWER("ΑΒΓ") returns "αβγ"\n');
                markdownString.appendMarkdown('```\n\n');
                markdownString.appendMarkdown('---\n\n'); // Second separator
                markdownString.appendMarkdown('`Description:`\n\n');
                markdownString.appendMarkdown('Converts uppercase characters in a string to lowercase.\n\n');
                return new vscode.Hover(markdownString);
            }
            if (word === 'UPPER') {
                const markdownString = new vscode.MarkdownString();
                markdownString.appendMarkdown('```hp-prime\n');
                markdownString.appendMarkdown('UPPER(string)\n\n');
                markdownString.appendMarkdown('```\n\n');
                markdownString.appendMarkdown('---\n\n'); // First separator
                markdownString.appendMarkdown('`Example:`\n\n');
                markdownString.appendMarkdown('```hp-prime\n');
                markdownString.appendMarkdown('•  UPPER("abc") returns "ABC"\n');
                markdownString.appendMarkdown('•  UPPER("αβγ") returns "ΑΒΓ"\n');
                markdownString.appendMarkdown('```\n\n');
                markdownString.appendMarkdown('---\n\n'); // Second separator
                markdownString.appendMarkdown('`Description:`\n\n');
                markdownString.appendMarkdown('Converts lowercase characters in a string to uppercase.\n\n');
                return new vscode.Hover(markdownString);
            }
            if (word === 'CHAR') {
                const markdownString = new vscode.MarkdownString();
                markdownString.appendMarkdown('```hp-prime\n');
                markdownString.appendMarkdown('CHAR(vector) or CHAR(integer)\n\n');
                markdownString.appendMarkdown('```\n\n');
                markdownString.appendMarkdown('---\n\n'); // First separator
                markdownString.appendMarkdown('`Example:`\n\n');
                markdownString.appendMarkdown('```hp-prime\n');
                markdownString.appendMarkdown('•  CHAR(65) returns "A"\n');
                markdownString.appendMarkdown('•  CHAR({82,77,72}) returns "RMH"\n');
                markdownString.appendMarkdown('```\n\n');
                markdownString.appendMarkdown('---\n\n'); // Second separator
                markdownString.appendMarkdown('`Description:`\n\n');
                markdownString.appendMarkdown('Returns the string corresponding to the character codes in vector or the unique code of the integer.\n\n');
                return new vscode.Hover(markdownString);
            }
            if (word === 'DIM') {
                const markdownString = new vscode.MarkdownString();
                markdownString.appendMarkdown('```hp-prime\n');
                markdownString.appendMarkdown('DIM(string)\n\n');
                markdownString.appendMarkdown('```\n\n');
                markdownString.appendMarkdown('---\n\n'); // First separator
                markdownString.appendMarkdown('`Example:`\n\n');
                markdownString.appendMarkdown('```hp-prime\n');
                markdownString.appendMarkdown('DIM("12345") returns 5, DIM("""") returns 1. \n');
                markdownString.appendMarkdown('(Note the use of two double quotes and the escape sequence).\n');
                markdownString.appendMarkdown('```\n\n');
                markdownString.appendMarkdown('---\n\n'); // Second separator
                markdownString.appendMarkdown('`Description:`\n\n');
                markdownString.appendMarkdown('Returns the number of characters in string.\n\n');
                return new vscode.Hover(markdownString);
            }
            if (word === 'STRING') {
                const markdownString = new vscode.MarkdownString();
                markdownString.appendMarkdown('```hp-prime\n');
                markdownString.appendMarkdown('STRING(expression)\n');
                markdownString.appendMarkdown('```\n\n');
                markdownString.appendMarkdown('---\n\n');
                markdownString.appendMarkdown('Evaluates the expression and returns the result as a string.\n\n');
                return new vscode.Hover(markdownString);
            }
            if (word === 'INSTRING') {
                const markdownString = new vscode.MarkdownString();
                markdownString.appendMarkdown('```hp-prime\n');
                markdownString.appendMarkdown('INSTRING(str1,str2)\n\n');
                markdownString.appendMarkdown('```\n\n');
                markdownString.appendMarkdown('---\n\n'); // First separator
                markdownString.appendMarkdown('`Example:`\n\n');
                markdownString.appendMarkdown('```hp-prime\n');
                markdownString.appendMarkdown('•  INSTRING("vanilla", "van") returns 1\n');
                markdownString.appendMarkdown('•  INSTRING("banana", "na") returns 3\n');
                markdownString.appendMarkdown('•  INSTRING("ab", "abc") returns 0\n');
                markdownString.appendMarkdown('```\n\n');
                markdownString.appendMarkdown('---\n\n'); // Second separator
                markdownString.appendMarkdown('`Description:`\n\n');
                markdownString.appendMarkdown('Returns the index of the first occurrence of str2 in str1. Returns 0 if str2 is not present in str1. Note that the first character in a string is at position 1.\n\n');
                return new vscode.Hover(markdownString);
            }
            if (word === 'LEFT') {
                const markdownString = new vscode.MarkdownString();
                markdownString.appendMarkdown('```hp-prime\n');
                markdownString.appendMarkdown('LEFT(str,n)\n\n');
                markdownString.appendMarkdown('```\n\n');
                markdownString.appendMarkdown('---\n\n'); // First separator
                markdownString.appendMarkdown('`Example:`\n\n');
                markdownString.appendMarkdown('```hp-prime\n');
                markdownString.appendMarkdown('LEFT("MOMOGUMBO",3) returns "MOM"\n');
                markdownString.appendMarkdown('```\n\n');
                markdownString.appendMarkdown('---\n\n'); // Second separator
                markdownString.appendMarkdown('`Description:`\n\n');
                markdownString.appendMarkdown('Returns the first n characters of the string str. If n ≥ DIM(str) or n < 0, returns str. If n == 0 returns the empty string.\n\n');
                return new vscode.Hover(markdownString);
            }
            if (word === 'RIGHT') {
                const markdownString = new vscode.MarkdownString();
                markdownString.appendMarkdown('```hp-prime\n');
                markdownString.appendMarkdown('RIGHT(str,n)\n\n');
                markdownString.appendMarkdown('```\n\n');
                markdownString.appendMarkdown('---\n\n'); // First separator
                markdownString.appendMarkdown('`Example:`\n\n');
                markdownString.appendMarkdown('```hp-prime\n');
                markdownString.appendMarkdown('RIGHT("MOMOGUMBO",5) returns "GUMBO"\n');
                markdownString.appendMarkdown('```\n\n');
                markdownString.appendMarkdown('---\n\n'); // Second separator
                markdownString.appendMarkdown('`Description:`\n\n');
                markdownString.appendMarkdown('Returns the last n characters of the string str. If n <= 0, returns an empty string. If n > DIM(str), returns str.\n\n');
                return new vscode.Hover(markdownString);
            }
            if (word === 'MID') {
                const markdownString = new vscode.MarkdownString();
                markdownString.appendMarkdown('```hp-prime\n');
                markdownString.appendMarkdown('MID(str,pos, [n])\n\n');
                markdownString.appendMarkdown('```\n\n');
                markdownString.appendMarkdown('---\n\n'); // First separator
                markdownString.appendMarkdown('`Example:`\n\n');
                markdownString.appendMarkdown('```hp-prime\n');
                markdownString.appendMarkdown('MID("MOMOGUMBO",3,5) returns "MOGUM", MID("PUDGE",4) returns "GE"\n');
                markdownString.appendMarkdown('```\n\n');
                markdownString.appendMarkdown('---\n\n'); // Second separator
                markdownString.appendMarkdown('`Description:`\n\n');
                markdownString.appendMarkdown('Extracts n characters from the string str starting at index pos. n is optional and if not specified, extracts all remaining characters from the string.\n\n');
                return new vscode.Hover(markdownString);
            }
            if (word === 'ROTATE') {
                const markdownString = new vscode.MarkdownString();
                markdownString.appendMarkdown('```hp-prime\n');
                markdownString.appendMarkdown('ROTATE(str,n)\n\n');
                markdownString.appendMarkdown('```\n\n');
                markdownString.appendMarkdown('---\n\n'); // First separator
                markdownString.appendMarkdown('`Example:`\n\n');
                markdownString.appendMarkdown('```hp-prime\n');
                markdownString.appendMarkdown('•  ROTATE("12345",2) returns "34512"\n');
                markdownString.appendMarkdown('•  ROTATE("12345",-1) returns "51234"\n');
                markdownString.appendMarkdown('•  ROTATE("12345",6) returns "12345"\n');
                markdownString.appendMarkdown('```\n\n');
                markdownString.appendMarkdown('---\n\n'); // Second separator
                markdownString.appendMarkdown('`Description:`\n\n');
                markdownString.appendMarkdown('Permutation of characters in the string str. If 0 <= n < DIM(str), it moves n places to the left. If -DIM(str) < n <= -1, it moves n places to the right. If n > DIM(str) or n < -DIM(str), it returns str.\n\n');
                return new vscode.Hover(markdownString);
            }
            if (word === 'STRINGFROMID') {
                const markdownString = new vscode.MarkdownString();
                markdownString.appendMarkdown('```hp-prime\n');
                markdownString.appendMarkdown('STRINGFROMID(integer)\n\n');
                markdownString.appendMarkdown('```\n\n');
                markdownString.appendMarkdown('---\n\n');
                markdownString.appendMarkdown('`Example:`\n\n');
                markdownString.appendMarkdown('```hp-prime\n');
                markdownString.appendMarkdown('•  STRINGFROMID(56) returns “Complex”\n');
                markdownString.appendMarkdown('•  STRINGFROMID(202) returns “Real”\n');
                markdownString.appendMarkdown('```\n\n');
                markdownString.appendMarkdown('---\n\n');
                markdownString.appendMarkdown('`Description:`\n\n');
                markdownString.appendMarkdown('Returns, in the current language, the integrated string associated in the internal string table with the specified integer.\n\n');
                return new vscode.Hover(markdownString);
            }
            if (word === 'REPLACE') {
                const markdownString = new vscode.MarkdownString();
                markdownString.appendMarkdown('```hp-prime\n');
                markdownString.appendMarkdown('REPLACE(object1, begin, object2)\n\n');
                markdownString.appendMarkdown('```\n\n');
                markdownString.appendMarkdown('---\n\n');
                markdownString.appendMarkdown('`Example:`\n\n');
                markdownString.appendMarkdown('```hp-prime\n');
                markdownString.appendMarkdown('REPLACE("12345","3","99") returns "12995"\n');
                markdownString.appendMarkdown('```\n\n');
                markdownString.appendMarkdown('---\n\n');
                markdownString.appendMarkdown('`Description:`\n\n');
                markdownString.appendMarkdown('Replaces part of object1 with object2 starting from the beginning. Objects can be arrays, vectors, or strings.\n\n');
                return new vscode.Hover(markdownString);
            }
            if (word === 'C→PX') {
                const markdownString = new vscode.MarkdownString();
                markdownString.appendMarkdown('```hp-prime\n');
                markdownString.appendMarkdown('C→PX(x,y) or C→PX({x,y})\n\n');
                markdownString.appendMarkdown('```\n\n');
                markdownString.appendMarkdown('---\n\n');
                markdownString.appendMarkdown('`Description:`\n\n');
                markdownString.appendMarkdown('Converts Cartesian coordinates into screen coordinates.\n\n');
                return new vscode.Hover(markdownString);
            }
            if (word === 'DRAWMENU') {
                const markdownString = new vscode.MarkdownString();
                markdownString.appendMarkdown('```hp-prime\n');
                markdownString.appendMarkdown('DRAWMENU({string1, string2, …, string6})\n\n');
                markdownString.appendMarkdown('```\n\n');
                markdownString.appendMarkdown('---\n\n');
                markdownString.appendMarkdown('`Example:`\n\n');
                markdownString.appendMarkdown('```hp-prime\n');
                markdownString.appendMarkdown('DRAWMENU ({"ABC", "", "DEF"}) creates a menu with the first and third buttons labeled ABC and DEF respectively. The other four menu keys are blank.\n');
                markdownString.appendMarkdown('```\n\n');
                markdownString.appendMarkdown('---\n\n');
                markdownString.appendMarkdown('`Description:`\n\n');
                markdownString.appendMarkdown('Draws a menu of six buttons at the bottom of the screen, with labels string1, string2,..., string6.\n\n');
                return new vscode.Hover(markdownString);
            }
            if (word === 'RGB') {
                const markdownString = new vscode.MarkdownString();
                markdownString.appendMarkdown('```hp-prime\n');
                markdownString.appendMarkdown('RGB(R, G, B, [A])\n\n');
                markdownString.appendMarkdown('```\n\n');
                markdownString.appendMarkdown('---\n\n');
                markdownString.appendMarkdown('`Example:`\n\n');
                markdownString.appendMarkdown('```hp-prime\n');
                markdownString.appendMarkdown('•  RGB(255,0,128) returns 16711808.\n');
                markdownString.appendMarkdown('•  RECT(RGB(0,0,255)) produces a blue screen\n');
                markdownString.appendMarkdown('•  LINE(0,0,8,8,RGB(0,255,0)) draws a green line\n');
                markdownString.appendMarkdown('```\n\n');
                markdownString.appendMarkdown('---\n\n');
                markdownString.appendMarkdown('`Description:`\n\n');
                markdownString.appendMarkdown('Returns an integer that can be used as the color parameter for a drawing function, based on the values of the red, green, and blue components (each from 0 to 255). If alpha is greater than 128, it returns the color marked as transparent. HP Prime does not support alpha channel blending.\n\n');
                return new vscode.Hover(markdownString);
            }
            if (word === 'ARC') {
                const markdownString = new vscode.MarkdownString();
                markdownString.appendMarkdown('```hp-prime\n');
                markdownString.appendMarkdown('ARC(G, x, y, r [ , a1, a2, c])\n\n');
                markdownString.appendMarkdown('```\n\n');
                markdownString.appendMarkdown('---\n\n');
                markdownString.appendMarkdown('`Example:`\n\n');
                markdownString.appendMarkdown('```hp-prime\n');
                markdownString.appendMarkdown('ARC(0,0,60,0,π,RGB(255,0,0)) draws a red semicircle centered at (0,0), using the current window settings from Config. of graph, and a radius of 60 pixels. The semicircle is drawn counterclockwise, from 0 to π.\n');
                markdownString.appendMarkdown('```\n\n');
                markdownString.appendMarkdown('---\n\n');
                markdownString.appendMarkdown('`Description:`\n\n');
                markdownString.appendMarkdown('Draws a circle or an arc on G, centered at point x, y, with radius r and color c starting at angle a1 and ending at angle a2.\n\n');
                markdownString.appendMarkdown('G can be any of the graphics variables and is optional. The default value is G0 and r is given in pixels.\n\n');
                markdownString.appendMarkdown('c is optional, and if not specified, black is used. It should be specified this way: #RRGGBB (in the same way a color is specified in HTML).\n\n');
                markdownString.appendMarkdown('a1 and a2 follow the current angle mode and are optional. The default is a complete circle.\n\n');
                return new vscode.Hover(markdownString);
            }
            if (word === 'DIMGROB') {
                const markdownString = new vscode.MarkdownString();
                markdownString.appendMarkdown('```hp-prime\n');
                markdownString.appendMarkdown('DIMGROB(G, w, h, [color]) or DIMGROB(G, w, h, list)\n\n');
                markdownString.appendMarkdown('```\n\n');
                markdownString.appendMarkdown('---\n\n');
                markdownString.appendMarkdown('`Descripción:`\n\n');
                markdownString.appendMarkdown('Establece las dimensiones de GROB G en w*h. Inicializa la gráfica G con un color o con los datos gráficos proporcionados en lista.\n\n');
                markdownString.appendMarkdown('Si el gráfico se inicializa utilizando datos gráficos, la lista es una lista de enteros. Cada entero, como se ve en base 16, describe un color cada 16 bits, en formato A1R5G5B5 (1 bit para el canal alfa y 5 bits para R, G y B).\n\n');
                return new vscode.Hover(markdownString);
            }
            if (word === 'FILLPOLY_P') {
                const markdownString = new vscode.MarkdownString();
                markdownString.appendMarkdown('```hp-prime\n');
                markdownString.appendMarkdown('FILLPOLY_P([G],{(x1, y1), (x2, y2),…(xn, yn)}, Color, [alpha])\n\n');
                markdownString.appendMarkdown('```\n\n');
                markdownString.appendMarkdown('---\n\n');
                markdownString.appendMarkdown('`Example:`\n\n');
                markdownString.appendMarkdown('```hp-prime\n');
                markdownString.appendMarkdown('FILLPOLY_P(G1,{(20,20), (100, 20), (100, 100), (20, 100)}, {#FF0000, 128}) draws a\n');
                markdownString.appendMarkdown('square, 80 pixels on a side, near the top-left corner of the screen, using the color\n');
                markdownString.appendMarkdown('purple and the transparency level 128.\n');
                markdownString.appendMarkdown('```\n\n');
                markdownString.appendMarkdown('---\n\n');
                markdownString.appendMarkdown('`Description:`\n\n');
                markdownString.appendMarkdown('For the polygon defined by the list of points, fills the polygon with the color defined by the color\n\n');
                markdownString.appendMarkdown('RGB number. If alpha is provided as an integer between 0 and 255 inclusive, the polygon is drawn with the\n\n');
                markdownString.appendMarkdown('corresponding level of transparency. You can use a vector of points instead of a list; in this case,\n\n');
                markdownString.appendMarkdown('the points can be expressed as complex numbers.\n\n');
                return new vscode.Hover(markdownString);
            }
            
            if (word === 'GETPIX') {
                const markdownString = new vscode.MarkdownString();
                markdownString.appendMarkdown('```hp-prime\n');
                markdownString.appendMarkdown('GETPIX([G], x, y)\n\n');
                markdownString.appendMarkdown('```\n\n');
                markdownString.appendMarkdown('---\n\n');
                markdownString.appendMarkdown('`Example:`\n\n');
                markdownString.appendMarkdown('```hp-prime\n');
                markdownString.appendMarkdown('GETPIX(G1, 150, 60)\n');
                markdownString.appendMarkdown('```\n\n');
                markdownString.appendMarkdown('---\n\n');
                markdownString.appendMarkdown('`Description:`\n\n');
                markdownString.appendMarkdown('Returns the color of the pixel G with coordinates x,y.\n\n');
                markdownString.appendMarkdown('G can be any of the graphic variables and is optional. The default value is G0, the current image.\n\n');
                return new vscode.Hover(markdownString);
            }
            
            if (word === 'GETPIX_P') {
                const markdownString = new vscode.MarkdownString();
                markdownString.appendMarkdown('```hp-prime\n');
                markdownString.appendMarkdown('GETPIX_P([G], x, y)\n\n');
                markdownString.appendMarkdown('```\n\n');
                markdownString.appendMarkdown('---\n\n');
                markdownString.appendMarkdown('`Example:`\n\n');
                markdownString.appendMarkdown('```hp-prime\n');
                markdownString.appendMarkdown('GETPIX_P(G1, 150, 60)\n');
                markdownString.appendMarkdown('```\n\n');
                markdownString.appendMarkdown('---\n\n');
                markdownString.appendMarkdown('`Description:`\n\n');
                markdownString.appendMarkdown('Returns the color of the pixel G with coordinates x,y.\n\n');
                markdownString.appendMarkdown('G can be any of the graphic variables and is optional. The default value is G0, the current image.\n\n');
                return new vscode.Hover(markdownString);
            }
            
            if (word === 'GROBH') {
                const markdownString = new vscode.MarkdownString();
                markdownString.appendMarkdown('```hp-prime\n');
                markdownString.appendMarkdown('GROBH(G)\n\n');
                markdownString.appendMarkdown('```\n\n');
                markdownString.appendMarkdown('---\n\n');
                markdownString.appendMarkdown('`Description:`\n\n');
                markdownString.appendMarkdown('G can be any of the graphic variables and is optional. The default value is G0.\n\n');
                return new vscode.Hover(markdownString);
            }
            
            if (word === 'GROBH_P') {
                const markdownString = new vscode.MarkdownString();
                markdownString.appendMarkdown('```hp-prime\n');
                markdownString.appendMarkdown('GROBH_P(G)\n\n');
                markdownString.appendMarkdown('```\n\n');
                markdownString.appendMarkdown('---\n\n');
                markdownString.appendMarkdown('`Description:`\n\n');
                markdownString.appendMarkdown('G can be any of the graphic variables and is optional. The default value is G0.\n\n');
                return new vscode.Hover(markdownString);
            }
            
            if (word === 'GROBW') {
                const markdownString = new vscode.MarkdownString();
                markdownString.appendMarkdown('```hp-prime\n');
                markdownString.appendMarkdown('GROBW(G)\n\n');
                markdownString.appendMarkdown('```\n\n');
                markdownString.appendMarkdown('---\n\n');
                markdownString.appendMarkdown('`Description:`\n\n');
                markdownString.appendMarkdown('G can be any of the graphic variables and is optional. The default value is G0.\n\n');
                return new vscode.Hover(markdownString);
            }
            
            if (word === 'GROBW_P') {
                const markdownString = new vscode.MarkdownString();
                markdownString.appendMarkdown('```hp-prime\n');
                markdownString.appendMarkdown('GROBW_P(G)\n\n');
                markdownString.appendMarkdown('```\n\n');
                markdownString.appendMarkdown('---\n\n');
                markdownString.appendMarkdown('`Description:`\n\n');
                markdownString.appendMarkdown('G can be any of the graphic variables and is optional. The default value is G0.\n\n');
                return new vscode.Hover(markdownString);
            }
            
            if (word === 'INVERT') {
                const markdownString = new vscode.MarkdownString();
                markdownString.appendMarkdown('```hp-prime\n');
                markdownString.appendMarkdown('INVERT([G, x1, y1, x2, y2])\n\n');
                markdownString.appendMarkdown('```\n\n');
                markdownString.appendMarkdown('---\n\n');
                markdownString.appendMarkdown('`Description:`\n\n');
                markdownString.appendMarkdown('Executes a reverse video of the selected area. G can be any of the graphic variables and is optional. The default value is G0.\n\n');
                markdownString.appendMarkdown('x2, y2 are optional, and if not specified, they will be the bottom right of the graphic.\n\n');
                markdownString.appendMarkdown('x1, y1 are optional, and if not specified, they will be the top left of the graphic. If only one pair x,y is specified, it refers to the top left.\n\n');
                return new vscode.Hover(markdownString);
            }
            
            if (word === 'INVERT_P') {
                const markdownString = new vscode.MarkdownString();
                markdownString.appendMarkdown('```hp-prime\n');
                markdownString.appendMarkdown('INVERT_P([G, x1, y1, x2, y2])\n\n');
                markdownString.appendMarkdown('```\n\n');
                markdownString.appendMarkdown('---\n\n');
                markdownString.appendMarkdown('`Description:`\n\n');
                markdownString.appendMarkdown('Executes a reverse video of the selected area. G can be any of the graphic variables and is optional. The default value is G0.\n\n');
                markdownString.appendMarkdown('x2, y2 are optional, and if not specified, they will be the bottom right of the graphic.\n\n');
                markdownString.appendMarkdown('x1, y1 are optional, and if not specified, they will be the top left of the graphic. If only one pair x,y is specified, it refers to the top left.\n\n');
                return new vscode.Hover(markdownString);
            }
            
            if (word === 'LINE') {
                const markdownString = new vscode.MarkdownString();
                markdownString.appendMarkdown('```hp-prime\n');
                markdownString.appendMarkdown('LINE([G], x1, y1, x2, y2, [color])\n\n');
                markdownString.appendMarkdown('```\n\n');
                markdownString.appendMarkdown('---\n\n');
                markdownString.appendMarkdown('`Description:`\n\n');
                markdownString.appendMarkdown('The basic format of LINE draws a line between the pixel coordinates of the graphic using the specified color.\n\n');
                return new vscode.Hover(markdownString);
            }
            
            if (word === 'LINE_P') {
                const markdownString = new vscode.MarkdownString();
                markdownString.appendMarkdown('```hp-prime\n');
                markdownString.appendMarkdown('LINE_P([G], x1, y1, x2, y2, [color])\n\n');
                markdownString.appendMarkdown('```\n\n');
                markdownString.appendMarkdown('---\n\n');
                markdownString.appendMarkdown('`Description:`\n\n');
                markdownString.appendMarkdown('The basic format of LINE_P draws a line between the pixel coordinates of the graphic using the specified color.\n\n');
                return new vscode.Hover(markdownString);
            }
            if (word === 'PIXON') {
                const markdownString = new vscode.MarkdownString();
                markdownString.appendMarkdown('```hp-prime\n');
                markdownString.appendMarkdown('PIXON([G], x, y [,color])\n\n');
                markdownString.appendMarkdown('```\n\n');
                markdownString.appendMarkdown('---\n\n');
                markdownString.appendMarkdown('`Description:`\n\n');
                markdownString.appendMarkdown('Sets the color of the pixel in the graphic variable G at the coordinates (x, y) to the specified color. G can be any of the graphic variables and is optional. The default value is G0, the current image.\n\n');
                return new vscode.Hover(markdownString);
            }
            if (word === 'PIXON_P') {
                const markdownString = new vscode.MarkdownString();
                markdownString.appendMarkdown('```hp-prime\n');
                markdownString.appendMarkdown('PIXON_P([G], x, y [,color])\n\n');
                markdownString.appendMarkdown('```\n\n');
                markdownString.appendMarkdown('---\n\n');
                markdownString.appendMarkdown('`Description:`\n\n');
                markdownString.appendMarkdown('Sets the color of the pixel in the graphic variable G at the coordinates (x, y) to the specified color. G can be any of the graphic variables and is optional. The default value is G0, the current image.\n\n');
                return new vscode.Hover(markdownString);
            }
            if (word === 'RECT') {
                const markdownString = new vscode.MarkdownString();
                markdownString.appendMarkdown('```hp-prime\n');
                markdownString.appendMarkdown('RECT([G, x1, y1, x2, y2, borderColor, fillColor])\n\n');
                markdownString.appendMarkdown('```\n\n');
                markdownString.appendMarkdown('---\n\n');
                markdownString.appendMarkdown('`Example:`\n\n');
                markdownString.appendMarkdown('```hp-prime\n');
                markdownString.appendMarkdown('EXPORT BOX()\n');
                markdownString.appendMarkdown('BEGIN\n');
                markdownString.appendMarkdown('RECT();\n');
                markdownString.appendMarkdown('RECT_P(40, 90, 320, 240, #000000, #FF0000);\n');
                markdownString.appendMarkdown('WAIT;\n');
                markdownString.appendMarkdown('END;\n');
                markdownString.appendMarkdown('```\n\n');
                markdownString.appendMarkdown('---\n\n');
                markdownString.appendMarkdown('`Description:`\n\n');
                markdownString.appendMarkdown('Draws a rectangle in G between the points x1,y1 and x2,y2 using the border color for the perimeter and the fill color for the interior.\n\n');
                markdownString.appendMarkdown('G can be any of the graphic variables and is optional. The default value is G0, the current image.\n\n');
                markdownString.appendMarkdown('x1, y1 are optional. The default values represent the top-left corner of the graphic.\n\n');
                markdownString.appendMarkdown('x2, y2 are optional. The default values represent the bottom-right corner of the image.\n\n');
                markdownString.appendMarkdown('Border color and fill color can be any color specified as #RRGGBB. Both are optional, and if not specified, the fill color will use the default border color values.\n\n');
                markdownString.appendMarkdown('To erase a GROB, execute RECT(G). To clear the screen, execute RECT().\n\n');
                markdownString.appendMarkdown('When optional arguments are provided in a command with multiple optional parameters (like RECT), the supplied arguments correspond first to the leftmost parameters. For example, in the following program, the arguments 40 and 90 in the RECT_P command correspond to x1 and y1. The argument #000000 corresponds to the border color, as it is the only additional argument. If there had been two additional arguments, they should have referred to x2 and y2 instead of border color and fill color. The program produces a rectangle with a black border and a red fill.\n\n');
                return new vscode.Hover(markdownString);
            }
            if (word === 'RECT_P') {
                const markdownString = new vscode.MarkdownString();
                markdownString.appendMarkdown('```hp-prime\n');
                markdownString.appendMarkdown('RECT_P([G, x1, y1, x2, y2, borderColor, fillColor])\n\n');
                markdownString.appendMarkdown('```\n\n');
                markdownString.appendMarkdown('---\n\n');
                markdownString.appendMarkdown('`Example:`\n\n');
                markdownString.appendMarkdown('```hp-prime\n');
                markdownString.appendMarkdown('EXPORT BOX()\n');
                markdownString.appendMarkdown('BEGIN\n');
                markdownString.appendMarkdown('RECT();\n');
                markdownString.appendMarkdown('RECT_P(40, 90, 320, 240, #000000, #FF0000);\n');
                markdownString.appendMarkdown('WAIT;\n');
                markdownString.appendMarkdown('END;\n');
                markdownString.appendMarkdown('```\n\n');
                markdownString.appendMarkdown('---\n\n');
                markdownString.appendMarkdown('`Description:`\n\n');
                markdownString.appendMarkdown('Draws a rectangle in G between the points x1,y1 and x2,y2 using the border color for the perimeter and the fill color for the interior.\n\n');
                markdownString.appendMarkdown('G can be any of the graphic variables and is optional. The default value is G0, the current image.\n\n');
                markdownString.appendMarkdown('x1, y1 are optional. The default values represent the top-left corner of the graphic.\n\n');
                markdownString.appendMarkdown('x2, y2 are optional. The default values represent the bottom-right corner of the image.\n\n');
                markdownString.appendMarkdown('Border color and fill color can be any color specified as #RRGGBB. Both are optional, and if not specified, the fill color will use the default border color values.\n\n');
                markdownString.appendMarkdown('To erase a GROB, execute RECT(G). To clear the screen, execute RECT().\n\n');
                markdownString.appendMarkdown('When optional arguments are provided in a command with multiple optional parameters (like RECT), the supplied arguments correspond first to the leftmost parameters. For example, in the following program, the arguments 40 and 90 in the RECT_P command correspond to x1 and y1. The argument #000000 corresponds to the border color, as it is the only additional argument. If there had been two additional arguments, they should have referred to x2 and y2 instead of border color and fill color. The program produces a rectangle with a black border and a red fill.\n\n');
                return new vscode.Hover(markdownString);
            }
            if (word === 'SUBGROB') {
                const markdownString = new vscode.MarkdownString();
                markdownString.appendMarkdown('```hp-prime\n');
                markdownString.appendMarkdown('SUBGROB(srcGRB [,x1, y1, x2, y2], trgtGRB)\n\n');
                markdownString.appendMarkdown('```\n\n');
                markdownString.appendMarkdown('---\n\n');
                markdownString.appendMarkdown('`Example:`\n\n');
                markdownString.appendMarkdown('```hp-prime\n');
                markdownString.appendMarkdown('SUBGROB(G1, G4) will copy G1 to G4.\n');
                markdownString.appendMarkdown('```\n\n');
                markdownString.appendMarkdown('---\n\n');
                markdownString.appendMarkdown('`Description:`\n\n');
                markdownString.appendMarkdown('Sets trgtGRB to be a copy of the area of srcGRB between the points x1,y1 and x2,y2.\n\n');
                markdownString.appendMarkdown('srcGRB can be any of the graphic variables and is optional. The default value is G0.\n\n');
                markdownString.appendMarkdown('trgtGRB can be any of the graphic variables except G0.\n\n');
                markdownString.appendMarkdown('x2, y2 are optional, and if not specified, it will be the bottom-right corner of srcGRB.\n\n');
                markdownString.appendMarkdown('x1, y1 are optional, and if not specified, it will be the top-left corner of srcGRB.\n\n');
                return new vscode.Hover(markdownString);
            }
            if (word === 'SUBGROB_P') {
                const markdownString = new vscode.MarkdownString();
                markdownString.appendMarkdown('```hp-prime\n');
                markdownString.appendMarkdown('SUBGROB_P(srcGRB [ ,x1, y1, x2, y2], trgtGRB)\n\n');
                markdownString.appendMarkdown('```\n\n');
                markdownString.appendMarkdown('---\n\n');
                markdownString.appendMarkdown('`Example:`\n\n');
                markdownString.appendMarkdown('```hp-prime\n');
                markdownString.appendMarkdown('SUBGROB_P(G1, G4) will copy G1 into G4.\n');
                markdownString.appendMarkdown('```\n\n');
                markdownString.appendMarkdown('---\n\n');
                markdownString.appendMarkdown('`Description:`\n\n');
                markdownString.appendMarkdown('Sets trgtGRB to be a copy of the area of srcGRB between the points x1,y1 and x2,y2.\n\n');
                markdownString.appendMarkdown('srcGRB can be any of the graphics variables and is optional. The default is G0.\n\n');
                markdownString.appendMarkdown('trgtGRB can be any of the graphics variables except G0.\n\n');
                markdownString.appendMarkdown('x2, y2 are optional and if not specified will be the bottom right of srcGRB.\n\n');
                markdownString.appendMarkdown('x1, y1 are optional and if not specified will be the top left of srcGRB.\n\n');
                return new vscode.Hover(markdownString);
            }
            if (word === 'TEXTOUT') {
                const markdownString = new vscode.MarkdownString();
                markdownString.appendMarkdown('```hp-prime\n');
                markdownString.appendMarkdown('TEXTOUT(text, [G], x, y ,source, c1, width, c2)\n\n');
                markdownString.appendMarkdown('```\n\n');
                markdownString.appendMarkdown('---\n\n');
                markdownString.appendMarkdown('`Example:`\n\n');
                markdownString.appendMarkdown('```hp-prime\n');
                markdownString.appendMarkdown('TEXTOUT("Hello",G1, 150, 200, 2, #FF0000, 150, #FFFFFF])\n');
                markdownString.appendMarkdown('```\n\n');
                markdownString.appendMarkdown('---\n\n');
                markdownString.appendMarkdown('`Description:`\n\n');
                markdownString.appendMarkdown('Draws text using color c1 in the graphics G at position x, y using font. Does not draw text larger than width in pixels and clears the background before drawing the text using color c2.\n\n');
                markdownString.appendMarkdown('G can be any of the graphics variables and is optional. The default is G0. This command returns the x coordinate of the pixel at the end of the text output.\n\n');
                markdownString.appendMarkdown('Source can be:\n\n');
                markdownString.appendMarkdown('0: currently selected font on the Home Screen Settings, 1: small font 2: large font. Source is optional and if specified is the currently selected font in Home Settings.\n\n');
                markdownString.appendMarkdown('c1 can be any color specified as #RRGGBB. The default is black (#000000). Width is optional and if not specified, no clipping will occur.\n\n');
                markdownString.appendMarkdown('c2 can be any color specified as #RRGGBB. c2 is optional. If not specified, the background will not be cleared.\n\n');
                return new vscode.Hover(markdownString);
            }
            if (word === 'TEXTOUT_P') {
                const markdownString = new vscode.MarkdownString();
                markdownString.appendMarkdown('```hp-prime\n');
                markdownString.appendMarkdown('TEXTOUT_P(text, [G], x, y ,source, c1, width, c2)\n\n');
                markdownString.appendMarkdown('```\n\n');
                markdownString.appendMarkdown('---\n\n');
                markdownString.appendMarkdown('`Example:`\n\n');
                markdownString.appendMarkdown('```hp-prime\n');
                markdownString.appendMarkdown('TEXTOUT_P("Hello",G1, 150, 200, 2, #FF0000, 150, #FFFFFF])\n');
                markdownString.appendMarkdown('```\n\n');
                markdownString.appendMarkdown('---\n\n');
                markdownString.appendMarkdown('`Description:`\n\n');
                markdownString.appendMarkdown('Draws text using color c1 in the graphics G at position x, y using font. Does not draw text larger than width in pixels and clears the background before drawing the text using color c2.\n\n');
                markdownString.appendMarkdown('G can be any of the graphics variables and is optional. The default is G0. This command returns the x coordinate of the pixel at the end of the text output.\n\n');
                markdownString.appendMarkdown('Source can be:\n\n');
                markdownString.appendMarkdown('0: currently selected font on the Home Screen Settings, 1: small font 2: large font. Source is optional and if specified is the currently selected font in Home Settings.\n\n');
                markdownString.appendMarkdown('c1 can be any color specified as #RRGGBB. The default is black (#000000). Width is optional and if not specified, no clipping will occur.\n\n');
                markdownString.appendMarkdown('c2 can be any color specified as #RRGGBB. c2 is optional. If not specified, the background will not be cleared.\n\n');
                return new vscode.Hover(markdownString);
            }
            if (word === 'TRIANGLE_P') {
                const markdownString = new vscode.MarkdownString();
                markdownString.appendMarkdown('```hp-prime\n');
                markdownString.appendMarkdown('TRIANGLE_P([G], x1, y1, x2, y2, x3, y3, c1, [c2, c3], [alpha], ["StringZ", z1, z2, z3])\n\n');
                markdownString.appendMarkdown('```\n\n');
                markdownString.appendMarkdown('---\n\n');
                markdownString.appendMarkdown('`Example:`\n\n');
                markdownString.appendMarkdown('```hp-prime\n');
                markdownString.appendMarkdown('RECT();\n');
                markdownString.appendMarkdown('TRIANGLE_P(40,20,120,100,40,100,RGB(128,0,255),128);\n');
                markdownString.appendMarkdown('WAIT();\n');
                markdownString.appendMarkdown('TRIANGLE_P({265,145,RGB(128,0,255),90},{240,220,RGB(255,200,50),100},{195,157,RGB(196,0,0),128});\n');
                markdownString.appendMarkdown('WAIT();\n');
                markdownString.appendMarkdown('```\n\n');
                markdownString.appendMarkdown('---\n\n');
                markdownString.appendMarkdown('`Description:`\n\n');
                markdownString.appendMarkdown('Draws a triangle between the specified pixel coordinate points in the graphics using the specified color and transparency (0 ≤ Alpha ≤ 255). If 3 colors are specified, it blends the colors between the vertices.\n\n');
                markdownString.appendMarkdown('The following form of TRIANGLE allows for the visualization of multiple triangles at once.\n\n');
                markdownString.appendMarkdown('This is primarily used if you have a set of vertices and want to display them all at once.\n\n');
                markdownString.appendMarkdown('The first 2 arrays indicate the x/y coordinates and the colors of each point. TRIANGLE_P will draw 1 quadrilateral for each set of 4 adjacent vertices and blend the colors associated with the 4 points.\n\n');
                markdownString.appendMarkdown('If a z projection array is provided, for each point, this array is multiplied by the vector [x,y,z,1] to create the x,y viewing coordinates.\n\n');
                markdownString.appendMarkdown('If zcode is a list containing 3 real numbers { ex, ey, ez } then x,y are further modified making x=ez/z*x-ex and y=ez/z*y-ey creating a perspective projection.\n\n');
                markdownString.appendMarkdown('If zstring is provided, z clipping will occur using the z value (see below).\n\n');
                markdownString.appendMarkdown('If zcode="N" or is a list starting with "N", then each z is normalized between 0 and 255.\n\n');
                return new vscode.Hover(markdownString);
            }
            if (word === 'PIXOFF') {
                const markdownString = new vscode.MarkdownString();
                markdownString.appendMarkdown('```hp-prime\n');
                markdownString.appendMarkdown('PIXOFF([G], x, y)\n\n');
                markdownString.appendMarkdown('```\n\n');
                markdownString.appendMarkdown('---\n\n');
                markdownString.appendMarkdown('`Description:`\n\n');
                markdownString.appendMarkdown('Sets the color of the pixel G with coordinates x,y to white. G can be any of the graphics variables and is optional. The default is G0, the current image.\n\n');
                return new vscode.Hover(markdownString);
            }
            if (word === 'PIXOFF_P') {
                const markdownString = new vscode.MarkdownString();
                markdownString.appendMarkdown('```hp-prime\n');
                markdownString.appendMarkdown('PIXOFF_P([G], x, y)\n\n');
                markdownString.appendMarkdown('```\n\n');
                markdownString.appendMarkdown('---\n\n');
                markdownString.appendMarkdown('`Description:`\n\n');
                markdownString.appendMarkdown('Sets the color of the pixel G with coordinates x,y to white. G can be any of the graphics variables and is optional. The default is G0, the current image.\n\n');
                return new vscode.Hover(markdownString);
            }
        }
    });
    
    context.subscriptions.push(disposable);
}

exports.activate = activate;
