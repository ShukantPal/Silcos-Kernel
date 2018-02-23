;+=============================================================================
; File: APBoot.asm
;
; Summary:
; Contains the boot-strap code for application processors. The first half runs
; in real-mode, loads the boot-GDT, and jumps to protected-mode. After that it
; loads page-tables & jumps to higher-half kernel. Finally, it calls APMain().
;
; All code & data from APBootSequenceStart to APBootSequenceEnd is copied from
; here to a pre-defined address.
;
; Symbols:
; APBoot - This marks the beginning of the boot file.
; APRealModeInit - This is the code, where the processor executes, just recieving a SIPI.
; APMain32 - This is the code, where the processor gets control (in the trampoline)
; APInvokeMain32 - This is the code, where the code jumps after enabling PM-mode. This will return
; 					to APMain32 at the end.
; APSetupRuntime - This is the code, not in the trampoline, which is executed after enabling paging
; 		to goto APMain, from which the hot-plug mechanism (runqueue) starts working.
;
; Copyright (C) 2017 - Shukant Pal
;=============================================================================+

global APBoot
global apSetupInfo
global APBootSequenceStart
global APBootSequenceEnd
global halLoadAddr
global halLoadPAddr

extern InitPaging

SECTION .text

GDT_ENTRY_SIZE			equ 8
GDT_SIZE 			equ (GDT_ENTRY_SIZE * 3)
GDT_POINTER_SIZE 		equ 6

STATUS_INIT 			equ	0x0002
STATUS_BOOTING	 		equ	0x00BB
STATUS_ERROR 			equ	0x00FF

PADDR_TRAMP 			equ 589824 ; KB(576) Trampoline is loaded here
PADDR_APMAIN32  		equ PADDR_TRAMP + APMain32-APBoot
PADDR_APSETUPRUNTIME 		equ PADDR_TRAMP + APSetupRuntime-APBoot
PADDR_DEFAULT_BOOT_GDT 		equ PADDR_TRAMP + defaultBootGDT-APBoot
PADDR_DEFAULT_BOOT_GDT_POINTER	equ PADDR_TRAMP + defaultBootGDTPointer-APBoot
PADDR_HAL_LOAD_ADDR		equ PADDR_TRAMP + halLoadAddr-APBoot
PADDR_HAL_LOAD_PADDR		equ PADDR_TRAMP + halLoadPAddr-APBoot

KCPUINFO 			equ (0xC0000000 + (20 << 20))

ALIGN 4
APBoot:
APBootSequenceStart:
halLoadAddr:			RESB 4
halLoadPAddr:			RESB 4

;=============================================================================;
; Symbol: APRealModeInit
;
; Summary:
; Entry-point for application processors, starting in real-mode. It turns off
; interrupts, loads the defaultBootGDT, and then enables protected-mode. It
; continues to APMain32 after finishing.
;
; Version: 1.5
; Since: Circuit 2.03
; Author: Shukant Pal
;=============================================================================;
[BITS 16]
APRealModeInit:
	CLI
	MOV AX, CS
	MOV DS, AX

	MOV WORD [apBootStatusRegister - APBoot], STATUS_BOOTING
	LGDT [PADDR_DEFAULT_BOOT_GDT_POINTER & 0xFF]

	MOV DWORD EAX, CR0
	OR DWORD EAX, 0x1		; Set PM-bit
	MOV DWORD CR0, EAX		; Enable Protected-Mode
	MOV DWORD EBX, PADDR_APMAIN32	; AP_MAIN32_INVOKER comes here

	JMP DWORD 0x8:AP_MAIN32_INVOKER
		
;========================================================================================;
; Symbol: APMain32
;
; Summary:
; APMain32Invoker jumps directly here; This code enables paging & jumps to the
; higher-half kernel at APSetupRuntime.
;
; Version: 1.8
; Since: Circuit 2.03
;========================================================================================;
[BITS 32]
ALIGN 4
APMain32:
	MOV EDX, APSetupRuntime		; InitPaging code will return to address given in EDX

	MOV ECX, InitPaging		; Load InitPaging symbol (it is in higher-half)
	SUB ECX, 0xC0000000		; Load physical address of InitPaging
	JMP ECX				; goto InitPaging

ALIGN 8
apSetupInfo:

global defaultBootGDT
defaultBootGDT:
defaultBootGDTStart:
; null-Entry
	DQ 0x0000000000000000
; KernelCode
	DW 	0xFFFF
	DW	0x0000
	DB 	0x00
	DB 	0x9A
	DB 	0xCF
	DB 	0x00
; KernelData
	DW 	0xFFFF
	DW 	0x0000
	DB 	0x00
	DB 	0x92
	DB 	0xCF
	DB 	0x00
defaultBootGDTEnd:
		
apBootStatusRegister: DW STATUS_INIT

global defaultBootGDTPointer
defaultBootGDTPointer:
	DW GDT_SIZE - 1
	DD PADDR_DEFAULT_BOOT_GDT

APBootSequenceEnd:
		
AP_MAIN32_INVOKER equ (APInvokeMain32 - 0xC0000000)
		
extern APInvokeMain32 ; moved to InitRuntime.asm (needed a fixed load address)
extern APMain
extern VAPICBase
APSetupRuntime:
	MOV DWORD EAX, [VAPICBase]
	MOV DWORD EAX, [EAX + 0x20]
	SHR EAX, 9
	
	MOV EBX, KCPUINFO
	ADD EBX, EAX
	MOV ESP, [EBX + 28]
	CALL APMain

;====================================================================;
; Function: APWaitForPermit
;
; Summary:
; All app. CPUs are required to wait until the BSP is finished with
; enabling the scheduler. This code waits until the BSP grants the
; apPermitLock to other CPUs to start scheduling.
;
; Version: 1.8
; Since: Circuit 2.03
;====================================================================;
extern apPermitLock
global APWaitForPermit
APWaitForPermit:
		PUSH EDX			; save EDX on stack as we are using it
		apTryForPermitAgain:		; loop until we get the permit
			PAUSE			; pause - as per spin-lock semantics
			PAUSE
			PAUSE
			MOV EDX, [apPermitLock]	; load the apPermitLock for checking
			CMP EDX, 0		; check if the lock was granted
			JNE apGotPermit		; goto apGotPermit if lock was granted
			JE apTryForPermitAgain	; otherwise, loop again
			
		apGotPermit:
			PUSH EAX
			PUSH ECX
			MOV ECX, [ESP+16]
			RDTSC
			MOV [ECX], EAX
			MOV [ECX+4], EDX
			POP ECX
			POP EAX
			POP EDX			; restore EDX as we are done with it
			RET
		
;====================================================================;
; Function: BSPGrantPermit
;
; Summary:
; This atomically frees the apGrantPermit
;
; Version: 1.8
; Since: Circuit 2.03
;====================================================================;
global BSPGrantPermit
BSPGrantPermit:
	MOV DWORD [apPermitLock], 1
	SFENCE
	RET

global SpinLock
SpinLock:
	PUSH EAX
	PUSH ECX
	MOV EAX, [ESP + 12]
	SpinLoop:
		PAUSE
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
