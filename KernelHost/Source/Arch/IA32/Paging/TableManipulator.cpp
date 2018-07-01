/**
 * @file TableManipulator.cpp
 *
 * Implements low-level access to the page-tables & page-directories
 * for the IA32 platform.
 *
 * # PAE Page-Table Organization
 *
 * In PAE-mode, the kernel organizes the page-tables using three
 * inbuilt tables - PDPT, GlobalDirectory, and GlobalTable.
 *
 * ## PDPT
 *
 * The boot-time PDPT holds four directory entries - with only the
 * third usable, for kernel-owned memory.
 *
 * ## Global Directory
 *
 * The global directory holds the page-tables for kernel-owned memory
 * and its first entry is actually a 2MB huge page to map the
 * KernelHost and other boot-time modules.
 *
 * ## Global Table
 *
 * The 511th entry in the global-directory points to the global
 * table.
 *
 * Changes:
 *
 * 1. Since Silcos 3.02, all the page-tables are continous in memory which
 * subsequently reduces the overhead of finding the next page-table while
 * mapping memory which covers more than one page-table.
 *
 * 2. Since Silcos 3.02, all the PDPT's entries are filled before usage,
 * excluding the KERNEL_CONTEXT (as only the 3rd entry is used there) to
 * optimize away page-directory access.
 *
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
#include <Memory/KFrameManager.h>
#include <Memory/KMemorySpace.h>
#include <Utils/Memory.h>
#include <KERNEL.h>

#define pageTablePtr(dir, tbl)(KERNEL_OFFSET + \
		HUGE_PAGES(507 + (dir)) + NORM_PAGES(tbl))

/**
 * Returns a ptr in kernel-memory to the page-table with the given
 * parameters. If none exists, one is allocated using KiFrameEntrap
 * the passed pageframe-allocator flags.
 *
 * @param doff - page-directory offset of page-table required; it is
 * 	calculated by dividing the required address by one gigabyte.
 * @param tbloff - page-table offset in the page-directory; it is
 * 	calculated by dividing the modulus of required address in
 * 	one gigabyte by one huge-page.
 * @param frFlags - flags by which a new page-table should be
 *	allocated if required.
 * @param cxt - the context in which page-table is to be found. It
 * 	should be the current context otherwise wrong page-table will
 *	be given.
 * @return - pointer to a page-table of the given parameters; null is
 * 	returned if a huge-page exists at the given address.
 */
U64 *PageExplorer::getPageTable(unsigned long tableIndice,
		unsigned long allocFlags, MemoryContext *cxt)
{
	U64 *pdir = PageExplorer::getDirectory(tableIndice >> 9);
	unsigned long ptbl = (unsigned) pageTableForOffset(tableIndice);

	if(!pdir[tableIndice & 511] & 1)
	{
		pdir[tableIndice & 511] = (U64) KiFrameEntrap(allocFlags) | 3;
		FlushTLB((unsigned long) pdir);
		memsetf((void *) ptbl, 0, NORM_PAGES(1));
	}
	else if(pdir[tableIndice & 511] >> 7 & 1)
		return (null);

	return ((U64 *)(ptbl));
}

/**
 * Allocates all the page-tables in the given range so that the pager can
 * use the continous page-table entries without worrying about if they
 * are even mapped. It also checks whether any **pse** pages come in
 * b/w.
 *
 * This should not be used for regular mappings as **pse pages** are
 * very useful. This is particularly useful when the client wants to
 * directly map a virtual address block to physical address block when
 * both are not aligned modulo a HUGE_PAGE.
 *
 * @version 1.0
 * @since Silcos 3.02
 * @author Shukant Pal
 */
U64 *PageExplorer::getAllPageTables(unsigned long tableIndice,
		unsigned long tableCount, unsigned long allocFlags,
		MemoryContext *cxt)
{
	U64 *dirEnt = PageExplorer::getDirectory(tableIndice >> 9,
			allocFlags, cxt) + (tableIndice & 511);
	U64 *dirStart = dirEnt, *dirEnd = dirEnt + tableCount;
	U64 *tableBase =(U64*) pageTablePtr((tableIndice >> 9), (tableIndice & 511));
	U64 *tablePtr = tableBase;

	while(dirEnt < dirEnd)
	{
		if(!(*dirEnt) & 1)
		{
			*dirEnt = (U64) KiFrameEntrap(allocFlags) | 3;
			FlushTLB((unsigned long) dirEnt);
			memsetf(tablePtr, 0, PGTAB_SIZE * sizeof(U64));
		}
		else if((*dirEnt >> 7) & 1)
		{
			memsetf(tableBase, 0, NORM_PAGES(dirEnt - dirStart));
			return (null);
		}

		tablePtr += PGTAB_SIZE;
		++(dirEnt);
	}

	return (tableBase);
}


