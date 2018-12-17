; Copyright (C) 2017 - Shukant Pal

section .text

	global TestCPUID
	TestCPUID:
		PUSHFD 								; Save EFLAGS
		PUSHFD 								; Store EFLAGS
		XOR DWORD [ESP], 0x200000 ; Invert the ID bit
		POPFD 									; Load stored EFLAGS (with inverted ID bit)
		PUSHFD 								; Store EFLAGS again
		POP EAX 								; EAX contains the modified EFLAGS
		XOR EAX, [ESP]					 	; EAX contains only modified bits
		POPFD 									; Restore EFLAGS
		AND EAX, 0x200000				; EAX is zero if the ID bit can't be changed, else non-zero
		RET										; Return to the caller

	global __cpuid
	__cpuid:
		PUSH EAX
		PUSH EBX
		PUSH ECX
		PUSH EDX
		PUSH EBP
		MOV EBP, ESP			; Load stack pointer into EBP
		ADD EBP, 24				; Store address of first argument here!
		MOV EAX, [EBP]			; Load CPUID.EAX
		MOV ECX, [EBP + 4]		; Load CPUID.ECX
		CPUID

		PUSH ESI				; Save ESI so that we could use it
		MOV ESI, [EBP + 8]		; Load pointer to cRegs buffer
		MOV [ESI], EAX
		MOV [ESI + 4], EBX
		MOV [ESI + 8], ECX
		MOV [ESI + 12], EDX
		POP ESI

		POP EBP
		POP EDX
		POP ECX
		POP EBX
		POP EAX
		RET

		PUSH EBX
		PUSH EDX
		MOV EAX, [ESP + 12]		; Load CPUID.EAX
		MOV ECX, [ESP + 16] 	; Load CPUID.ECX
		CPUID 					; Invoke CPUID
		PUSH EBP 				; Save the base pointer
		MOV EBP, [ESP + 24] 	; Load the destination pointer
		MOV [EBP], EAX			; Output EAX
		MOV [EBP + 4], EBX		; Output EBX
		MOV [EBP + 8], ECX		; Output ECX
		MOV [EBP + 12], EDX		; Output EDX
		POP EBP 				; Restore the base pointer
		POP EDX					; Restore EDX
		POP EBX					; Restore EBX
		RET

		global FindMaskWidth
		FindMaskWidth:
			MOV EAX, [ESP + 4]
			MOV ECX, 0
			DEC EAX
			BSR CX, AX
			JZ fmwNext
			INC CX
			fmwNext:
			MOV EAX, ECX
			RET
	
