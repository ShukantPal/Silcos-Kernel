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
struct _CONTEXT
{
	unsigned int OwnerId;// resource-management ID of the primary owner (not used yet!)
	unsigned int Flags;// generic flags (not used yet!)
	PAGE_TRANSALATOR HardwarePage;// arch-specific tables & flags
	unsigned int UsedBy;// no. of resource-holders using this
	Spinlock ContextLock;// lock for manipulating this context
} CONTEXT;

extern CONTEXT SystemCxt;//< Primary context for the system. Used for kernel
			 //< and during booting.

decl_c void SwitchContext(CONTEXT *pgContext);
decl_c void EnsureMapping(ADDRESS vaddr, PADDRESS paddr, CONTEXT *pgContext,
				unsigned long frFlags, PAGE_ATTRIBUTES pgAttr);
decl_c void EnsureUsability(ADDRESS address, CONTEXT *pgContext,
				unsigned long frFlags, PAGE_ATTRIBUTES pgAttr);
decl_c void EnsureFaulty(ADDRESS address, CONTEXT *cxt);
decl_c bool CheckUsability(ADDRESS address, CONTEXT *pgContext);
decl_c void EnsureAllUsable(unsigned long vaddr, unsigned long mapSize,
				unsigned long frFrames, CONTEXT *cxt,
				PAGE_ATTRIBUTES attr);
decl_c void EnsureAllMappings(unsigned long address, PADDRESS pAddress,
			unsigned long mapSize, CONTEXT *pgContext,
			PAGE_ATTRIBUTES pgAttr);
decl_c void EnsureAllFaulty(unsigned long vaddr, unsigned long mapSize,
			CONTEXT *cxt);
#endif/* Memory/Pager.h */
