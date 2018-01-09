; File - 86InitPaging.asm
;
; Summary -
; Pagination support starts from this file, which also contains the permanent
; page tables for the kernel.
;
; Changes:
; # IdentityDirectory was removed from perm-data. This was because both
;   GlobalDirectory & IdentityDirectory mapped the same addresses - 0 to 2mb
;   of low memory. Now GlobalDirectory is mapped twice in the PDPT.
;
; Copyright (C) 2017 - Sukant Pal

SECTION .KernelPermData

	global GlobalTable
	align 0x1000
	GlobalTable:
		times 1022 dd 0
		DD GlobalDirectory
		DD 0

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

	global PDPT
	align 0x20
	PDPT:
		DD (GlobalDirectory - 0xC0000000 + 1)
		DD 0
		TIMES 4 DD 0
		DD (GlobalDirectory - 0xC0000000 + 1)
		DD 0

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

		JMP DWORD EDX

	global SwitchPaging
	SwitchPaging:
		MOV ECX, [ESP + 4]
		MOV CR3, ECX
		RET

		global SpinLock
		SpinLock:
			PUSH EAX
			PUSH ECX
			MOV EAX, [ESP + 12]
			SpinLoop:
				MOV ECX, 1
				XCHG [EAX], ECX
				CMP ECX, 0
				JE SpinLoop
			MFENCE
			POP ECX
			POP EAX
			RET

		global SpinUnlock
		SpinUnlock:
			PUSH EAX
			MOV EAX, [ESP + 8]
			MOV DWORD [EAX], 0
			MFENCE
			POP EAX
			RET
