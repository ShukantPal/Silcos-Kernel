; Copyright (C) 2017 - Sukant Pal

SECTION .data

	global GlobalTable
	align 0x1000
	GlobalTable:
		times 1022 dd 0
		DD GlobalDirectory
		DD 0
		
	global IdentityDirectory
	align 0x1000
	IdentityDirectory:
		DD 0 | 3 | (1 << 7)
		DD 0
		DD 2097152 | 3 | (1 << 7)
		DD 0
		TIMES 1020 DD 0

	; ---------------------------

	global GlobalDirectory
	align 0x1000
	GlobalDirectory:
		DD 0 | 3 | (1 << 7)
		DD 0
		DD 2097152 | 3 | (1 << 7)
		DD 0
		TIMES 1018 DD 0
		DD (GlobalTable - 0xC0000000 + 3)
		DD 0

	; ---------------------------

	global PDPT
	align 0x20
	PDPT:
		DD (IdentityDirectory - 0xC0000000 + 1)
		DD 0
		TIMES 4 DD 0
		DD (GlobalDirectory - 0xC0000000 + 1)
		DD 0

	; ---------------------------

SECTION .text

	global InitPaging
	InitPaging:
		MOV ECX, CR4
		BTS ECX, 5
		MOV CR4, ECX

		MOV ECX, (PDPT - 0xC0000000)
		MOV CR3, ECX

		MOV ECX, CR0
		OR ECX, 0x80000000
		MOV CR0, ECX

		LEA ECX, [EraseIdentity]
		JMP ECX

	; ---------------------------

	global EraseIdentity
	extern InitEntrance
	EraseIdentity:
	;	MOV DWORD [IdentityDirectory], 0
	;	INVLPG [0]
		JMP DWORD EDX

	; ---------------------------

	global SwitchPaging
	SwitchPaging:
		MOV ECX, [ESP + 4]
		MOV CR3, ECX
		RET

	; ---------------------------

; -----------------------------------
