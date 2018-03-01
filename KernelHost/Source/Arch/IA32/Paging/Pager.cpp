///
/// @file Pager.cpp
///
/// Implements the paging-support for IA32 systems. Currently, only
/// PAE paging-mode is supported. Legacy support will be backported
/// in the future.
///
/// Changes:
/// a. Since Silcos 3.02, the KERNEL_CONTEXT macro is used in place of
///   kernel-owned memory (previously referred by a null ptr). This
///   was done to reduce the overhead of a mis-predicted branch.
///
/// b. Support for huge-pages has been implemented while using large
///   amounts of memory.
///
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

#include <IA32/PageExplorer.h>
#include <Memory/Address.h>
#include <Memory/Pager.h>
#include <Memory/KMemorySpace.h>
#include <Utils/Memory.h>

import_asm void SwitchPaging(U32);
import_asm U64 PDPT[4];
import_asm U64 GlobalDirectory[512];
import_asm U64 GlobalTable[512];

#define HUGE_PAGE MB(2)
#define DIRC_SIZE GB(1)

MemoryContext kernelPager;

///
/// Gives the number of page-tables that span the size given. Technically,
/// it gives the next multiple of HUGE_PAGE.
///
/// @param mapSize - amount of memory
///
static inline unsigned long getPageTableScope(unsigned long mapSize)
{
	if(mapSize & 0x1FFFFF)
		mapSize += (~mapSize & 0x1FFFFF) + 1;

	return (mapSize >> 21);
}

///
/// Switches to the given address-space and maps all required recursive
/// mapping pages.
///
/// @param ncxt - new address-space context object
/// @version 1.0
/// @since Circuit 1.02
/// @author Shukant Pal
///
void Pager::switchSpace(MemoryContext *ncxt)
{
	PDPT(ncxt)[3] = (U64)(((unsigned long) GlobalDirectory - 0xc0000000) | 1);

	GlobalTable[511] = (U64) (((unsigned long) GlobalDirectory - 0xc0000000) | 3);
	GlobalTable[510] = PDPT(ncxt)[2] | 2;
	GlobalTable[509] = PDPT(ncxt)[1] | 2;
	GlobalTable[508] = PDPT(ncxt)[0] | 2;

	GlobalDirectory[510] = (U64)(((unsigned long) GlobalDirectory - 0xc0000000) | 3);
	GlobalDirectory[509] = PDPT(ncxt)[2] | 2;
	GlobalDirectory[508] = PDPT(ncxt)[1] | 2;
	GlobalDirectory[507] = PDPT(ncxt)[0] | 2;

	// Implement physical-address locating. Right now, not required as
	// user-space doesn't exist.
	SwitchPaging((unsigned long) &PDPT(ncxt) - 0xc0000000);
}

///
/// Ensures that the virtual-address given is not mapped to any physical-address
/// so that a page-fault occurs on accessing it. The caller should ensure that
/// the physical-address mapped so was freed before losing it here.
///
/// @param address - virtual-address to be unmapped
/// @param cxt - context where this address resides
/// @version 1.1
/// @since Silcos 2.05
/// @author Shukant Pal
///
void Pager::dispose(VirtAddr address)
{
	U64 *pgTable = PageExplorer::getPageTable(address >> 21, FLG_ATOMIC);

	if(pgTable != NULL)
	{
		pgTable[(address % MB(2)) / KB(4)] = 0;
		FlushTLB(address);
	}
}

///
/// Ensures that all pages in the the range vaddr to +mapSize is unmapped
/// an accessing any address in it causes a page-fault. Support for huge pages
/// has not been yet given.
///
/// @param vaddr - virtual-address base
/// @param mapSize - bytes to unmap, should be multiple of page-size otherwise
/// 			a (huge/small) page can be left out.
/// @author Shukant Pal
///
void Pager::disposeAll(VirtAddr base, VirtAddr limit)
{
	U64 *dirEnt = PageExplorer::getDirectory(base >> 30);
	U64 *ptabEnt = PageExplorer::getPageTable(base >> 21, FLG_ATOMIC)
			+ ((base & 0x1FFFFF) >> 12);
	unsigned page = base >> 12, pagel = limit >> 12;

	while(page < pagel)
	{
		if(PageExplorer::hasPageTable(dirEnt))
		{
			do {
				*(ptabEnt++) = 0;
			} while((page & ~511) != 0 && (page < pagel));
		}
		else
		{
			*(dirEnt++) = 0;
			page = (page & ~0x1FFFFF) + HUGE_PAGE;
			ptabEnt = PageExplorer::getPageTable(base >> 21, FLG_ATOMIC);
		}
	}
}

///
/// Allows software to write to a specific physical address by mapping it in
/// virtual memory. This may be to write to an device register or for shared
/// memory region.
///
/// @param vadr - address in the page to be mapped
/// @param padr - physical address held by page-frame to written by software
/// @param frFlags - flags to allocate page-table if required
/// @param pgAttr - mapping attributes
/// @version 2.1
/// @since Silcos 3.02
/// @author Shukant Pal
///
void Pager::map(VirtAddr vadr, PhysAddr padr, unsigned frFlags,
		PageAttributes pgAttr)
{
	U64 *pgTable = PageExplorer::getPageTable(vadr >> 21, frFlags);
	if(pgTable != NULL)
	{
		pgTable[(vadr & 0x1FFFFF) >> 12] = padr | pgAttr | 3;
		FlushTLB(vadr);
	}
}

///
/// Ensures that all the pages in the range [base, limit) are mapped to
/// exclusively allocated page-frames and can be used for storing data. Any
/// page-frame already mapped in this range will be overwritten, and hence,
/// a memory-leak may occur if it wasn't properly mapped.
///
/// The lower & upper limits given are expected to be page-aligned otherwise
/// incomplete mapping may occur.
///
/// @param base - lower-bound of addresses to map, with 4K pages
/// @param limit - upper-bound of addresses to map, with 4K pages
/// @param allocFlags - the allocation flags for the page-frames and
/// 			any page-table
/// @param attr - the attributes by which page-frames are mapped
/// @version 1.0
/// @since Silcos 3.02
/// @author Shukant Pal
///
void Pager::useAllSmall(VirtAddr base, VirtAddr limit, unsigned allocFlags,
		PageAttributes attr)
{
	U64 *pte = PageExplorer::getAllPageTables(base >> 21,
			getPageTableScope(limit - base + (base % KPGSIZE)),
			allocFlags) + ((base & 0x1FFFFF) >> 12);
	U64 *ptlimit = pte + ((limit - base) >> 12);

	while(pte < ptlimit)
	{
		PageExplorer::setPage(pte, allocFlags, attr);
		FlushTLB(base);

		++(base); ++(pte);
	}
}

///
/// Ensures that all the huge-pages in the range [base, limit) are mapped to
/// exclusively allocated page-frames and can be used for storing data. Note
/// that if any huge-page is already present in this range, then it will not
/// be replaced and the client may overwrite data. Therefore, the given range
/// should be owned by the caller.
///
/// @param base - lower-bound of range which is to be mapped, with huge-pages
/// @param limit - upper-bound of range which is to be mapped, with huge-pages
/// @param allocFlags - flags for allocating huge-pages on the way
/// @param attr - attributes for mapping the huge-pages
/// @version 1.0
/// @since Silcos 3.02
/// @author Shukant Pal
///
void Pager::useAllHuge(VirtAddr base, VirtAddr limit,
		unsigned allocFlags, PageAttributes attr)
{
	if(base & 0x1FFFFF)
		base += ~(base & 0x1FFFFF) + 1;

	limit &= ~(0x1FFFFF);

	U64 *pde = PageExplorer::getDirectory(base >> 30, allocFlags)
			+ ((base % DIRC_SIZE) >> 21);
	U64 *pdlimit = pde + getPageTableScope(limit - base);

	base >>= 21;
	while(pde++ < pdlimit)
	{
		PageExplorer::setHugePage(pde, allocFlags, attr);
		FlushTLB(base << 21);
		++(base);
	}
}

///
/// Ensures that the page holding the given address is usable by
/// mapping it. Note that the address given need not be page-aligned.
///
/// @version 2.1
/// @since Silcos 3.02
/// @author Shukant Pal
///
void Pager::use(VirtAddr vadr, unsigned allocFlags,
		PageAttributes pgAttr)
{
	U64 *pgTable = PageExplorer::getPageTable(vadr >> 21, allocFlags);

	if(pgTable != NULL && !(pgTable[(vadr % MB(2)) / KB(4)] & 1))
	{
		PhysAddr pAddress = KeFrameAllocate(0, ZONE_KERNEL, allocFlags);
		pgTable[(vadr % MB(2)) / KB(4)] = pAddress | pgAttr;
		FlushTLB(vadr);
	}
	else
	{
		DbgLine("Ensure use: already mapped");
		pgTable[(vadr % MB(2)) / KB(4)] |= pgAttr;
	}
}

///
/// Ensures that the address in the range [vBase, vBase + mapSize) are usable
/// by mapping them. It optimizes TLB usage by using "huge" pages whereever
/// possible.
///
/// @version 1.0
/// @since Silcos 3.02
/// @author Shukant Pal
///
void Pager::useAll(VirtAddr vBase, VirtAddr vLimit,
			unsigned allocFlags, PageAttributes attr)
{
	unsigned hBase = vBase, hLimit = (vLimit) & (~0x1FFFFF);

	if(hBase & 0x1FFFFF)
	{
		hBase += (~hBase & 0x1FFFFF) + 1;
	}

	if(hLimit > vBase)
	{
		if(vBase != hBase)
			Pager::useAllSmall(vBase, hBase, allocFlags, attr);
	}
	else
	{
		Pager::useAllSmall(vBase, vLimit, allocFlags, attr);
		return;
	}

	Pager::useAllHuge(hBase, hLimit, allocFlags, attr);

	if(vLimit > hLimit)
		Pager::useAllSmall(hLimit, vLimit, allocFlags, attr);
}

///
/// Maps the virtual-addresses starting from vaddr to the physical-addresses
/// starting from paddr. It should be used when mapping *multi-page* blocks
/// to memory. It assumes that *page-size extension* is not being used in
/// the area being mapped.
///
/// Support for PSE will be implemented.
///
/// @param vaddr - virtual-address base from which mapping starts
/// @param paddr - physical-address base from which it is mapped
/// @param mapSize - no. of bytes to map in memory; should be multiple of
/// 			page-size, otherwise one page may be lost
/// @version 1.0
/// @since Silcos 2.05
/// @author Shukant Pal
///
void Pager::mapAll(VirtAddr vaddr, PhysAddr paddr,
		unsigned mapSize, unsigned allocFlags,
		PageAttributes pgAttr)
{
	U64 *pgTbl = PageExplorer::getAllPageTables(vaddr >> 21,
			getPageTableScope(mapSize), FLG_ATOMIC) +
					(vaddr % HUGE_PAGE) / 4096;
	paddr >>= 12;
	vaddr >>= 12;
	unsigned long pmax = (unsigned long) paddr + (mapSize >> 12);

	while(paddr < pmax)
	{
		*(pgTbl++) = (paddr << 12) | pgAttr;
		FlushTLB(vaddr << 12);
		++(paddr); ++(vaddr);
	}
}

/*
 * Checks whether the given address can be used without causing a
 * page-fault.
 *
 * @param addr - virt-addr to check
 * @param cxt - context being used
 * @author Shukant Pal
 */
decl_c bool CheckUsability(ADDRESS addr, CONTEXT *cxt)
{
	U64 *pgTbl = PageExplorer::getPageTable(addr/GB(1), FLG_NONE);
	if(pgTbl)
		return (pgTbl[(addr % MB(2)) / KB(4)] & 1);
	else
		return (FALSE);
}

decl_c void EraseIdentityPage(){ FlushTLB(0); }

