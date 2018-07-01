/*
   @file Entry.S

   This is the absolute entry point of the KernelHost. The Initor module
   long jumps to the (@code HostEntry) label, from which arguments are
   copied off the stack, and then EarlyMain() is called.

    --------------------------------------------------------------------
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License;
   along with this program.  If not, see <http://www.gnu.org/licenses/>

   Copyright (C) 2017 - Shukant Pal
  */
.section .data

.extern defaultBootGDT
bsGDTPointer:
	.word 23
	.long defaultBootGDT - 0xC0000000

bootStack:
	.long 0x0

.section .text

/**
   Locates the AutoDat buffer allocated by the Initor module, finds
   the new stack, copies the argument into that, saves the stack
   frame in (@code bootStack), and then calls EarlyMain(), expecting
   it to return at (@code $CallMain).

   @version 1.0
   @since Silcos 3.05
   @author Shukant Pal
 */
.extern EarlyMain
.extern kernelPager
.global HostEntry
.type HostEntry, @function
HostEntry:
/* Switch to the new stack, copying the old one into EBP. */
	mov 12(%esp), %ebp
	add 8(%esp), %ebp
	xchg %esp, %ebp

/* Copy all the arguments in %eax and then push them onto
   the new stack. */
   	movl 32(%ebp), %eax
   	pushl %eax
	movl 28(%ebp), %eax
	pushl %eax
	movl 24(%ebp), %eax
	pushl %eax
	movl 20(%ebp), %eax
	pushl %eax
	movl 16(%ebp), %eax
	pushl %eax
	movl 12(%ebp), %ecx /* Used %ecx as it should be saved  */
	pushl %ecx
	movl 8(%ebp), %edx	/* Here, we used %edx to save it for later */
	pushl %edx
	movl 4(%ebp), %eax
	pushl %eax
/* Save the current stack pointer into bootStack,
   locating its physical address */
	movl $bootStack, %eax
	subl $0xC0000000, %eax
	addl %edx, %eax
	movl %esp, (%eax)
	addl $0xC0000000, (%eax)
	subl %edx, (%eax)
/* Push the only arguments required by EarlyMain() again, to prevent
   any data-corruption due to changes to local variables. */
	pushl %ecx
	pushl %edx
	pushl $CallMain
	call EarlyMain

.extern Main
.type CallMain, @function
CallMain:
	movl (bootStack), %esp
	call Main
	jmp .

.global APInvokeMain32
APInvokeMain32:
	movw 0x10, %ax
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %fs
	movw %ax, %gs
	movw %ax, %ss
	jmpl *(%ebx)

.section .bss

.global admMultibootTable
.global admMultibootTableStart
.global admMultibootTableEnd
admMultibootTableStart: 	.long 0	# void *admMultibootTable
admMultibootTableEnd: 		.long 0	# U32 admMultibootTableSize
admBoundaryStart:		.quad 0	# U32 admBoundaryStart (PADDR)
admBoundaryEnd:			.quad 0	# U32 admBoundaryEnd (PADDR)


