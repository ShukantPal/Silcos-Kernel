/**
 * Copyright (C) 2017 - Shukant Pal
 *
 * This is the interface for allocation kernel space pages, and is technically the
 * vmm of the kernel. It uses the zone manager to specifically divide the kernel
 * space into two zones - KObject and KModule.
 *
 * It conjoins with the PMM in respect with allocation policy and algorithms.
 */
#ifndef MEMORY_KMEMORYMANAGER_H
#define MEMORY_KMEMORYMANAGER_H

#include "Address.h"
#include "BuddyManager.h"
#include "ZoneManager.h"

#ifdef NAMESPACE_MAIN
	VOID SetupKMemoryManager(
		VOID
	);
#endif

#ifdef NS_KMEMORYMANAGER
	#define KPG_AT(pgAddress) (KDYNAMIC + sizeof(KPAGE) * (((unsigned long) pgAddress - KDYNAMIC) >> KPGOFFSET))
	#define KPGOPAGE(pgOffset) (KPAGE*) (KDYNAMIC + sizeof(KPAGE) * pgOffset)
	#define KPGADDRESS(kpPtr) (KDYNAMIC + (KPGSIZE * ((kpPtr - KDYNAMIC) / sizeof(KPAGE))))

	typedef
	struct _KPAGE {
		struct Memory::Internal::BuddyBlock BInfo;
		ULONG HashCode;
	} KPAGE;
#endif

#define ZONE_KOBJECT 0
#define ZONE_KMODULE 1

#define MAXPGORDER 5
#define PGVECTORS BDSYS_VECTORS(MAXPGORDER)

/**
 * KiPagesAllocate() - 
 *
 * Summary:
 * This function allocates (unmapped) pages in kernel memory. Mapping must be done
 * by the client. It provides two zones - ZONE_KOBJECT and ZONE_KMODULE. These
 * zones are mainly used for object allocation and module loading.
 *
 * Args:
 * pgOrder - Order of no. of pages required 
 * prefZone - Zone no. required (invalid no. becomes 1)
 * pgFlags - Zone-allocator flags
 *
 * Returns:
 * The address of the first page.
 *
 * @Version 1
 * @Since Circuit 2.03
 * @Author Shukant Pal
 */
ADDRESS KiPagesAllocate(
	ULONG pgOrder,
	ULONG prefZone,
	ULONG pgFlags
);

/**
 * KiPagesFree() - 
 *
 * Summary:
 * This function frees the pages returned by the user.
 *
 * Args:
 * pgAddress - Address of first page
 *
 * Returns:
 * 1
 *
 * @Version 1
 * @Since Circuit 2.03
 * @Author Shukant Pal
 */
ULONG KiPagesFree(
	ADDRESS pgAddress
);

/**
 * KiPagesExchange() -
 *
 * Summary:
 * Used for exchanging virtual address spaces. It has builtin parameters for some zone
 * manager arguments.
 *
 * Args:
 * pgAddress - Area for extension
 * status - Status info ptr
 * znFlags - Flags on allocation full-block
 *
 * @Version 1
 * @Since Circuit 2.03
 */
ADDRESS KiPagesExchange(
	ADDRESS pgAddress,
	ULONG *status,
	ULONG znFlags
);

#define KiPageAllocate(prefZone) KiPagesAllocate(0, prefZone, FLG_NONE)
#define KiPageFree(pgAddress) KiPagesFree(pgAddress)

#endif /* Memory/KMemoryManager.h */
