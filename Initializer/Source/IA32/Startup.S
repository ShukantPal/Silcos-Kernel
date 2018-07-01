/**
 @file Startup.asm

 This file setups the environment for the initializer.

 --------------------------------------------------------------------
 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>

 Copyright (C) 2017 - Shukant Pal
 */

.section .MultibootHeader

.align 0x8
MultibootHeaderStart:
	.long 0xE85250D6
	.long 0
	.long 24
	.long 0x100000000 - 0xE85250D6 - 24
	.word 0
	.word 0
	.long 8
MultibootHeaderEnd:

.section .data

.type BootGDTPointer, @object
BootGDTPointer:
	.word 23
	.long BootGDT

BootGDT:
BootGDT_NullEntry:
	.long 0x00000000
	.long 0x00000000
BootGDT_KernelCode:
	.word 0xFFFF
	.word 0x0000
	.byte 0x00
	.byte 0x9A
	.byte 0xCF
	.byte 0x00
BootGDT_KernelData:
	.word 0xFFFF
	.word 0x0000
	.byte 0x00
	.byte 0x92
	.byte 0xCF
	.byte 0x00

.section .text

.global StartupEntry
.type StartupEntry, @function
StartupEntry:
	cli
	lgdt (BootGDTPointer)
	ljmp $0x08, $ResetSegmentSelectors

ResetSegmentSelectors:
	movw $0x10, %dx
	movw %dx, %ds
	movw %dx, %es
	movw %dx, %fs
	movw %dx, %gs
	movw %dx, %ss

SetupStack:
	mov $(InitorStack + 4096), %esp

.extern LoadKernel
Startup:
	PUSH %EBX
	PUSH %EAX
	CALL LoadKernel
	JMP .

.section .bss
	.align 4096
	.lcomm InitorStack, 4096
