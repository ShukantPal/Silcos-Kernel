/* @file Pager.c
 *
 * By default, the kernel uses PAE paging. This file implements the PAE paging
 * support. Currently, legacy paging is not support but will be in future
 * builds.
 * -------------------------------------------------------------------
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 *
 * Copyright (C) 2017 - Shukant Pal
 */
#include <IA32/PageExplorer.h>
#include <Memory/Address.h>
#include <Memory/Pager.h>
#include <Memory/KMemorySpace.h>
#include "../../../../../Interface/Utils/CtPrim.h"

import_asm void SwitchPaging(U32);
import_asm U64 PDPT[4];
import_asm U64 GlobalDirectory[512];
import_asm U64 GlobalTable[512];

CONTEXT SystemCxt;

///
/// Allows software to write to a specific physical address by mapping it in
/// virtual memory. This may be to write to an device register or for shared
/// memory region.
///
/// @param address - virtual address to be mapped
/// @param pAddress - Physical address of page-frame
/// @param cxt - mmu-context in which virtual-address resides. This is
/// 		required to map in an user-mode address. To do so in a
/// 		kernel-owned memory region, null also works.
/// @param frFlags - flags with which page-table is to be allocated, if required
/// @param pgAttr - attributes to be applied on the page while mapping
/// @version 1.0
/// @since Circuit 2.03
/// @author Shukant Pal
///
decl_c void EnsureMapping(ADDRESS vaddr, PADDRESS paddr, CONTEXT *cxt,
				unsigned long frFlags, PAGE_ATTRIBUTES pgAttr)
{
	U64 *pgTable = GetPageTable(vaddr / GB(1), (vaddr % GB(1)) / MB(2),
					frFlags, cxt);
	if(pgTable != NULL)
	{
		pgTable[(vaddr % MB(2)) / KB(4)] = paddr | pgAttr | 3;
		FlushTLB(vaddr);
	}
}

///
/// Typically allocates an page-frame with the specified flags and maps it to
/// the virtual address given. This is used in allocators to map allocated
/// regions in virtual memory and pass them to kernel code. User-mode software
/// may also request extensions in heaps, files, shm, etc.
///
/// @param address - virtual memory address to be allocated
/// @param pgContext - address space in which the virtual-address resides. This
/// 			can be null for kernel-owned memory.
/// @param frFlags - flags with which page-frame & page-table (if not already
/// 			present) are allocated
/// @param pgAttr - attributes with which virtual-address is mapped
/// @version 1
/// @since Circuit 2.03
/// @author Shukant Pal
///
decl_c void EnsureUsability(ADDRESS address, CONTEXT *pgContext,
				unsigned long frFlags, PAGE_ATTRIBUTES pgAttr)
{
	U64 *pgTable = GetPageTable(address / GB(1), (address % GB(1)) / MB(2),
					frFlags, pgContext);

	if(pgTable != NULL && !(pgTable[(address % MB(2)) / KB(4)] & 1))
	{
		PADDRESS pAddress = KeFrameAllocate(0, ZONE_KERNEL, frFlags);
		pgTable[(address % MB(2)) / KB(4)] = pAddress | pgAttr;
		FlushTLB(address);
	}
	else
	{
		DbgLine("Ensure use: already mapped");
		pgTable[(address % MB(2)) / KB(4)] |= pgAttr;
	}
}

///
/// Ensures that the virtual-address given is not mapped to any physical-address
/// so that a page-fault occurs on accessing it. The caller should ensure that
/// the physical-address mapped so was freed before losing it here.
///
/// @param address - virtual-address to be unmapped
/// @param cxt - context where this address resides; null if kernel-memory
/// @version 1.1
/// @since Silcos 2.05
/// @author Shukant Pal
///
decl_c void EnsureFaulty(ADDRESS address, CONTEXT *cxt)
{
	U64 *pgTable = GetPageTable(address / GB(1), (address % GB(1)) / MB(2),
					ATOMIC, cxt);

	if(pgTable != NULL)
	{
		pgTable[(address % MB(2)) / KB(4)] = 0;
		FlushTLB(address);
	}
}

///
/// Ensures that all pages in the range from vaddr to +mapSize are usable
/// and will not cause a page-fault by mapping them to page-frames allocated
/// from the page-frame allocator. Support for huge-pages has not be
/// implemented BUT WILL BE.
///
/// @version 1.0
/// @since Silcos 3.02
/// @author Shukant Pal
///
decl_c void EnsureAllUsable(unsigned long vaddr, unsigned long mapSize,
			unsigned long frFlags, CONTEXT *cxt,
			PAGE_ATTRIBUTES attr)
{
	U64 *pgTbl;
	U32 pgEnt = (vaddr % MB(2)) >> 12;
	S32 mapCounter = mapSize >> 12;

	while(mapCounter > 0)
	{
		pgTbl = GetPageTable(vaddr >> 30, (vaddr % GB(1)) >> 21,
				FLG_ATOMIC, cxt);
		while(mapCounter > 0 && pgEnt < 512)
		{
			pgTbl[pgEnt++] = KeFrameAllocate(0, ZONE_KERNEL,
						frFlags) | attr;
			FlushTLB(vaddr);
			vaddr += KB(4);
			--(mapCounter);
		}

		pgEnt = 0;
	}
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
decl_c void EnsureAllMappings(ADDRESS vaddr, PADDRESS paddr,
				unsigned long mapSize, CONTEXT *pgContext,
				PAGE_ATTRIBUTES pgAttr)
{
	U64 *pgTable;
	U32 pgEntry = (vaddr % MB(2)) >> 12;
	PADDRESS pMapper = paddr;
	S32 mapCounter = mapSize >> 12;
	
	while(mapCounter > 0)
	{
		pgTable = GetPageTable(vaddr >> 30, (vaddr % GB(1)) >> 21,
						FLG_ATOMIC, pgContext);
		while(mapCounter > 0 && pgEntry < 512)
		{
			pgTable[pgEntry++] = pMapper | pgAttr;
			pMapper += KB(4);
			FlushTLB(vaddr);
			vaddr += KB(4);
			--(mapCounter);
		}
		
		pgEntry = 0;
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
decl_c void EnsureAllFaulty(ADDRESS vaddr, unsigned long mapSize, CONTEXT *cxt)
{
	U64 *pgTbl;
	U32 tblIdx = (vaddr % MB(2)) >> 12;
	S32 mapCounter = mapSize >> 12;

	while(mapCounter > 0)
	{
		pgTbl = GetPageTable(vaddr >> 30, (vaddr % GB(1)) >> 21,
						FLG_ATOMIC, cxt);

		vaddr >>= 12;
		if((mapCounter + tblIdx) >= 512)
		{
			mapCounter -= 512 - tblIdx;

			while(tblIdx < 512)
			{
				pgTbl[tblIdx++] = 0;
				FlushTLB(vaddr << 12);
				++(vaddr);
				++(tblIdx);
			}

			vaddr <<= 12;
		}
		else
		{
			while(mapCounter--)
			{
				pgTbl[tblIdx++] = 0;
				FlushTLB(vaddr << 12);
				++(vaddr);
			}
			return;
		}
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
	U64 *pgTbl = GetPageTable(addr/GB(1), (addr%GB(1)) / MB(2),
					FLG_NONE, cxt);
	if(pgTbl)
		return (pgTbl[(addr % MB(2)) / KB(4)] & 1);
	else
		return (FALSE);
}

decl_c void EraseIdentityPage(){ FlushTLB(0); }

/* Requires attention (while building processes & scheduling). Note that
 * this re-writes is for reverse mapping.
 */
decl_c void SwitchContext(CONTEXT *Ctx)
{
	PDPT(Ctx)[3] = (U64) (((unsigned long) GlobalDirectory - 0xc0000000) | 1);

	GlobalTable[511] = (U64) (((unsigned long) GlobalDirectory - 0xc0000000) | 3);
	GlobalTable[510] = PDPT(Ctx)[2] | 2;
	GlobalTable[509] = PDPT(Ctx)[1] | 2;
	GlobalTable[508] = PDPT(Ctx)[0] | 2;

	GlobalDirectory[510] = PDPT(Ctx)[2] | 2;
	GlobalDirectory[509] = PDPT(Ctx)[1] | 2;
	GlobalDirectory[508] = PDPT(Ctx)[0] | 2;
	GlobalDirectory[507] = (U64) (((unsigned long) GlobalDirectory - 0xc0000000) | 3);

	SwitchPaging((unsigned long) &PDPT(Ctx) - 0xc0000000);	
}

