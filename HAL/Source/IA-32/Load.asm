;
; Implements the loading parts of hardware-structure initialization. These
; in the form of LGDT, LIDT and LTR instructions.
;
; Other than that it is part of loading the C++ global objects and calling
; their constructors during __init() phase of HAL-module loading.
;

section .text

Exit:
	RET

global ExecuteLGDT
ExecuteLGDT:
	PUSH EAX
	MOV EAX, [ESP + 8]
	LGDT [EAX]
	MOV AX, 0x10
	MOV DS, AX
	MOV ES, AX
	MOV FS, AX
	MOV GS, AX
	MOV SS, AX
	POP EAX
	JMP 0x8:Exit

global ExecuteLIDT
ExecuteLIDT:
	PUSH EAX
	MOV EAX, [ESP + 8]
	LIDT [EAX]
	STI
	POP EAX
	RET

global ExecuteLTR
ExecuteLTR:
	PUSH EAX
	MOV AX, 0x28
	LTR AX
	POP EAX
	RET
