///
/// @file Pager.h
///
/// Provides a generic interface for manipulating page-tables and mapping
/// various ranges of virtual-addresses to valid page-frames.
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

#ifndef HAL__MEMORY__PAGING_H__
#define HAL__MEMORY__PAGING_H__

#include <Types.h>
#include <Synch/Spinlock.h>
#include "KFrameManager.h"
#include <Utils/LinkedList.h>

/* Paging attribute macros */
#ifdef x86
	#include <IA32/PageTrans.h>
#endif

/* Unified attributes */
#define KernelData (PagePresent | PageReadWrite)

#ifdef NS_PMFLGS
	#define KERNEL_DATA (PRESENT | READ_WRITE)
#endif

///
/// Refers to the page-tables and directories used by the mmu to map virtual
/// to physical addresses. It has a "page-transalator" specific to each
/// architecture which holds the hardware tables.
///
/// @version 2.0
/// @since Circuit 2.01
/// @author Shukant Pal
///
typedef
struct MemoryContext
{
	unsigned int OwnerId;// resource-management ID of the primary owner (not used yet!)
	unsigned int Flags;// generic flags (not used yet!)
	PAGE_TRANSALATOR HardwarePage;// arch-specific tables & flags
	unsigned int UsedBy;// no. of resource-holders using this
	Spinlock ContextLock;// lock for manipulating this context

	MemoryContext() // @suppress("Class members should be properly initialized")
	{

	}
} CONTEXT;

extern MemoryContext kernelPager;//< Primary context for the system. Used for kernel
			 //< and during booting.

#define KERNEL_CONTEXT (&kernelPager)

class Pager
{
public:
	static void init(unsigned long retAddr, unsigned long envBase,
			U64 *globalTable, U64 *globalDirectory, U64 *bootPDPT);
	static void init2(U64 *globalDirectory, U64 *globalTable,
			unsigned long pdptPhys);
	static void switchSpace(MemoryContext *cxt);

	static void dispose(VirtAddr vadr);

	static void disposeAll(VirtAddr base, VirtAddr limit);

	static void map(VirtAddr vadr, PhysAddr padr,
			unsigned allocFlags, PageAttributes attr);
	static void mapHuge(VirtAddr vadr, PhysAddr padr,
			unsigned allocFlags, PageAttributes attr);
	static void mapAll(VirtAddr base, PhysAddr pbase, unsigned size,
			unsigned allocFlags, PageAttributes attr);
	static void mapAllHuge(VirtAddr base, PhysAddr pbase, unsigned size,
			unsigned allocFlags, PageAttributes attr);
	static void useAllSmall(VirtAddr base, VirtAddr limit,
			unsigned allocFlags, PageAttributes attr);
	static void useAllHuge(VirtAddr base, VirtAddr limit,
			unsigned allocFlags, PageAttributes attr);
	static void use(VirtAddr base, unsigned allocFlags,
			PageAttributes attr);
	static void useAll(VirtAddr base, VirtAddr limit,
			unsigned allocFlags, PageAttributes attr);

	static U64 *globalDirectory;
	static U64 *globalTable;
private:
	Pager() // @suppress("Class members should be properly initialized")
	{

	}
};

#endif/* Memory/Pager.h */
