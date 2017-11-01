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

using namespace Memory;
using namespace Memory::Internal;

/** @Prefix kp - Kernel Page */
/** @Prefix kpt - Kernel Page Table */
/** @Prefix kdm - Kernel Dynamic Memory */

#define KMEM_ZONE_COUNT 2

SPIN_LOCK kmLock;

// Vectors (bit-fields) used by the (buddy) allocator
static USHORT allocatorVectors[(1 + PGVECTORS) * 2];

// Allocation lists used by the (buddy) allocator
static struct LinkedList allocatorLists[PGVECTORS * 2];

// You know that there are two page-zones - ZONE_KOBJECT &
// ZONE_KMODULE for objects & module-memory respectively.
static struct Zone pageZones[2];

// All zones are mix-able for kernel-memory & thus, there is
// only one zone-preference.
static struct ZonePreference pagePreferences[1];

// This is the 'core' allocation engine for the page-allocator, and
// more allocators may be used in the future (for memory-extensions)
static class ZoneAllocator coreEngine;

CHAR msgSetupKMemoryManager[] = "Setting up KMemoryManager... ";

ADDRESS KiPagesAllocate(ULONG bOrder, ULONG prefZone, ULONG pgFlags){
	SpinLock(&kmLock);
	struct Zone *znInfo = &pageZones[prefZone & 1];
	ULONG bInfo = (ULONG) coreEngine.allocateBlock(bOrder, 0, znInfo, pgFlags);
	SpinUnlock(&kmLock);
	return (KPGADDRESS(bInfo));
}

ULONG KiPagesFree(ADDRESS pgAddress){
	SpinLock(&kmLock);
	KPAGE *page = (KPAGE*) KPG_AT(pgAddress);
	page->HashCode = pgAddress;// Initial hash-code for the page
	coreEngine.freeBlock((struct BuddyBlock *) page);
	SpinUnlock(&kmLock);
	return (1);
}

ADDRESS KiPagesExchange(ADDRESS pgAddress, ULONG *status, ULONG znFlags) {
	// @Prefix ea - Extension / Allocation
	DbgLine("exchaning not impl");
	//ULONG eaInfo = (ULONG) ZnExchangeBlock((BDINFO *) KPG_AT(pgAddress), status, 0, znFlags,  &pageAllocator);
	//return (KPGADDRESS(eaInfo));
return (0);
}

ULONG db = 0;
ULONG _tes = 0;

VOID SetupKMemoryManager(VOID){
	DbgLine(msgSetupKMemoryManager);

	// SETUP KPAGE-TABLE MEMORY

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

	memsetf((VOID *) KDYNAMIC, 0, kptSize);
	kptTable = KDYNAMIC;

	// SETUP coreEngine & ZONES

	coreEngine.resetAllocator((struct BuddyBlock *) kptTable, pagePreferences, 1, pageZones, 2);
	ZoneAllocator::configureZones(sizeof(KPAGE), MAXPGORDER, allocatorVectors, allocatorLists, pageZones, 2);
	ZoneAllocator::configurePreference(pageZones, pagePreferences, 2);

	pageZones[0].memorySize = pageZones[1].memorySize = kdmSize / (2 * KPGSIZE);
	pageZones[0].memoryReserved = pageZones[1].memoryReserved = kdmSize / (KPGSIZE << 5);

	pageZones[0].memoryAllocator.setEntryTable((UBYTE*) KDYNAMIC);
	pageZones[1].memoryAllocator.setEntryTable((UBYTE*) (KDYNAMIC + kptSize / 2));

	ZoneAllocator::configureZoneMappings(pageZones, 2);

	// FREE ALL USABLE MEMORY

	// No. of pages taken by the kernel-kpage-table
	ULONG kptPages = (kptSize % KPGSIZE)
			? (1 + (kptSize - (kptSize % KPGSIZE)) / KPGSIZE) : (kptSize / KPGSIZE);
	// Total no. pages that are in kd-memory
	ULONG kdmPages = kdmSize / KPGSIZE;

	// Blocks from end of kpage-table to end-of-kernel-dynamic-memory
	// will be freed.
	DbgInt(kdmPages / 2);
	DbgLine("_free");
	KPAGE *usablePage = (KPAGE*) KPGOPAGE(kptPages);
	while(kptPages < kdmPages) {
		coreEngine.freeBlock((struct BuddyBlock *) usablePage);
		++(usablePage);
		++(kptPages);
		++_tes;
	}

	coreEngine.resetStatistics();
}
