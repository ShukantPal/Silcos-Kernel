/**
   @file Load.S
   -------------------------------------------------------------------
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
.section .text

.global ExecuteLGDT
.type ExecuteLGDT, @function
ExecuteLGDT:
	pushl %eax
	mov 8(%esp), %eax
	lgdtl (%eax)
	mov $0x10, %ax
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %fs
	mov %ax, %gs
	mov %ax, %ss
	popl %eax
	ljmp $0x8, $Exit

Exit:
	ret

.global ExecuteLIDT
.type ExecuteLIDT, @function
ExecuteLIDT:
	pushl %eax
	movl 8(%esp), %eax
	lidt (%eax)
	sti
	pop %eax
	ret

.global ExecuteLTR
.type ExecuteLTR, @function
ExecuteLTR:
	pushl %eax
	movw $0x28, %ax
	ltr %ax
	popl %eax
	ret
