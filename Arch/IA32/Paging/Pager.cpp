/**
 * File: Pager.c
 *
 * Summary: Here IA32-PAE paging is implemented alongwith the paging stubs.
 *
 * Functions:
 * EnsureMapping() - used for logical-to-physical address mapping
 * EnsureUsability() - used for allocating page-frame to logical address
 * EnsureFaulty() - unmaps a kernel-page from its physical address
 * CheckValidity() - used for check whether the address is mapped
 *
 * Changes:
 * # EnsureFaulty added for use by the slab-allocator to unmap a slab before
 * 	disposing it.
 *
 * Copyright (C) 2017 - Shukant Pal
 */

#include <IA32/PageExplorer.h>
#include <Memory/Address.h>
#include <Memory/Pager.h>
#include <Memory/KMemorySpace.h>
#include <Util/CtPrim.h>
#include <Util/IndexMap.h>

import_asm void SwitchPaging(U32);
import_asm U64 PDPT[4];
import_asm U64 GlobalDirectory[512];
import_asm U64 IdentityDirectory[512];
import_asm U64 GlobalTable[512];

CONTEXT SystemCxt;

/**
 * Function: EnsureMapping
 *
 * Summary: This function is for making a logical address usable or, in other words,
 * mapping a page to a specific (given) page-frame.
 *
 * Args:
 * ADDRESS address - Logical address of page
 * PADDRESS pAddress - Physical address of page-frame
 * CONTEXT *pgContext - Address space being used
 * unsigned long frFlags - Flags with which a page-table should be allocated (if needed)
 * PAGE_ATTRIBUTES pgAttr - Attributes for the page-mapping
 *
 * @Version 1
 * @Since Circuit 2.03
 */
void EnsureMapping(ADDRESS address, PADDRESS pAddress, CONTEXT *pgContext,
					unsigned long frFlags, PAGE_ATTRIBUTES pgAttr)
{
	U64 *pgTable = GetPageTable(address / GB(1), (address % GB(1)) / MB(2), frFlags, pgContext);
	if(pgTable != NULL)
	{
		pgTable[(address % MB(2)) / KB(4)] = pAddress | pgAttr | 3;
		FlushTLB(address);
	}
}
/**
 * Function: EnsureUsability
 *
 * Summary: This function is called when a logical address must be mapped, but to
 * any physical address. It is particularly useful for allocation of kernel-objects.
 *
 * If the page is already mapped then no re-mapping is done for saving the data.
 *
 * Args:
 * ADDRESS address - Logical address of page
 * CONTEXT *pgContext - Address space being used
 * unsigned long frFlags - KFrameManager flags to allocated page-frame and pagetable (if needed)
 * PAGE_ATTRIBUTES pgAttr - Attributes with which mapping should be done
 *
 * @Version 1
 * @Since Circuit 2.03
 */
void EnsureUsability(ADDRESS address, CONTEXT *pgContext, unsigned long frFlags, PAGE_ATTRIBUTES pgAttr)
{
	U64 *pgTable = GetPageTable(address / GB(1), (address % GB(1)) / MB(2), frFlags, pgContext);

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

void EnsureFaulty(ADDRESS address, CONTEXT *pgContext)
{
	U64 *pgTable = GetPageTable(address / GB(1), (address % GB(1)) / MB(2), ATOMIC, pgContext);

	if(pgTable != NULL)
	{
		pgTable[(address % MB(2)) / KB(4)] = 0;
	}
}

void EnsureAllMappings(ADDRESS address, PADDRESS pAddress, unsigned long mapSize,
					CONTEXT *pgContext, PAGE_ATTRIBUTES pgAttr)
{
	U64 *pgTable;
	U32 pgEntry = (address % MB(2)) >> 12;
	PADDRESS pMapper = pAddress;
	S32 mapCounter = mapSize >> 12;
	
	while(mapCounter >= 0)
	{
		pgTable = GetPageTable(address >> 30, (address % GB(1)) >> 21, FLG_ATOMIC, pgContext);
		
		while(mapCounter >= 0 && pgEntry < 512)
		{
			pgTable[pgEntry++] = pMapper | pgAttr;
			pMapper += KB(4);
			address += KB(4);
			FlushTLB(address);
			--(mapCounter);
		}
		
		pgEntry = 0;
	}
}

bool CheckUsability(ADDRESS Address, CONTEXT *pageContext)
{
	U64 *PageTable
		= GetPageTable(Address / (MB(2) * 512), (Address % (MB(2) * 512)) / MB(2), FLG_NONE, pageContext);

	if(PageTable)
	{
		return PageTable[(Address % MB(2)) / KB(4)] & 1;
	}
	else
	{
		return (FALSE);
	}
}

void EraseIdentityPage(){
	((U32 *) IdentityDirectory) [0] = 0;
	FlushTLB(0);
}

void SwitchContext(CONTEXT *Ctx)
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

ADDRESS AtStack(CONTEXT *P) {
	return (NULL);
}

void DtStack(CONTEXT *P, unsigned long StackBase) {
	return;
}

