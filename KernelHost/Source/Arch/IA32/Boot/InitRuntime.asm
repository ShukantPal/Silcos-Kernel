; @file InitRuntime.asm
; This is where Silcos kernel development starts. Here the kernel enters from
; the InitRuntime symbols, an alias for InitRuntime_. This is placed in the
; higher-half kernel and turns interrupts, load the defaultBootGDT, enables
; paging, initializes the multiboot-info parser, and then finally calls
; Main() and if Main() returns ends into a infinite loop, although an scheduler
; interrupt will follow and kernel will execute there.
;
; --------------------------------------------------------------------
; This program is free software: you can redistribute it and/or modify
; it under the terms of the GNU General Public License as published by
; the Free Software Foundation, either version 3 of the License, or
; (at your option) any later version.
;
; This program is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
; GNU General Public License for more details.
;
; You should have received a copy of the GNU General Public License
; along with this program.  If not, see <http://www.gnu.org/licenses/>
;
; Copyright (C) 2017 - Shukant Pal
;

[BITS 32]

; HAL/APBoot.asm (../HAL..) (constants imported, not included just for sake)

GDT_ENTRY_SIZE 		equ 8						; sizeof(GDT_ENTRY)
GDT_SIZE 		equ 3 * GDT_ENTRY_SIZE 				; Total size of defaultBootGDT
KERNEL_OFFSET		equ 0xC0000000					; Memory::KMemorySpace::KERNEL_OFFSET
BS_PADDR_GDT_POINTER	equ (bsGDTPointer - KERNEL_OFFSET)		; bsGDTPointer physical address
BS_MULTIBOOT_INTERFACE	equ (0xC000000 + (20 << 20) + (472 << 12))	; Memory::KMemorySpace::MULTIBOOT_INTERFACE

ADM_MEMORY_SIZE		equ (16 << 12)					; 16 KB - ADM Size

SECTION .data

global KernelStack_
KernelStack_ equ (KernelStack + 8192)

bsGDTPointer:
	DW GDT_SIZE - 1
	DD (defaultBootGDT - KERNEL_OFFSET)

defaultBootGDT:					; CPU-Setup-Info.DefaultBootGDT
;=============================================================================;
defaultBootGDTStart:				; defaultBootGDT (0)
; GDT_ENTRY - NULL
	DQ 0x0000000000000000			; GDT_ENTRY
;=============================================================================;
; GDT_ENTRY - KernelCode
	DW 	0xFFFF				; KernelCode.Limit
	DW	0x0000				; KernelCode.BaseLow
	DB 	0x00				; KernelCode.BaseMiddle
	DB 	0x9A				; KernelCode.Access
	DB 	0xCF				; KernelCode.Granularity
	DB 	0x00				; KernelCode.BaseHigh
;=============================================================================;
; GDT_ENTRY - KernelData
	DW 	0xFFFF				; KernelData.Limit
	DW 	0x0000				; KernelData.BaseLow
	DB 	0x00				; KernelData.BaseMiddle
	DB 	0x92				; KernelData.Access
	DB 	0xCF				; KernelData.Granularity
	DB 	0x00				; KernelCode.BaseHigh
;=============================================================================;
defaultBootGDTEnd:				; defaultBootGDT + 23
;=============================================================================;

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
;
; Loads the GDT & segment selectors; then calls InitPaging to enable
; higher-half kernel. Then execution continues to InitEnvironment.
;
; Shukant Pal
;
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

global InitEntrance
InitEntrance equ InitEnvironment

global InitEnvironment
extern LoadMultibootTags
extern Main
extern ctorsStart		; Constructor-Section Start
extern ctorsEnd			; Constructor-Section End
InitEnvironment:
	MOV ESP, KernelStack_	; load kernel-stack for usage in C, C++ code
	PUSH EAX		; save multiboot register
	PUSH EBX		; save multiboot register

	CALL LoadMultibootTags	; load multiboot-tags
	;CALL InitAllObjects
	CALL Main		; initializes the system until scheduler is enabled
	JMP $			; if it comes here, we are seriously stupid

;=============================================================================;
; Calls all the global object constructors serially from the CTORS
; section. This is generally called after memory-management has initialized
; so that constructors can utilize it.
;
; @see (wiki) EarlyInitRace
; @version 1.0
; @since Silcos 3.05
; @author Shukant Pal
;=============================================================================;
global InitAllObjects
InitAllObjects:
	PUSHAD				; save all regs due to ctor's usage
	MOV EBX, ctorsStart		; value of addr of ptr of first ctor
	ADD EBX, 4
	JMP .incCtor
	.callCtor:
		PUSH EBX
		CALL [EBX]		; call each ctor using the ptr
		POP EBX
		ADD EBX, 4		; inc the ctor ptr
	.incCtor:
		CMP EBX, ctorsEnd	; test whether no more ctors are left
		JB .callCtor		; if not, continue
	POPAD				; restore regs
	RET

;; imported from HAL (not in mainline KernelHost)
ALIGN 4
[BITS 32]
global APInvokeMain32
APInvokeMain32:
	MOV AX, 0x10		; Load data segment selector
	MOV DS, AX
	MOV ES, AX
	MOV FS, AX
	MOV GS, AX
	MOV SS, AX
	JMP DWORD EBX

SECTION .bss

global KernelStack
KernelStack: RESB 8192	; 8-kb pre-boot stack

global admMultibootTable
global admMultibootTableStart
global admMultibootTableEnd
admMultibootTableStart: 	RESB 4	; void *admMultibootTable
admMultibootTableEnd: 		RESB 4	; U32 admMultibootTableSize
admBoundaryStart:		RESB 8	; U32 admBoundaryStart (PADDR)
admBoundaryEnd:			RESB 8	; U32 admBoundaryEnd (PADDR)
