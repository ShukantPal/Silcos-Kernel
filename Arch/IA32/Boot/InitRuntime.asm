;+=============================================================================
; File: InitRuntime.asm
;
; Summary: This file contains the code, from where the kernel enters after the bootloader is done with its
; work. Knowledge of the multiboot specification will be highly helpful in understanding the goal of the code
; and how the ADM works. This file contains the compile-time ADM setup, which allocates memory from the
; memory-map.
;
; ADM setup is kindof intricate, but a very simple concept. ADM is used for seperating the kernel and
; its early-boot structures. Using the memory map to get a free region, without a stack or paging, is really
; tough. So, the kernel levers the multiboot2 module features. All static data is packed into a must-be-present
; module. The kernel will fail to boot without this module, as all data structures will not be allocable.
;
;
; NOTE: This file contains the C~ language as comments against some symbols instead of micro-explanations
; to make it easier to understand.
;
; (c~ IS INTERNAL TO THE __CIRCUIT_PLATFORM__ AND IS DEFINED AS A SEPERATED, COMMENT LANGUAGE)
;
; Symbols: //TODO
;
; Copyright (C) 2017 - Sukant Pal
;=============================================================================+

[BITS 32]

; USING APBOOT SERVICE
extern defaultBootGDT
extern defaultBootGDTPointer			; HAL/APBoot.asm (../HAL..)

GDT_ENTRY_SIZE 		equ 8									; sizeof(GDT_ENTRY)
GDT_SIZE 				equ 3 * GDT_ENTRY_SIZE 						; Total size of defaultBootGDT
KERNEL_OFFSET			equ 0xC0000000								; Memory::KMemorySpace::KERNEL_OFFSET
BS_PADDR_GDT_POINTER	equ (bsGDTPointer - KERNEL_OFFSET)				; bsGDTPointer physical address
BS_MULTIBOOT_INTERFACE	equ (0xC000000 + (20 << 20) + (472 << 12))		; Memory::KMemorySpace::MULTIBOOT_INTERFACE

ADM_MEMORY_SIZE		equ (16 << 12)								; 16 KB - ADM Size

SECTION .data
	global KernelStack_
	KernelStack_ equ (KernelStack + 8192)

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

	;=========================================================================;
	;=========================================================================;
	; Symbol: ADMSearchMultibootTag
	;
	; Summary: ADMSearchMultibootTag is used for searching a multiboot tag in the table given
	; by the (multiboot) bootloader.
	;
	; NOTE: This is boot-time function and is done before paging and after loading multiboot-table;
	;
	; Args:
	; ESI ~MULTIBOOT_TAG $curTag # Tag from which the search should start, or NULL if all
	; EDI ~U32 $tagType	# Type of tag required
	;
	; Returns: EDX
	; Uses: ECX
	; @Version 1
	; @Since Circuit 2.03
	;=========================================================================;
	;=========================================================================;
	global ADMSearchMultibootTag					; #include <Multiboot2.h, InitRuntime.asm>
																			;															;
	ADMSearchMultibootTag:						; <global> ADMSearchMultibootTag		;
	CMP ESI, 0								; if(ESI != NULL){					;
	JNE admSearchFromTag						; 	goto admPrepareTag;				;
	MOV ESI, [admMultibootTableStart - 0xC0000000]	;} else ESI = *admMultibootTableStart;	;
											;								;
	admPrepareTag:								; <local> admPrepareTag				;
	ADD ESI, 8								; 	ESI += sizeof(MULTIBOOT_TAG);		;
											;								;
	admSearchFromTag:							; <local> admSearchFromTag			;
	CMP ESI, [admMultibootTableEnd - 0xC0000000]		; while[ESI < *admMultibootTableEnd]	;
	JGE admNotifyNULL							; (defaultExit: admNotifyNULL){		;
	CMP [ESI], EDI								; 	if(ESI.Type == EDI)				;
	JE admNotifyTag							;		goto admNotifyTag; // RET indirect	;
	ADD ESI, [ESI + 4]							;   else{							;
	MOV ECX, ESI								;	ESI += ESI.Size				;
	AND ECX,  ~7								; 	ECX = EDI & 7					;
	CMP ECX, ESI								;  	if(ECX == EDI){ // EDI is 8-byte aligned;
	JE admSearchFromTag							;	continue;						;
	ADD ECX, 8								; 	ECX += 8;						;
	MOV ESI, ECX								;  	ESI = ECX; __dump(ECX)			;
	JMP admSearchFromTag						; }								;
											;								;
	admNotifyTag:								; <local> admNotifyTag				;
	JMP EDX									; return; // goto EDX; 1#Return Address	;
	admNotifyNULL:								; <local>admNotifyNULL				;
	MOV ESI, 0								; ESI = NULL; // curTag = NULL		;
	JMP EDX									; return;							;
	;=========================================================================;

SECTION .text

	global InitRuntime
	InitRuntime equ (InitRuntime_ )

	;=========================================================================;
	;=========================================================================;
	; Symbol: InitRuntime_
	;
	; Summary: InitRuntime_ is the entry point in the kernel, to which the bootloader jumps. It loads
	; the defaultBootGDT, but using the bsGDTPointer (not defaultBootGDTPointer). It then loads
	; the segment selectors and initializes the ADM. After that PAE paging is initialized and then
	; it goes to InitEntrance to setup thing further in pagination mode.
	;=========================================================================;
	;=========================================================================;
	; local InitRuntime_
	extern InitPaging
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
	InitEnvironment:
		MOV ESP, KernelStack_
		PUSH EAX
		PUSH EBX
		CALL LoadMultibootTags
		CALL Main
		INT 0x20
		JMP $

SECTION .bss
	global KernelStack
	KernelStack: RESB 8192
	
	global admMultibootTable
	global admMultibootTableStart
	global admMultibootTableEnd
	admMultibootTableStart: 	RESB 4										; VOID *admMultibootTable
	admMultibootTableEnd: 	RESB 4										; U32 admMultibootTableSize
	admBoundaryStart:		RESB 8										; U32 admBoundaryStart (PADDR)
	admBoundaryEnd:		RESB 8										; U32 admBoundaryEnd	(PADDR)
