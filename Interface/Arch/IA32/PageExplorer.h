///
/// @file PageExplorer.h
/// @module KernelHost
///
/// Provides abstraction layers to the Pager to easily implement routines
/// that access and manipulate the page-level organization. Currently, only
/// PAE transversing is supported.
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

#ifndef KERNHOST_ARCH_IA32_PAGE_EXPLORER_H__
#define KERNHOST_ARCH_IA32_PAGE_EXPLORER_H__

#include <Memory/Address.h>
#include <Memory/Pager.h>

import_asm void SwitchPaging(unsigned int);
///
/// Provides an abstraction over IA32 page-level organization. It exposes
/// utilities that allow you to map page-tables, page-directories, and switch
/// between contexts to the Pager. It is not for use by platform-independent
/// code but does provide support for mapping huge-pages.
///
/// @version 1.0
/// @since Silcos 3.02
/// @author Shukant Pal
/// @see Pager
///
class PageExplorer final
{
public:

#define HUGE_PAGES(pdirIdx)((unsigned long)(pdirIdx) << 21)
#define NORM_PAGES(ptblIdx)((unsigned long)(ptblIdx) << 12)

#define PGTAB_BASE (GB(3) + HUGE_PAGES(507))
#define PGTAB_SIZE 512

#define PGDIR_BASE (GB(3) + HUGE_PAGES(511) + NORM_PAGES(508))
#define PGDIR_SIZE 512

	static unsigned long getDirectoryIndex(unsigned long virtualAddress) {
		return ((virtualAddress >> 21) % 512);
	}

	static unsigned long getTableIndex(unsigned long virtualAddress) {
		return ((virtualAddress >> 12) % 512);
	}

	///
	/// Returns the ptr to the first entry in the page-directory
	/// at the given offset in the PDPT.
	///
	/// @param dirOffset - offset of the directory whose entries are
	/// 		       required. This is calculated by (vaddr >> 30)
	/// @param frFlags - @deprecated
	/// @param cxt - the current context
	/// @return first entry in the required directory
	///
	static inline U64 *getDirectory(
			unsigned long dirOffset, unsigned long frFlags = 0,
			MemoryContext *cxt = null) {
		return ((U64*) PGDIR_BASE + (dirOffset << 9));
	}

	static inline U64 *pageTableForOffset(unsigned long globalTableOffset)
	{
		return ((U64*) PGTAB_BASE + (globalTableOffset << 9));
	}

	static U64 *getPageTable(unsigned long tableIndice,
			unsigned long allocFlags, MemoryContext *context = null);

	static U64 *getAllPageTables(unsigned long tableIndice,
			unsigned long tableCount, unsigned long allocFlags,
			MemoryContext *cxt = null);

	static U64 *remAllPagesTables(unsigned short tableIndice,
			unsigned short tableCount, unsigned long allocFlags,
			MemoryContext *cxt);

	static inline void setPage(U64 *ptabEnt, unsigned long allocFlags,
			PageAttributes attr)
	{
		*ptabEnt = (U64) KeFrameAllocate(0, ZONE_KERNEL, allocFlags)
				| attr;
	}

	static inline bool hasPageTable(U64 *dirEnt)
	{
		return ((*dirEnt & 1) && (*dirEnt >> 7 & 1));
	}

	///
	/// Sets the directory-entry given to point to a page-table. It
	/// doesn't check for huge-pages so be careful about that. If the
	/// PRESENT bit in the entry is set, it won't do anything.
	///
	/// @param dirEnt - the entry which should point to a page-table
	/// @param allocFlags - the flags by which pageframe will be allocated
	/// @return - whether new page-table was allocated
	///
	static inline U64 *setPageTable(U64 *dirEnt, unsigned long allocFlags)
	{
		if((~*dirEnt) & 1)
			*dirEnt = KiFrameEntrap(allocFlags) | 3;

		return (pageTableForOffset(dirEnt - (U64*) PGDIR_BASE));
	}

	///
	/// Sets the directory-entry given to point to a huge-page will the
	/// given attributes. It will unmap the previous page-table if found
	/// mapped. On the other hand, if a huge-page is already mapped it
	/// won't do anything.
	///
	/// @param dirEnt - page-directory entry which should point to a
	/// 		    huge-page.
	/// @param allocFlags - alloc-flags for 2mb page-frame
	/// @param attr - the set of attributes to map the huge-page
	///
	static void setHugePage(U64 *dirEnt, unsigned allocFlags,
			PageAttributes attr)
	{
		if((*dirEnt & 1))
		{
			if(*dirEnt >> 7 & 1)
				return;
			KiFrameFree(*dirEnt & 0x00000FFFFFFFF000);
		}

		*dirEnt = KeFrameAllocate(9, allocFlags, ZONE_KERNEL) |
				(attr) | (1 << 7);

		FlushTLB((unsigned long) pageTableForOffset(dirEnt - (U64*)
				PGDIR_BASE));
	}

private:
	PageExplorer();
};

#endif/* Arch/IA32/PageExplorer.h */
