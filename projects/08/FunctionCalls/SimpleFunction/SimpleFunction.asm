
// function SimpleFunction.test 2
(SimpleFunction.test)
@SP
M=M+1
M=M+1
A=M-1
M=0
A=A-1
M=0

// push local 0
@LCL
A=M
D=M
@SP
M=M+1
A=M-1
M=D

// push local 1
@LCL
A=M+1
D=M
@SP
M=M+1
A=M-1
M=D

// add
@SP
AM=M-1
D=M
A=A-1
M=D+M

// not
@SP
A=M-1
M=!M

// push argument 0
@ARG
A=M
D=M
@SP
M=M+1
A=M-1
M=D

// add
@SP
AM=M-1
D=M
A=A-1
M=D+M

// push argument 1
@ARG
A=M+1
D=M
@SP
M=M+1
A=M-1
M=D

// sub
@SP
AM=M-1
D=M
A=A-1
M=M-D

// return
@LCL
D=M
@R13
M=D
@5
A=D-A
D=M
@R14
M=D
@SP
A=M-1
D=M
@ARG
A=M
M=D
@ARG
D=M+1
@SP
M=D
@R13
AM=M-1
D=M
@THAT
M=D
@R13
AM=M-1
D=M
@THIS
M=D
@R13
AM=M-1
D=M
@ARG
M=D
@R13
AM=M-1
D=M
@LCL
M=D
@R14
A=M
0;JMP
