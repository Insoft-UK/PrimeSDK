# HP PPL/PPL+ Programming Manual

### 1. Introduction to HP PPL
HP PPL (Prime Programming Language) is a high-level, Pascal-like language developed for the HP Prime Graphing Calculator. It allows users to create custom programs and applications, leveraging the calculator’s advanced features, including its Computer Algebra System (CAS), graphics capabilities, and user interface components.

### 2. Program Structure
**A basic HP PPL program follows this structure**:

<sub>PPL</sub>
```
EXPORT ProgramName()
BEGIN
  // Your code here
END;
```

<sub>PPL+</sub>
```
CATALOG ProgramName()
BEGIN
  // Your code here
END;
```

- **EXPORT**: Makes the program accessible from the calculator’s program catalog.
- **ProgramName**: The name of your program.
- **BEGIN … END;**: Encloses the main body of the program.

### 3. Variables and Data Types

**HP PPL supports various data types**:
- **Numeric Types**: Real and complex numbers.
- **Strings**: Text enclosed in double quotes.
- **Lists**: Ordered collections, e.g., {1, 2, 3}.
- **Matrices**: Two-dimensional arrays.
- **User-defined types**: Created using structures.

**Variables can be declared using the LOCAL keyword**:

<sub>PPL</sub>
```
LOCAL x, y, z;
```

<sub>PPL+</sub>
```
VAR x, y, z;
```

### 4. Control Structures

**HP PPL includes standard control structures**:

- **Conditional Statements**:

<sub>PPL</sub>
```
IF condition THEN
  // code
ELSE
  // code
END;
```

- **Loops**:
  - **FOR Loop**:

<sub>PPL</sub>
```
FOR i FROM 1 TO 10 DO
  // code
END;
```

- **WHILE Loop**:

<sub>PPL</sub>
```
REPEAT
  // code
UNTIL condition;
```

### 5. Functions and Procedures
**You can define functions to encapsulate reusable code**:

<sub>PPL</sub>
```
EXPORT Add(a, b)
BEGIN
  RETURN a + b;
END;
```
Functions can return values using the RETURN statement.

### 6. Input and Output

- **Displaying Output**:
- **PRINT("Text");**: Displays text in the terminal view.
- **TEXTOUT_P("Text", x, y);**: Outputs text on the graphics screen at coordinates (x, y).
- **Receiving Input**:
- **INPUT(var);**: Prompts the user to input a value for var.
- **INPUT({var1, var2});**: Prompts for multiple inputs.

### 7. Graphics and Drawing
**HP PPL provides commands for graphical operations**:

- **Drawing Primitives**:
- **LINE_P(x1, y1, x2, y2);**: Draws a line between two points.
- **RECT_P(x1, y1, x2, y2);**: Draws a rectangle.
- **ARC_P(x, y, radius, startAngle, endAngle);**: Draws an arc.
- **Graphics Screen Management**:
- **RECT();**: Clears the graphics screen.
- **FREEZE;**: Displays the current graphics screen.
- **WAIT(n);**: Pauses the program for n seconds.

### 8. Working with Lists and Matrices

- **Creating Lists**:

<sub>PPL</sub>
```
LOCAL myList := {1, 2, 3};
```

- **Accessing List Elements**:

<sub>PPL</sub>
```
myList(1); // Accesses the first element
```

-	**Creating Matrices**:

<sub>PPL</sub>
```
LOCAL myMatrix := [[1, 2], [3, 4]];
```

-	**Accessing Matrix Elements**:

<sub>PPL</sub>
```
myMatrix(1, 2); // Accesses element at row 1, column 2
```

### 9. Error Handling
**Use TRY...CATCH blocks to handle errors gracefully**:

<sub>PPL+</sub>
```
TRY
  // Code that may cause an error
CATCH
  // Error handling code
END;
```

### 10. Additional Resources
**For more in-depth information and examples, consider the following resources**:

- **HP Prime Programming Tutorial by Edward Shore**: An in-depth tutorial covering various aspects of HP PPL, including graphics, user input, and mathematical operations.
- **HP Prime [User Guide](https://dev.cemetech.net/tools/prime?utm_source=chatgpt.com)**: The official user guide provides comprehensive information on the calculator’s features and programming capabilities.
  
- **HP Prime Documentation Files on [hpcalc.org](https://www.hpcalc.org/prime/docs/?utm_source=chatgpt.com)**: A collection of documentation files, including command references and tutorials.
  
- **HP Prime Commands Wiki on [TI-Planet](https://wiki.tiplanet.org/HP_Prime/Commands?utm_source=chatgpt.com)**: An online reference of HP Prime commands and their usage.

