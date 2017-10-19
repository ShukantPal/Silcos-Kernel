/*=++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * File: KMemoryManager.c
 *
 * Summary:
 * Kernel memory is allocated and deallocated through the KMemoryManager interface,
 * which is abstracted into the ZoneManager-Allocator through this implementation.
 *
 * Kernel memory is divided into only two zones -
 * ZONE_KOBJECT - Object-memory for the executive
 * ZONE_KMODULE - Module-space for executive-modules and kernel-drivers
 *
 * Copyright (C) 2017 - Shukant Pal
 *+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=*/

#define NS_KMEMORYMANAGER

#include <HAL/Processor.h>
#include <Memory/KMemoryManager.h>
#include <Memory/KMemorySpace.h>
#include <Util/Memory.h>
#include <KERNEL.h>

#include <Synch/Spinlock.h>

/** @Prefix kp - Kernel Page */
/** @Prefix kpt - Kernel Page Table */
/** @Prefix kdm - Kernel Dynamic Memory */

USHORT znObjectLiInfo[1 + PGVECTORS];
LINKED_LIST znObjectLi[PGVECTORS];

USHORT znModuleLiInfo[1 + PGVECTORS];
LINKED_LIST znModuleLi[PGVECTORS];

SPIN_LOCK kmLock;

// <KtObject> KMemoryManager.pageDomains </KtObject>
ZNINFO pageDomains[2] = {
	{// <KtObject> KMemoryManager.pageDomains[KMemoryManager::ZONE_KOBJECT] </KtObject>
		.MmManager = {
			.DescriptorSize = sizeof(KPAGE),
			.HighestOrder = MAXPGORDER,
			.ListInfo = &znObjectLiInfo[0],
			.BlockLists = &znObjectLi[0]
		},
		.ZnPref = 0,
		.Cache = { .ChMemoryOffset = PGCH_OFFSET }
	}, {// <KtObject> KMemoryManager.pageDomains[KMemoryManager::ZONE_KMODULE] </KtObject>
		.MmManager = {
			.DescriptorSize = sizeof(KPAGE),
			.HighestOrder = MAXPGORDER,
			.ListInfo = &znModuleLiInfo[0],
			.BlockLists = &znModuleLi[0]
		},
		.ZnPref = 0,
		.Cache = { .ChMemoryOffset = PGCH_OFFSET + sizeof(CHREG) }
	}
};// </KtObject>

ZNPREF pagePreferences = {
	.ZnPref = 0
};

ZNSYS pageAllocator = {
	.ZnPref = &pagePreferences,
	.ZnSet = &pageDomains[0],
	.ZnPrefCount = 1,
	.ZnPrefBase = 0
//	.ZnCacheRefill = 4
};

CHAR *msgSetupKMemoryManager = "Setting up KMemoryManager... ";

ADDRESS KiPagesAllocate(ULONG bOrder, ULONG prefZone, ULONG pgFlags){
	SpinLock(&kmLock);
	ZNINFO *znInfo = &pageDomains[prefZone];
	ULONG bInfo = (ULONG) ZnAllocateBlock(bOrder, 0, znInfo, pgFlags, &pageAllocator);
	SpinUnlock(&kmLock);
	return (KPGADDRESS(bInfo));
}

ULONG KiPagesFree(ADDRESS pgAddress){
	SpinLock(&kmLock);
	KPAGE *page = (KPAGE*) KPG_AT(pgAddress);
	page->HashCode = pgAddress;
	ZnFreeBlock((BDINFO*) page, &pageAllocator);
	SpinUnlock(&kmLock);
	return (1);
}

ADDRESS KiPagesExchange(ADDRESS pgAddress, ULONG *status, ULONG znFlags) {
	// @Prefix ea - Extension / Allocation
	ULONG eaInfo = (ULONG) ZnExchangeBlock((BDINFO *) KPG_AT(pgAddress), status, 0, znFlags,  &pageAllocator);
	return (KPGADDRESS(eaInfo));
}

VOID SetupKMemoryManager(VOID){
	DbgLine(msgSetupKMemoryManager);

	ADDRESS kptTable = KDYNAMIC;
	ULONG kptSize;

	ULONG kdmSize = (mmUsable >> 3) & (~(2 * (KPGSIZE << 5) - 1)); /* Align 64/128-KB */
	if(kdmSize > KDYNAMIC_UPPER)
		kdmSize = KDYNAMIC_UPPER;
	else if(kdmSize < KDYNAMIC_LOWER)
		kdmSize = KDYNAMIC_LOWER;

	kptSize = sizeof(KPAGE) * (kdmSize / KPGSIZE);
	ULONG kptFence = kptTable + kptSize;
	while(kptTable < kptFence) {
		EnsureUsability(kptTable, NULL, FLG_ATOMIC | FLG_NOCACHE | KF_NOINTR, KernelData);
		kptTable += KPGSIZE;
	}

	memset((VOID *) KDYNAMIC, 0, kptSize);
	kptTable = KDYNAMIC;

	pageDomains[0].MmSize = pageDomains[1].MmSize = kdmSize / (2 * KPGSIZE);
	pageDomains[0].MmReserved = pageDomains[1].MmReserved = kdmSize / (KPGSIZE << 5);
	ClnInsert((CLNODE *) &pageDomains[0], CLN_LAST, &pagePreferences.ZoneList);
	ClnInsert((CLNODE *) &pageDomains[1], CLN_LAST, &pagePreferences.ZoneList);
	pageDomains[0].MmManager.DescriptorTable = (UBYTE*) KDYNAMIC;
	pageDomains[1].MmManager.DescriptorTable = (UBYTE*) (KDYNAMIC + (kptSize / 2));

	ZNINFO *pgDomain = &pageDomains[0];
	for(ULONG pgDomainOffset=0; pgDomainOffset<2; pgDomainOffset++){
		ZnReverseMap(pgDomainOffset, pgDomain, &pageAllocator);
		++pgDomain;
	}

	ULONG kptPages = (kptSize % KPGSIZE) ? (1+ (kptSize - (kptSize % KPGSIZE)) / KPGSIZE) : (kptSize / KPGSIZE);
	ULONG kdmPages = kdmSize / KPGSIZE;

	while(kptPages < kdmPages) {
		ZnFreeBlock((BDINFO *) KPGOPAGE(kptPages), &pageAllocator);
		++kptPages;
	}

	pgDomain = &pageDomains[0];
	for(ULONG pgDomainOffset=0; pgDomainOffset<2; pgDomainOffset++){
		pgDomain->MmAllocated = 0;
		pgDomain = &pageDomains[pgDomainOffset];
	}
}
