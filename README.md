# FORTRAN-Like
Lexer, parser, and interpreter for a simple FORTRAN-like programming language developed for a course project
Implements a LR recursive descent parser
Has four types INTEGER, REAL(float), CHAR(string) and nine keywords (PROGRAM, PRINT, READ, INTEGER, END, IF, THEN, REAL, CHAR)
Runs through the command line, after compiling the given cpp files, simmply run the executable with your code as a text file input
Note: Certain logical operations(>, <=, >=, &&, ||) do not exist
# Sample Input-Output
Input:
```
PROGRAM circle
	REAL : r, a, p, b  
	CHAR : str1, str2
	r = 5
	a = (3.14) * r * r
	IF ( r == 5) THEN
	  p = 2 * 3.14 * a
	  print, "r= ", r, " a= ", a, " p= ", p
	END IF
	
END PROGRAM circle
```

Output:
```
r= 5 a= 78.5 p= 492.98

Successful Execution
```

Input:
```
PROGRAM arith
	REAL : r, a, p, b  
	CHAR : str1, str2
	r = 5
	p = 7
	str1 = "Success"
	str2 = "Failure"
	a = 1 \* p \* r + r \* r
	IF ( a == 60) THEN
	  PRINT, str1
	END IF
	if (a == 200) then
	  print, str2
	end if
	
END PROGRAM arith
```
Output:
```
Success

Successful Execution
```
