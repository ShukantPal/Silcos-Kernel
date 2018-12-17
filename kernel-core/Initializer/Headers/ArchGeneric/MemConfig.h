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

/*
 * For each architecture, the following memconfig macros are defined:
 *
 * 1. <tt>ARCH_PAGESZ</tt> - The size of a (small) virtual page
 *
 * 2. <tt>ARCH_HPAGESZ</tt> - The size of a huge virtual page
 *
 * 3. <tt>ARCH_PAGE_ALIGN(mem)</tt> - Aligns the given mem address to the
 * 	page in which it lies. Simply erases the lower bits in mem.
 *
 * 4. <tt>ARCH_HPAGE_ALIGN(mem)</tt> - Aligns the given mem address to the
 *	huge page in which it lies. Simply erases the lower bits in mem.
 *
 * 5. <tt>ALIGNED_PAGE(mem)</tt> - Returns the value set in ARCH_PAGE_ALIGN
 *
 * 6. <tt>ALIGNED_HPAGE(mem)</tt> - Returns the value set in ARCH_HPAGE_ALIGN
 *
 * 7. <tt>PAGES_SIZE(mem)</tt> - Simply increases the given memory size (mem)
 *	so that it aligns with ARCH_PAGESZ
 *
 * 8. <tt>HUGE_PAGES_SIZE(mem)</tt> - Simply increases the given memory size
 *	(mem) so that it aligns with ARCH_HPAGESZ
 *
 * 9. <tt>PREF_ENVBASE</tt> - The preferred (sometimes only) kernel-env base in
 *	physical memory.
 *
 * 10. <tt>KERNEL_OFFSET</tt> - Minimum offset in virtual memory where
 *	kernel-env resides.
 */

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
