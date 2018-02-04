/**
 * Copyright (C) 2017 - Shukant Pal
 */
#ifndef MEMORY_KMEMORYSPACE_H
#define MEMORY_KMEMORYSPACE_H

#include "Address.h"
#include <TYPE.h>

#ifdef x86
	#define KERNEL_OFFSET 0xc0000000
	#define KERNEL_BASE 0xC0100000
	#define KARCHINFO (KERNEL_OFFSET + MB(10))
	#define KCONFIGINFO (KERNEL_OFFSET + MB(15))
	#define KCPUINFO (KERNEL_OFFSET + MB(20)) // Used in APIC, Processor, and APBOOT
	#define KPROCESS_TABLE (KERNEL_OFFSET + MB(64))
	#define KTHREAD_TABLE (KERNEL_OFFSET + MB(128))
	#define KDYNAMIC (KERNEL_OFFSET + MB(256))
	#define KFRAMEMAP (KERNEL_OFFSET + MB(768))
	#define PSTACKTOP (KERNEL_OFFSET)
	#define PSTACKSIZE (KB(8))

	#define MULTIBOOT_INTERFACE (KERNEL_OFFSET + MB(1022) + KB(472))

	#define KDYNAMIC_LOWER (MB(32))
	#define KDYNAMIC_UPPER (MB(512))
	#define KPGSIZE (KB(4))
	#define KPGOFFSET 12

	#define PInit (KERNEL_OFFSET + MB(250))
	#define TInit (KERNEL_OFFSET + MB(100))

	#define KPROCESS_SPACE MB(64)
	#define KTHREAD_SPACE MB(128)
#endif

extern ADDRESS ArchBlock;
extern ADDRESS stcConfigBlock;

#endif
