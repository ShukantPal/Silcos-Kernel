;+=============================================================================
; File: APBoot.asm
;
; Summary: This file contains the initialization code for multi-processing systems. The application
; processors execute code on a pre-defined 64-KB aligned address in lower memory. This ensures
; that the initial IP is zero, and on the other hand the code segment (CS) can be directly read from a
; register.
;
; The total code and data, which is copied to the predefined address, is called the trampoline. It is also
; called the boot file here.
;
; NOTE: The bootstrap processor also uses the defaultBootGDT and the bsGDTPointer (InitRuntime.asm) to
; load a GDT before paging. This is because, although the processor is given control in PM-mode,
; the GDT-selectors cannot be trusted on. Thus, before doing any changes to the GDT here, be sure
; to check compatiblity in ../Boot/InitRuntime.asm.
;
; Symbols:
; APBoot - This marks the beginning of the boot file.
;
; APRealModeInit - This is the code, where the processor executes, just recieving a SIPI.
; 
; APMain32 - This is the code, where the processor gets control (in the trampoline)
; 
; APInvokeMain32 - This is the code, where the code jumps after enabling PM-mode. This will return
; to APMain32 at the end.
;
; APSetupRuntime - This is the code, not in the trampoline, which is executed after enabling paging
; to goto APMain, from which the hot-plug mechanism (runqueue) starts working.
;
; Copyright (C) 2017 - Shukant Pal
;=============================================================================+

global APBoot
global apSetupInfo
global APBootSequenceStart
global APBootSequenceEnd

extern InitPaging

SECTION .text

		GDT_ENTRY_SIZE 		equ 			8
		GDT_SIZE 					equ 			(GDT_ENTRY_SIZE * 3)
		GDT_POINTER_SIZE 	equ 			6

		STATUS_INIT 				equ			0x0002
		STATUS_BOOTING	 	equ			0x00BB
		STATUS_ERROR 			equ			0x00FF

		ADM_PADDR_TRAMPOLINE 								equ 	(589824) 		; 576 KB
		ADM_PADDR_APMAIN32  									equ 	(ADM_PADDR_TRAMPOLINE + APMain32 	- APBoot)
		ADM_PADDR_APSETUPRUNTIME 						equ 	(ADM_PADDR_TRAMPOLINE + APSetupRuntime - APBoot)
		ADM_PADDR_DEFAULT_BOOT_GDT 					equ 	(ADM_PADDR_TRAMPOLINE + defaultBootGDT - APBoot)
		ADM_PADDR_DEFAULT_BOOT_GDT_POINTER 	equ 	(ADM_PADDR_TRAMPOLINE + defaultBootGDTPointer - APBoot)

		KCPUINFO equ (0xC0000000 + (20 << 20))

		ALIGN 4
		;========================================================================================;
		;========================================================================================;
		; Symbol: APBoot
		;
		; Summary: This symbol represent the starting of the trampoline/boot file. It is used for getting relative offsets as
		; seen in the ADM_PADDR_... constants in the starting the code-file (asm). This is successful because it is sure
		; that the trampoline will be located at a 64-KB ALIGNED address in lower memory.
		;
		; @Version 1
		; @Since Circuit 2.03
		;========================================================================================;
		;========================================================================================;
		APBoot:
		APBootSequenceStart:

		;========================================================================================;
		;========================================================================================;
		; Symbol: APRealModeInit
		;
		; Summary: APRealModeInit is where the processor starts executed after receiving the SIPI sequence in the real
		; mode.
		;
		; @Version 1
		; @Since Circuit 2.03
		;========================================================================================;
		;========================================================================================;
		[BITS 16]
		APRealModeInit:
		CLI
		
		MOV AX, CS													; Load CS into AX
		MOV DS, AX													; Copy CS into DS
		
		MOV WORD [apBootStatusRegister - APBoot], STATUS_BOOTING

		LGDT [ADM_PADDR_DEFAULT_BOOT_GDT_POINTER & 0xFF]

		MOV DWORD EAX, CR0								; Load CR0
		OR  	DWORD EAX, 0x1								; Set PM-bit
		MOV DWORD CR0, EAX								; Enable Protected-Mode	
		JMP	DWORD 0x8:AP_MAIN32_INVOKER		; Invoke APMain32
		
		;========================================================================================;
		;========================================================================================;
		; Symbol: APMain32
		;
		; Summary: APMain32 is the executing point, from which higher-half/paging is enabled. It is located inside the
		; trampoline.
		;
		; @Version 1
		; @Since Circuit 2.03
		;========================================================================================;
		;========================================================================================;
		[BITS 32]
		ALIGN 4
		APMain32:
		MOV EDX, APSetupRuntime							; InitPaging code will return to address given in EDX

		MOV ECX, InitPaging										; Load InitPaging symbol (it is in higher-half)
		SUB 	ECX, 0xC0000000									; Load physical address of InitPaging
		JMP 	ECX														; goto InitPaging

		ALIGN 8
		apSetupInfo:
		;========================================================================================;
		;========================================================================================;
		; Symbol: defaultBootGDT
		;
		; Summary: defaultBootGDT contains the GDT for early-initialization, and is required for
		; entering PM-mode.
		;
		; It is a read-only field, and contains all required values already. It contains a NULL, 
		; KernelCode, and KernelData segments.
		;
		; @Version 1
		; @Since Circuit 2.03
		;========================================================================================;
		;========================================================================================;
		global defaultBootGDT
		defaultBootGDT:					; PROCESSOR_SETUP_INFO.DefaultBootGDT
		;========================================================================================;
		defaultBootGDTStart:									; defaultBootGDT (0)
		; GDT_ENTRY - NULL
			DQ 0x0000000000000000							; GDT_ENTRY	
		;========================================================================================;
		; GDT_ENTRY - KernelCode
			DW 	0xFFFF												; KernelCode.Limit	
			DW	0x0000												; KernelCode.BaseLow
			DB 	0x00													; KernelCode.BaseMiddle
			DB 	0x9A													; KernelCode.Access
			DB 	0xCF													; KernelCode.Granularity
			DB 	0x00													; KernelCode.BaseHigh
		;========================================================================================;
		; GDT_ENTRY - KernelData
			DW 	0xFFFF												; KernelData.Limit
			DW 	0x0000												; KernelData.BaseLow
			DB 	0x00													; KernelData.BaseMiddle
			DB 	0x92													; KernelData.Access
			DB 	0xCF													; KernelData.Granularity
			DB 	0x00													; KernelCode.BaseHigh
		;========================================================================================;
		defaultBootGDTEnd:										; defaultBootGDT + 23
		;========================================================================================;
		
		apBootStatusRegister: DW STATUS_INIT			; PROCESSOR_SETUP_INFO.BootStatusRegister	
		
		;========================================================================================;
		;========================================================================================;
		; Symbol: defaultBootGDTPointer
		;
		; Summary: defaultBootGDTPointer contains the GDT pointer for early-initialization, and 
		; is located at a fixed physical address (copyed there).	
		;========================================================================================;
		;========================================================================================;
		global defaultBootGDTPointer	
		defaultBootGDTPointer:								; PROCESSOR_SETUP_INFO.DefaultBootGDTPointer
			DW GDT_SIZE - 1										; defaultBootGDTPointer.Limit
			DD ADM_PADDR_DEFAULT_BOOT_GDT		; defaultBootGDTPointer.Base
		;========================================================================================;
		APBootSequenceEnd:
		
		AP_MAIN32_INVOKER equ (APInvokeMain32-0xC0000000)
		
		ALIGN 4
		[BITS 32]
		APInvokeMain32:
			MOV AX, 0x10											; Load data segment selector
			MOV DS, AX		
			MOV ES, AX
			MOV FS, AX
			MOV GS, AX
			MOV SS, AX
			JMP DWORD 0x8:ADM_PADDR_APMAIN32
				
		extern APMain
		extern VAPICBase
		APSetupRuntime:
			MOV DWORD EAX, [VAPICBase]
			MOV DWORD EAX, [EAX + 0x20]
			SHR EAX, 12												; SHR 24 to get APIC_ID and then SHL 12 to multiply by KPGSIZE
	
			MOV EBX, KCPUINFO
			ADD EBX, EAX
			MOV ESP, [EBX + 28]	

			CALL APMain

		;====================================================================;
		;====================================================================;
		; Function: APWaitForPermit
		;
		; Summary: This function is called before a AP starts setting up its runqueue and start
		; running the idler thread. It waits for the BSP to update the apPermitLock, or grant
		; the permit. This permit makes all APs wait until the KiClockRespond handler is mapped
		; to the 0x20 entry of IDT and the early-kernel timer to be discarded.
		;
		; @Version 1
		; @Since Circuit 2.03
		;====================================================================;
		;====================================================================;
		extern apPermitLock
		global APWaitForPermit
		APWaitForPermit:
			PUSH EDX													; Save EDX on stack
			apTryForPermitAgain:								; Loop until the permit is granted
			PAUSE														; Pause for looping
			MOV EDX, [apPermitLock]							; Load apPermitLock value (atomic)
			LFENCE														; Make sure the load is done!
			CMP EDX, 0												; Test lock value
			JNE apGotPermit										; If not locked, go back to APMain caller
			JE apTryForPermitAgain							; Else, try again
			
			apGotPermit:
			POP EDX													; Restore EDX from stack
			RET															; Go back to caller
		
		;====================================================================;
		;====================================================================;
		; Function: BSPGrantPermit
		;
		; Summary: This function is used for allowing the APs to continue to setup thier runqueue
		; by the BSP.
		;
		; @Version 1
		; @Since Circuit 2.03
		;====================================================================;
		;====================================================================;
		global BSPGrantPermit
		BSPGrantPermit:
			MOV DWORD [apPermitLock], 1					; Unlock
			SFENCE									; Make sure the value is written to RAM
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
			LOCK MOV DWORD [EAX], 0
			MFENCE
			POP EAX
			RET
