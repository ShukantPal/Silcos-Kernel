/**
 * @file MemConfig.h
 *
 * Declares memory-specific macros to generalize operations based on the
 * memory configuration.
 *
 *  Created on: 12-Jun-2018
 *      Author: sukantdev
 */

#ifndef INITIALIZER_ARCHGENERIC_MEMCONFIG_H_
#define INITIALIZER_ARCHGENERIC_MEMCONFIG_H_

#include "Config.h"

#if ARCH == IA32
	#define ARCH_PAGESZ 0x1000
	#define ARCH_HPAGESZ 0x200000
	#define ARCH_PAGE_ALIGN(mem) mem = mem & ~0xFFF
	#define ARCH_HPAGE_ALIGN(mem) mem = mem & ~0x1FFFFF
	#define ALIGNED_PAGE(mem) (mem & ~0xFFF)
	#define ALIGNED_HPAGE(mem) (mem & ~0x1FFFFF)
	#define PAGES_SIZE(mem) (mem % 0x1000) ? \
					(ALIGNED_PAGE(mem) + 0x1000) : mem
	#define HUGE_PAGES_SIZE(mem) (mem % 0x1000000) ? \
					(ALIGNED_HPAGE(mem) + 0x1000000) : mem

	#define PREF_ENVBASE 0x1000000
	#define KERNEL_OFFSET 0xC0000000
#endif

#endif/* ArchGeneric/MemConfig.h */
