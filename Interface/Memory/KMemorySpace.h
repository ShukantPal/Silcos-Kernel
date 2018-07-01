///
/// @file KMemorySpace.h
///
/// "Configuration" header to define location in kernel-memory for various
/// data-structures.
/// -------------------------------------------------------------------
/// This program is free software: you can redistribute it and/or modify
/// it under the terms of the GNU General Public License as published by
/// the Free Software Foundation, either version 3 of the License, or
/// (at your option) any later version.
///
/// This program is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
/// GNU General Public License for more details.
///
/// You should have received a copy of the GNU General Public License
/// along with this program.  If not, see <http://www.gnu.org/licenses/>
///
/// Copyright (C) 2017 - Shukant Pal
///
#ifndef MEMORY_KMEMORYSPACE_H
#define MEMORY_KMEMORYSPACE_H

#include "Address.h"
#include <TYPE.h>

#ifdef x86
	#define KERNEL_OFFSET 0xc0000000
	#define KARCHINFO (KERNEL_OFFSET + MB(10))
	#define KCONFIGINFO (KERNEL_OFFSET + MB(16))
	#define CONSOLE	(KERNEL_OFFSET + MB(1022))
	#define KCPUINFO (KERNEL_OFFSET + MB(20)) // Used in APIC, Processor, and APBOOT
	#define KPROCESS_TABLE (KERNEL_OFFSET + MB(64))
	#define KTHREAD_TABLE (KERNEL_OFFSET + MB(128))
	#define KDYNAMIC (KERNEL_OFFSET + MB(256))
	#define KFRAMEMAP (KERNEL_OFFSET + MB(768))
	#define PSTACKTOP (KERNEL_OFFSET)
	#define PSTACKSIZE (KB(8))

	#define MULTIBOOT_INTERFACE (KERNEL_OFFSET + MB(1022) + KB(4))

	#define PAGE 4096

	#define KDYNAMIC_LOWER (MB(32))
	#define KDYNAMIC_UPPER (MB(512))
	#define KPGSIZE (KB(4))
	#define KPGOFFSET 12

	#define PInit (KERNEL_OFFSET + MB(250))
	#define TInit (KERNEL_OFFSET + MB(100))

	#define KPROCESS_SPACE MB(64)
	#define KTHREAD_SPACE MB(128)

	#define AUTO_DAT	KB(32)
#endif

extern ADDRESS ArchBlock;
extern ADDRESS stcConfigBlock;

#endif
