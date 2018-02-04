section .text

	Exit:
		RET

	global ExecuteLGDT
	ExecuteLGDT:
		MOV EAX, [ESP + 4]
		LGDT [EAX]
		MOV AX, 0x10
		MOV DS, AX
		MOV ES, AX
		MOV FS, AX
		MOV GS, AX
		MOV SS, AX
		JMP 0x8:Exit

	global ExecuteLIDT
	ExecuteLIDT:
		MOV EAX, [ESP + 4]
		LIDT [EAX]
		STI
		RET

	global ExecuteLTR
	ExecuteLTR:
		MOV AX, 0x28
		LTR AX
		RET
