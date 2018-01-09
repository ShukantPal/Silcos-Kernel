; Copyright (C) - Shukant Pal
;
; provides the implementation for a scheduler-invoker on the ia32 arch, and
; is called when ever the apic-timer 'ticks'.
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
	MFENCE

	PUSH EAX
	PUSH EBX
	PUSH ECX
	PUSH EDX
	PUSH ESI
	PUSH EDI
	PUSH EBP

	MOV EBP, ESP 							; load the stack-frame in EBP
	ADD EBP, 28 							; go to the stack-frame with interrupt-context

	MOV EDX, [VAPICBase]
	MOV EDX, [EDX + 0x20] 					; load PROCESSOR_ID << 24
	SHR EDX, 24								; load APIC_ID
	CMP [BSP_ID], EDX 						; test if the cpu is the BSP
	JNE KiScheduleEntry

	; Update kTime
	PUSH EDX								; save PROCESSOR_ID on stack
	MOV EDX, 1								; load 1 for incrementing time
	XADD [XMilliTime], EDX 					; update time and load old time into EDX
	CMP [XMilliTime], EDX					; test for overflow in lower 32-bits
	POP EDX									; restore PROCESSOR_ID into EDX
	JLE Incro								; if overflow occured, increment upper-32bits
	JMP KiRunnerUpdate

Incro:
	LOCK INC DWORD [XMilliTime + 4]			; increment upper-32 bits

KiRunnerUpdate:

;-F-F-F-F-F-
;
; store the eip and save user-mode & kernel-mode stack-frame pointers for
; later reference. invoke the scheduler by storing the cpu-struct in edx.
;
extern Schedule
KiScheduleEntry:
	SHL EDX, 12								; get the offset of the cpu-struct
	ADD EDX, 0xc0000000 + 20 * 1024 * 1024	; load the address of cpu-struct

	MOV EBX, [EDX + 24] 					; load the current kernel-task
	MOV EDI, [EBX + 32] 					; cache the task's kernel-mode stack

	MOV ECX, [EBP]							; load the eip from which interrupt has occured
	MOV [EBX + 16], ECX 					; store eip into task->eip

	CMP DWORD [EBP + 4], 0x8				; test whether we came from a kernel/user context
	JE KiScheduleKernel						; we have seperate handlers for each case

	KiScheduleUser:
		MOV ECX, [EBP + 12] 				; load the user-mode stack pointer (from interrupt-frame)
		MOV ESI, [EBX + 28]					; cache the user-mode struct
		MOV [ESI + 4], ECX 					; save the user-mode pointer
	KiScheduleKernel:
		SUB EBP, 28							; come to the frame holding the saved registers
		MOV [EDI + 4], EBP					; save the kernel-mode stack-pointer

KiInvokeScheduler:
	PUSH EDX								; save EDX (to avoid changes)
	PUSH EDX								; pass cpu-argument
	CALL Schedule

;-F-F-F-F-F-
;
; executes after the core-scheduler runs and the next kernel-task is stored
; in the cpu-struct. calls task->run() for further execution of the system
; software.
;
KiScheduleExit:
	ADD ESP, 4								; go to previous stack-frame (before args)
	POP EDX									; restore the cpu-struct

	MOV EBX, [EDX + 24] 					; load the new-task
	MOV EDI, [EBX + 32] 					; cache the task's kernel-stack struct

	MOV ESP, [EDI + 4] 						; store the kernel-mode stack pointer
	PUSH EBX								; save the task-struct on stack
	ADD ESP, 4								; restore the stupid stack

	CMP DWORD [EBX + 20], 0					; test whether task->run() is 0
	JE KiDispatchStatus						; if 0, do default execution, else below
	PUSH EBX								; pass the task-arg
	CALL [EBX + 20]							; call task->run()

	KiDispatchStatus:
		BTR DWORD [EBX + 24], 0				; check the dispatch status (new/already executed)
		JC KiJumpNew						; if new, go to the new-task handler

	; NOTE:::: EBX (Runner) = ESP - 32

	CALL EOI								; ensure other interrupts are allowed (without overwriting edx)
	POP EBP
	POP EDI
	POP ESI
	POP EDX
	POP ECX
	POP EBX
	POP EAX
	IRET

;-F-F-F-F-
;
; invokes a newly-created task which does not have any register stack-frame.
;
KiJumpNew:
	BT DWORD [EBX + 24], 1					; test for kernel/user-mode dispatch
	JC KiJumpKernel

	KiJumpUser:
		; //TODO: Implement user-mode intr return
		PUSH DWORD 0x23
		PUSH DWORD [EBX + 32]
		PUSH DWORD (1 << 9) 				; push and eflags with IF=1
		PUSH DWORD 0x0 ; CS
		PUSH DWORD [EBX + 16]
		CALL EOI
		IRET

	KiJumpKernel:
		PUSH DWORD (1 << 1) | (1 << 9) 		; push an eflags with IF=1
		PUSH DWORD 0x8
		PUSH DWORD [EBX + 16]
		CALL EOI
		IRET

	; end-of-intr
	global EOI
	EOI:
		MOV EDX, [VAPICBase]
		MOV DWORD [EDX + 0xB0], 0
		RET
