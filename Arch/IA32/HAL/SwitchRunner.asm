; Copyright (C) - Shukant Pal

; SwitchRunner.asm provides the dispatcher module for the CircuitScheduler
; and consists of a intricate branching code. It provides services of the BSP
; such as kTime, RRTS (Runner's Realtime Status), etc.
;
; This file can be configured easily for -
; 1. Which register's to save in context switch?
; 2. BSP Services
;

SECTION .bss
	global ef
	ef : resb 4

SECTION .text

	extern VAPICBase
	extern BSP_ID
	extern XMilliTime
	global KiClockRespond
	KiClockRespond:
		MFENCE ; Finish all memory operations

		PUSH EAX
		PUSH EBX
		PUSH ECX
		PUSH EDX
		PUSH ESI
		PUSH EDI
		PUSH EBP

		MOV EBP, ESP 				; Load ESP into EBP
		add EBP, 28 					; Load Interrupt Stack Frame

		MOV EDX, [VAPICBase] 	; Use APIC Registers
		MOV EDX, [EDX + 0x20] 	; Load PROCESSOR_ID << 24
		SHR EDX, 24					; Get APIC_ID
		CMP [BSP_ID], EDX 		; Compare for BSP_ID
		JNE KiScheduleEntry 		; If not the BSP, don't update time

		; Update kTime
		PUSH EDX						; Save PROCESSOR_ID on stack
		MOV EDX, 1					; Load 1 for incrementing time
		XADD [XMilliTime], EDX 	; Update time and load old time into EDX
		CMP [XMilliTime], EDX	; Test for overflow in lower 32-bits
		POP EDX						; Restore PROCESSOR_ID into EDX
		JLE Incro						; If overflow occured, increment upper-32bits
		JMP KiRunnerUpdate		; Else, continue

		Incro:
		LOCK INC DWORD [XMilliTime + 4]

	KiRunnerUpdate:
		

	; KiScheduleEntry() - 
	;
	; Summary:
	; This function is used for invoking the scheduler in secure way. It is internal
	; here and is directly used in APs. It assumes the registers to be PUSHed on the
	; stack and the PROCESSOR_ID to be stored in the EDX register.
	;
	; It will grab the processor info, and then pass on the required information to the
	; scheduler. From, there the scheduler will return to the KiScheduleExit code.
	;
	; This function operates inside the kernel stack of the thread.
	;
	; @Before PreemptAndScheduler, @Tick.asm
	; @Version 1.2
	; @Since Circuit 2.03
	extern Schedule
	KiScheduleEntry:
		SHL EDX, 12												; Get KPAGE offset for the PROCESSOR struct
		ADD EDX, 0xc0000000 + 20 * 1024 * 1024	; Add offset to KCPUINFO to get the PROCESSOR struct

		MOV EBX, [EDX + 24] ; Runner Info
		MOV EDI, [EBX + 32] ; Runner's KernelStack

		MOV ECX, [EBP]
		MOV [EBX + 16], ECX ; Runner's EIP

		CMP DWORD [EBP + 4], 0x8
		je KiScheduleKernel

		KiScheduleUser:
			MOV ECX, [EBP + 12] 								; UserStack.Pointer
			MOV ESI, [EBX + 28] 								; Store the UserStack
			MOV [ESI + 4], ECX 								; Save UserStack.Pointer
			SUB EBP, 28											; Load Save-Register StackFrame
			MOV [EDI + 4], EBP									; Save KernelStack.Pointer
			JMP KiInvokeScheduler							; Invoke the scheduler

		KiScheduleKernel:
			SUB EBP, 28											; Load Save-Register StackFrame
			MOV [EDI + 4], EBP									; Save KernelStack.Pointer

		KiInvokeScheduler:
		PUSH EDX
		PUSH EDX												; Pass Arg-pCPU
		CALL Schedule											; Scheduler the next Runner

	; KiScheduleExit() -
	;
	; Summary:
	; It invokes the Run() command in the Runner's info.
	;
	; @Version1
	; @Since Circuit 2.03
	KiScheduleExit:
		ADD ESP, 4
		POP EDX

		MOV EBX, [EDX + 24] ; New Runner
		MOV EDI, [EBX + 32] ; Runner->KernelStack (*)

		MOV ESP, [EDI + 4] ; Runner.KernelStack->Pointer
		PUSH EBX	; Save Runner
		ADD ESP, 4

		CMP DWORD [EBX + 20], 0
		JE KiDispatchStatus
		PUSH EBX
		CALL [EBX + 20] ; Runner.Run()	

		KiDispatchStatus:
		BTR DWORD [EBX + 24], 0
		JC KiJumpNew
		; EBX (Runner) = ESP - 32

		CALL EOI
		POP EBP
		POP EDI
		POP ESI
		POP EDX
		POP ECX
		POP EBX
		POP EAX
		IRET

	; KiJumpNew() -
	;
	; Summary - 
	; This function is used to return to freshly made runners, because their interrupt state is not saved
	; on the stack.
	;
	; @Version 1
	; @Since Circuit 2.03
	KiJumpNew:
		BT DWORD [EBX + 24], 1
		JC KiJumpKernel

		KiJumpUser:
			; //TODO: Implement user-mode intr return
			PUSH DWORD 0x23
			PUSH DWORD [EBX + 32]
			PUSH DWORD (1 << 9) ; EFLAGS with IF
			PUSH DWORD 0x0 ; CS
			PUSH DWORD [EBX + 16]
			CALL EOI
			IRET

		KiJumpKernel:
			xchg bx, bx
			PUSH DWORD (1 << 1) | (1 << 9) ; EFLAGS with IF
			PUSH DWORD 0x8
			PUSH DWORD [EBX + 16]
			CALL EOI
			IRET

	global EOI
	EOI:
		MOV EDX, [VAPICBase]
		MOV DWORD [EDX + 0xB0], 0
		RET
