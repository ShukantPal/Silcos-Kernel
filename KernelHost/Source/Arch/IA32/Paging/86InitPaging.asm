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

SECTION .text

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
