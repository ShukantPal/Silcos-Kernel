/**
 * Copyright (C) 2017 - Shukant Pal
 */

#include <ACPI/RSDP.h>
#include <ACPI/RSDT.h>
#include <ACPI/XSDT.h>
#include <Memory/Pager.h>
#include <Memory/KMemorySpace.h>
#include <KERNEL.h>
#include "../../../Interface/Utils/Memory.h"

using namespace ACPI;

RSDT *SystemRsdt;
XSDT *SystemXsdt;

static VirtualRSDT *loadedTables;

const char *sysRSDTNotFound = "RSDT NOT FOUND";

/**
 * Function: FormVirtualRSDT
 * Attributes: file-only (static)
 *
 * Summary:
 * This function establishes the virtual-RSDT inplace by mapping all the ACPI
 * tables into specific 4KB blocks. Virtual addresses of these tables are
 * placed into a global virtual-RSDT which is used later to search for other
 * tables by their name.
 *
 * Origin:
 * This was implemented to save time to search for a table. Re-mapping of all
 * tables was avoided by pre-mapping all of them at once.
 *
 * Author: Shukant Pal
 */
static void FormVirtualRSDT()
{
	loadedTables = (VirtualRSDT*) (stcConfigBlock * KPGSIZE);
	EnsureUsability((unsigned long) loadedTables, NULL, KF_NOINTR, KernelData);
	++(stcConfigBlock);

	loadedTables->physTable = SystemRsdt;
	loadedTables->matrixBase = stcConfigBlock * 4096;
	loadedTables->stdTableCount = SystemRsdt->entryCount();

	PADDRESS stdTablePAddr;
	for(unsigned long stdTableIndex = 0;
			stdTableIndex < loadedTables->stdTableCount;
			stdTableIndex++)
	{
		stdTablePAddr = (PADDRESS) SystemRsdt->ConfigurationTables[stdTableIndex];
		EnsureMapping(stcConfigBlock * 4096, stdTablePAddr & 0xFFFFF000, NULL, KF_NOINTR, KernelData);

		loadedTables->stdTableAddr[stdTableIndex] =
				(stcConfigBlock << KPGOFFSET) +
				(unsigned long) stdTablePAddr % KPGSIZE;

		++(stcConfigBlock);
	}
}

/**
 * Function: SetupRSDTHolder
 *
 * Summary:
 * Initializes all data structs that depend directly upon the RSDT. That also
 * includes the global virtual-RSDT.
 *
 * Version: 1.02
 * Since: Circuit 2.03
 * Author: Shukant Pal
 */
void SetupRSDTHolder()
{
	if(SystemRsdp->Revision == 0)
	{
		EnsureMapping(stcConfigBlock * 4096, SystemRsdp -> RsdtAddress & 0xfffff000, NULL, KF_NOINTR, 3);

		RSDT *Rsdt = (RSDT *) (stcConfigBlock * 4096 + (SystemRsdp -> RsdtAddress % 4096));
		SystemRsdt = Rsdt;
		++stcConfigBlock;

		FormVirtualRSDT();

		if(!VerifySdtChecksum(&(SystemRsdt -> RootHeader)))
		{
			DbgLine(sysRSDTNotFound);
			asm volatile("cli; hlt;");
		}
	}
}

/**
 * Function: SearchACPITableByName
 *
 * Summary:
 * Searches for a ACPI table in the virtual-RSDT by its name. As per specs,
 * it only compares first 4 bytes of the given string.
 *
 * Args:
 * const char *tblSign - signature of the table
 *
 * Changes:
 * # Uses the virtual-RSDT instead of mapping all tables every time
 *
 * Author: Shukant Pal
 */
void *SearchACPITableByName(const char *tblSign, SDTHeader *stdTable)
{
	unsigned long tableIdx;
	SDTHeader *thisTable;

	if(stdTable != null)
	{
		tableIdx = (((unsigned) stdTable & KPGOFFSET)
				- loadedTables->matrixBase) >> KPGOFFSET;
	}
	else
	{
		tableIdx = 0;
	}

	while(tableIdx < loadedTables->stdTableCount)
	{
		thisTable = (SDTHeader*) loadedTables->stdTableAddr[tableIdx];
		if(memcmp(thisTable, tblSign, 4))
			return (void*) thisTable;
		++(tableIdx);
	}

	return (null);
}
