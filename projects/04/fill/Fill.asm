// This file is part of www.nand2tetris.org
// and the book "The Elements of Computing Systems"
// by Nisan and Schocken, MIT Press.
// File name: projects/04/Fill.asm

// Runs an infinite loop that listens to the keyboard input.
// When a key is pressed (any key), the program blackens the screen,
// i.e. writes "black" in every pixel;
// the screen should remain fully black as long as the key is pressed. 
// When no key is pressed, the program clears the screen, i.e. writes
// "white" in every pixel;
// the screen should remain fully clear as long as no key is pressed.

// Put your code here.
	@clr 	// Currently set color
	M=0		// Start is white (0 = white, -1 = black)
	D=0
	@CHECKSTATE
	0;JMP

(GETKBD)
	@KBD
	D=M
	@CHECKSTATE
	D;JEQ	// If no key is set, D=0
	D=-1	// Some key is set, set D=-1

(CHECKSTATE)	// On entry, D must be color to set (0 or -1)
	@toSet
	M=D
	@clr
	D=D-M		// If D==M => 0
	@GETKBD
	D;JEQ		// Check keyboard again

	// We have a state change
	// Store color to set
	@toSet
	D=M
	@clr
	M=D

	// Fill
	@i 		// Running counter, start at 0
	M=0
(FILLLOOP)
	@SCREEN
	D=A
	@i
	D=D+M
	@pos
	M=D
	@clr
	D=M
	@pos
	A=M
	M=D
	@i
	MD=M+1
	@8192	// Screen size
	D=D-A
	@FILLLOOP
	D;JLT
	@GETKBD
	0;JMP