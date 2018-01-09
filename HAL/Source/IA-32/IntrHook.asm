; Copyright (C) - Shukant Pal

section .text

	global DoubleFault
	extern HandleDF
	DoubleFault:
		call HandleDF	
		iret

	global InvalidTSS
	extern HandleIT
	InvalidTSS:
		call HandleIT
		iret

	global SegmentNotPresent
	extern HandleSNP
	SegmentNotPresent:
		call HandleSNP
		iret

	global GeneralProtectionFault
	extern HandleGPF
	GeneralProtectionFault:
		call HandleGPF
		iret

	global PageFault
	extern HandlePF
	PageFault:
		mov eax, cr2
		mov [regInfo], eax
		call HandlePF
		iret

	global Spurious
	extern DbgErro
	Spurious:
		iret

	extern EOI
	global TimerUpdate
	TimerUpdate:
		LOCK INC DWORD [DelayTime]							; Update current time
		PUSH EDX											; Save EDX
		CALL EOI											; Do a EOI
		POP EDX												; Restore EDX
		MFENCE
		IRET

	global TimerWait
	TimerWait:
		MOV EBX, [ESP + 4]									; Load wait time into EBX
		ADD EBX, [DelayTime]								; Calculate stopping time
		CompareTimer:
		NOP
		PAUSE
		CMP DWORD [DelayTime], EBX							; If current time is equ to stopping
		JNE CompareTimer									; if not, loop again
		RET
	
	global Executable_ProcessorBinding_IPIRequest_Invoker
	extern Executable_ProcessorBinding_IPIRequest_Handler
	Executable_ProcessorBinding_IPIRequest_Invoker:
		PUSHA
		PUSH EBP
		PUSH ESI
		PUSH EDI
		CALL Executable_ProcessorBinding_IPIRequest_Handler
		CALL EOI
		POP EDI
		POP ESI
		POP EBP
		POPAD
		IRET

	global RR_BalanceRunqueue
	extern RrBalanceRoutine
	RR_BalanceRunqueue:
	;	PUSHA
	;	CALL RrBalanceRoutine
	;	POPA
		IRET

section .bss
	global regInfo
	regInfo : resb 4
	DelayTime : resb 4
