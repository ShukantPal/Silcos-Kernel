[BITS 32]

; USING APBOOT SERVICE
extern defaultBootGDT
extern defaultBootGDTPointer													; HAL/APBoot.asm (../HAL..)

GDT_ENTRY_SIZE 		equ 8														; sizeof(GDT_ENTRY)
GDT_SIZE 				equ 3 * GDT_ENTRY_SIZE 									; Total size of defaultBootGDT
KERNEL_OFFSET			equ 0xC0000000											; Memory::KMemorySpace::KERNEL_OFFSET
BS_PADDR_GDT_POINTER	equ (bsGDTPointer - KERNEL_OFFSET)						; bsGDTPointer physical address
BS_MULTIBOOT_INTERFACE	equ (0xC000000 + (20 << 20) + (472 << 12))				; Memory::KMemorySpace::MULTIBOOT_INTERFACE

ADM_MEMORY_SIZE		equ (16 << 12)												; 16 KB - ADM Size

SECTION .data
	global KernelStack_
	KernelStack_ equ (KernelStack + 4096)

	bsGDTPointer:
		DW GDT_SIZE - 1
		DD (defaultBootGDT - KERNEL_OFFSET)	

SECTION .BsTransferCtl

	ALIGN 0x8
	MultibootHeaderStart:
		DD 0xE85250D6
		DD 0
		DD (MultibootHeaderEnd - MultibootHeaderStart)
		DD 0x100000000 - (0xE85250D6 + (MultibootHeaderEnd - MultibootHeaderStart))
		DW 0
		DW 0
		DD 8
	MultibootHeaderEnd:

SECTION .text

	global InitRuntime
	InitRuntime equ (InitRuntime_ )
	extern InitPaging
	;/**
	; * Symbol: InitRuntime_
	; * Attributes: no-stack, multiboot, init, asm
	; *
	; * Summary:
	; * Loads the GDT & segment selectors; then calls InitPaging to enable
	; * higher-half kernel. Then execution continues to InitEnvironment.
	; *
	; * Author: Shukant Pal
	; */
	InitRuntime_:
		CLI
		LGDT [BS_PADDR_GDT_POINTER]
		JMP 0x8:(__SYM_farjump - 0xC0000000)
	__SYM_farjump:
	
		MOV DX, 0x10
		MOV DS, DX
		MOV ES, DX
		MOV FS, DX
		MOV GS, DX
		MOV SS, DX

		MOV [admMultibootTableStart - KERNEL_OFFSET], EBX
		MOV EDX, [EBX]
		ADD EDX, EBX
		MOV [admMultibootTableEnd - KERNEL_OFFSET], EDX
		
	__SYM_pager:
		MOV EDX, InitEntrance
		JMP InitPaging

	; ---------------------------

	global InitEntrance
	InitEntrance equ InitEnvironment

	; ---------------------------

	global InitEnvironment
	extern LoadMultibootTags
	extern Main
	extern ctorsStart						; Constructor-Section Start
	extern ctorsEnd							; Constructor-Section End
	InitEnvironment:
		MOV ESP, KernelStack_				; load kernel-stack for usage in C, C++ code
		PUSH EAX							; save multiboot register
		PUSH EBX							; save multiboot register
		MOV EBX, ctorsStart					; load ctor-pointer array
		JMP InitializeGlobalObjects			; call all ctors

		CallConstructor:
		CALL [EBX]							; call ctor
		ADD EBX, 4							; goto next ctor

		InitializeGlobalObjects:
		CMP EBX, ctorsEnd					; test if current ctor is the last one
		JB CallConstructor					; if not, continue (in loop)

		CALL LoadMultibootTags				; load multiboot-tags
		CALL Main							; initializes the system until scheduler is enabled
		INT 0x20							; if Main() returns, call the scheduler-tick
		JMP $								; if it comes here, we are seriously stupid

SECTION .bss
	global KernelStack
	KernelStack: RESB 4096					; +4-kb pre-boot stack
	
	global admMultibootTable
	global admMultibootTableStart
	global admMultibootTableEnd
	admMultibootTableStart: 		RESB 4										; void *admMultibootTable
	admMultibootTableEnd: 			RESB 4										; U32 admMultibootTableSize
	admBoundaryStart:				RESB 8										; U32 admBoundaryStart (PADDR)
	admBoundaryEnd:					RESB 8										; U32 admBoundaryEnd	(PADDR)
