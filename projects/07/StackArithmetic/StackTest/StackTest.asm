
// push constant 17
@17
D=A
@SP
M=M+1
A=M-1
M=D

// push constant 17
@17
D=A
@SP
M=M+1
A=M-1
M=D

// eq
@SP
AM=M-1
D=M
A=A-1
A=M
D=D-A
@_cond_StackTest.0
D;JEQ
D=-1
(_cond_StackTest.0)
D=!D
@SP
A=M-1
M=D

// push constant 17
@17
D=A
@SP
M=M+1
A=M-1
M=D

// push constant 16
@16
D=A
@SP
M=M+1
A=M-1
M=D

// eq
@SP
AM=M-1
D=M
A=A-1
A=M
D=D-A
@_cond_StackTest.1
D;JEQ
D=-1
(_cond_StackTest.1)
D=!D
@SP
A=M-1
M=D

// push constant 16
@16
D=A
@SP
M=M+1
A=M-1
M=D

// push constant 17
@17
D=A
@SP
M=M+1
A=M-1
M=D

// eq
@SP
AM=M-1
D=M
A=A-1
A=M
D=D-A
@_cond_StackTest.2
D;JEQ
D=-1
(_cond_StackTest.2)
D=!D
@SP
A=M-1
M=D

// push constant 892
@892
D=A
@SP
M=M+1
A=M-1
M=D

// push constant 891
@891
D=A
@SP
M=M+1
A=M-1
M=D

// lt
@SP
AM=M-1
D=M
A=A-1
A=M
D=D-A
@_cond_t_StackTest.0
D;JLE
D=-1
@_cond_e_StackTest.0
0;JMP
(_cond_t_StackTest.0)
D=0
(_cond_e_StackTest.0)
@SP
A=M-1
M=D

// push constant 891
@891
D=A
@SP
M=M+1
A=M-1
M=D

// push constant 892
@892
D=A
@SP
M=M+1
A=M-1
M=D

// lt
@SP
AM=M-1
D=M
A=A-1
A=M
D=D-A
@_cond_t_StackTest.1
D;JLE
D=-1
@_cond_e_StackTest.1
0;JMP
(_cond_t_StackTest.1)
D=0
(_cond_e_StackTest.1)
@SP
A=M-1
M=D

// push constant 891
@891
D=A
@SP
M=M+1
A=M-1
M=D

// push constant 891
@891
D=A
@SP
M=M+1
A=M-1
M=D

// lt
@SP
AM=M-1
D=M
A=A-1
A=M
D=D-A
@_cond_t_StackTest.2
D;JLE
D=-1
@_cond_e_StackTest.2
0;JMP
(_cond_t_StackTest.2)
D=0
(_cond_e_StackTest.2)
@SP
A=M-1
M=D

// push constant 32767
@32767
D=A
@SP
M=M+1
A=M-1
M=D

// push constant 32766
@32766
D=A
@SP
M=M+1
A=M-1
M=D

// gt
@SP
AM=M-1
D=M
A=A-1
A=M
D=D-A
@_cond_t_StackTest.3
D;JGE
D=-1
@_cond_e_StackTest.3
0;JMP
(_cond_t_StackTest.3)
D=0
(_cond_e_StackTest.3)
@SP
A=M-1
M=D

// push constant 32766
@32766
D=A
@SP
M=M+1
A=M-1
M=D

// push constant 32767
@32767
D=A
@SP
M=M+1
A=M-1
M=D

// gt
@SP
AM=M-1
D=M
A=A-1
A=M
D=D-A
@_cond_t_StackTest.4
D;JGE
D=-1
@_cond_e_StackTest.4
0;JMP
(_cond_t_StackTest.4)
D=0
(_cond_e_StackTest.4)
@SP
A=M-1
M=D

// push constant 32766
@32766
D=A
@SP
M=M+1
A=M-1
M=D

// push constant 32766
@32766
D=A
@SP
M=M+1
A=M-1
M=D

// gt
@SP
AM=M-1
D=M
A=A-1
A=M
D=D-A
@_cond_t_StackTest.5
D;JGE
D=-1
@_cond_e_StackTest.5
0;JMP
(_cond_t_StackTest.5)
D=0
(_cond_e_StackTest.5)
@SP
A=M-1
M=D

// push constant 57
@57
D=A
@SP
M=M+1
A=M-1
M=D

// push constant 31
@31
D=A
@SP
M=M+1
A=M-1
M=D

// push constant 53
@53
D=A
@SP
M=M+1
A=M-1
M=D

// add
@SP
AM=M-1
D=M
A=A-1
A=M
D=D+A
@SP
A=M-1
M=D

// push constant 112
@112
D=A
@SP
M=M+1
A=M-1
M=D

// sub
@SP
AM=M-1
D=M
A=A-1
A=M
D=A-D
@SP
A=M-1
M=D

// neg
@SP
A=M-1
D=M
D=-D
@SP
A=M-1
M=D

// and
@SP
AM=M-1
D=M
A=A-1
A=M
D=D&A
@SP
A=M-1
M=D

// push constant 82
@82
D=A
@SP
M=M+1
A=M-1
M=D

// or
@SP
AM=M-1
D=M
A=A-1
A=M
D=D|A
@SP
A=M-1
M=D

// not
@SP
A=M-1
D=M
D=!D
@SP
A=M-1
M=D
