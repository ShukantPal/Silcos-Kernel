/**
 * File: KFrameManager.c
 *
 * Summary: This file contains the code to implement the PMA, and interfaces with the zone allocator
 * to abstract its algorithm.
 *
 * Each NUMA domain is divided into four zones - DMA, KERNEL32, KERNELn, CODEn, and DATAn. There are three
 * zonal preferences - DMA, KERNEL32 (32-bit platform) and NORMAL. DMA always extends till the 16th
 * megabyte. KERNEL32 is used for 32-bit devices and extends till the 4th GB (when RAM > 4-GB) and then
 * CODEn, DATAn cover the rest. On UMA systems, the kernel "may" divide the zones into seperate virtual
 * memory-nodes from which allocation is seperately done.
 *
 * Functions:
 * KeFrameAllocate() - This function interfaces with the zone allocator for pageframe allocation.
 * KeFrameFree() - This function interfaces with the zone allocator for pageframe deallocation.
 * TypifyMRegion() - Used for declaring pageframe types with a region
 * SetupKFrameManager() - This function is in NS_MAIN, and reads the multiboot memory map and
 * modules, for typifying each pageframe. Then, all type 1 (free) pageframes are released into the
 * zone allocator, freeing all available memory
 *
 * Copyright (C) 2017 - Shukant Pal
 */

/** @Prefix m - Multiboot */

#define NS_KFRAMEMANAGER

#define NS_ADM
	#define NS_ADM_MULTIBOOT

#include <HardwareAbstraction/Processor.h>
#include <Memory/KFrameManager.h>
#include "../../../Interface/Utils/CtPrim.h"
#include <Multiboot2.h>
#include <Synch/Spinlock.h>
#include <KERNEL.h>

using namespace Memory;
using namespace Memory::Internal;

#define MAX_FRAME_ORDER 9 /* MAX-2M blocks*/
#define FRAME_VECTORS BDSYS_VECTORS(MAX_FRAME_ORDER)

#define KFRAME_ZONE_COUNT	5

PhysAddr KiMapTables(void);
PhysAddr mmLow;
PhysAddr mmHigh;
PhysAddr mmUsable;
PhysAddr mmTotal;
unsigned long pgUsable;
unsigned long pgTotal;

unsigned long memFrameTableSize;

Spinlock kfLock;

// Vector (bit-fields) for use by (buddy) allocator
static unsigned short allocatorVectors[(1 + FRAME_VECTORS) * 5];

// Allocation lists used by (buddy) allocator for all 5 zones
static LinkedList allocatorLists[FRAME_VECTORS * 5];

// You know there are five frame-zones - DMA, Driver, Kernel
// Code & Data.
static Zone frameZones[5];

// There are three zonal preferences for frame-allocation
// 1. DMA, which means addresses usable by DMA devices
// 2. Drivers, which means address fitting under 32/64-bits
// 3. Code, Data & Kernel, which all imply normal data
static ZonePreference framePreferences[3];

// This ZoneAllocator is the 'core' allocation engine for the
// KFrameManager. It may use more allocators in the future.
static ZoneAllocator coreEngine;

char msgSetupKFrameManager[] = "Setting up KFrameManager...";
char msgMemoryTooLow[] = "At least 128MB of memory is required to run the kernel.";

extern bool oballocNormaleUse;

/**
 * Function: KeFrameAllocate()
 *
 * Summary:  This function is used for allocating a physical pageframe from the system, for
 * a specific set of flags. Clients must also specify the zone from which the pageframe should
 * belong to.
 *
 * Args:
 * fOrder - Frame order needed
 * prefZone - Preferred zone's number
 * fFlags - Allocation-controlling flags
 *
 * Returns: Physical address of the pageframe, which can be mapped to any virtual address.
 *
 * @Version 1
 * @Since Circuit 2.03
 * @Author Shukant Pal
 * @See ZNSYS, ZNINFO, ZnAllocateBlock() - "ZoneManager.h"
 * // TODO:@Deprecated - Use System::getMemoryFrame()
 */
PhysAddr KeFrameAllocate(unsigned long fOrder, unsigned long prefZone, unsigned long znFlags)
{
	if(!oballocNormaleUse)
		znFlags |= FLG_NOCACHE;

	SpinLock(&kfLock);
	Zone *frDomain = frameZones + prefZone;

	__INTR_OFF
	unsigned long bInfo = (unsigned long) FRADDRESS(coreEngine.allocateBlock(fOrder, 0, frDomain, znFlags));
	if(!FLAG_SET(znFlags, OFFSET_NOINTR) && oballocNormaleUse)
	{ // Turn interrupts if allowed
		__INTR_ON
	}

	//Dbg("frame: "); DbgInt(bInfo); DbgLine("");
	SpinUnlock(&kfLock);
	return (bInfo);
}

unsigned long KeFrameFree(PhysAddr pAddress)
{
	SpinLock(&kfLock);
	coreEngine.freeBlock((BuddyBlock *) FRAME_AT(pAddress));
	SpinUnlock(&kfLock);
	return (1);
}

void TypifyMRegion(unsigned long typeValue, unsigned long regionStartKFrame, unsigned long regionEndKFrame)
{
	if(regionEndKFrame > pgTotal)
		regionEndKFrame = pgTotal;// BUG: pgTotal is less than memory

	BuddyBlock *pfCurrent = (BuddyBlock *) FROPAGE(regionStartKFrame);
	BuddyBlock *pfLimit = (BuddyBlock *) FROPAGE(regionEndKFrame);
	
	while((unsigned long) pfCurrent < (unsigned long) pfLimit)
	{
		pfCurrent->BdType = typeValue;
		pfCurrent = (BuddyBlock *) ((unsigned long ) pfCurrent + sizeof(MMFRAME));
	}
}

void KfReserveModules()
{
	MULTIBOOT_TAG_MODULE *muModule =
			(MULTIBOOT_TAG_MODULE*) SearchMultibootTagFrom(NULL, MULTIBOOT_TAG_TYPE_MODULE);

	while(muModule != NULL)
	{
		#ifdef ADM_MULTIBOOT_MODULE_DEBUGGER/* Debug *::Multiboot::Module Support */
			Dbg("Module ::ModuleStart "); DbgInt(muModule->ModuleStart); DbgLine(";"); 
		#endif
		TypifyMRegion(MULTIBOOT_MEMORY_MODULE, muModule->ModuleStart >> KPGOFFSET, muModule->ModuleEnd >> KPGOFFSET);
		muModule = (MULTIBOOT_TAG_MODULE*)
				SearchMultibootTagFrom(muModule, MULTIBOOT_TAG_TYPE_MODULE);
	}
}

/* Free all memory available by parsing the BdType field for every kfpage */
void KfParseMemory()
{
	BuddyBlock *kfPointer = (BuddyBlock *) FROPAGE(0);
	BuddyBlock *kfWall = (BuddyBlock *) FROPAGE(pgTotal);

	unsigned long last_bdtype = 10101001;
	
	while((ADDRESS) kfPointer <= (ADDRESS) kfWall)
	{
		if(kfPointer->BdType == MULTIBOOT_MEMORY_AVAILABLE)
			coreEngine.freeBlock(kfPointer);

		kfPointer = (BuddyBlock *) ((unsigned long) kfPointer + sizeof(MMFRAME));
		
		if(last_bdtype != kfPointer->BdType)
		{
			last_bdtype = kfPointer->BdType;
		}
	}
}

/**
 * Function: SetupKFrameManager()
 *
 * Summary:
 * This function is used for initializing the KFrameManager and parses the Multiboot2 structure
 * for getting basic meminfo and mmap (memory-map). It marks all reserved & module & other 'to-be-used'
 * regions as not-free. All other regions (marked free) are then released into the system and allow
 * the KFrameManager to allocate & deallocate kframes.
 *
 * @Version 1
 * @Since Circuit 2.03
 */
void SetupKFrameManager()
{
	DbgLine(msgSetupKFrameManager);

	MULTIBOOT_TAG_BASIC_MEMINFO *mInfo =
			(MULTIBOOT_TAG_BASIC_MEMINFO*)
			SearchMultibootTagFrom(NULL, MULTIBOOT_TAG_TYPE_BASIC_MEMINFO);

	MULTIBOOT_TAG_MMAP *mmMap =
			(MULTIBOOT_TAG_MMAP*)
			SearchMultibootTagFrom(NULL, MULTIBOOT_TAG_TYPE_MMAP);

	mmLow = mInfo->MmLower;
	mmHigh = mInfo->MmUpper;

	MULTIBOOT_MMAP_ENTRY *regionEntry  = (MULTIBOOT_MMAP_ENTRY*) ((U32) mmMap + sizeof(MULTIBOOT_TAG_MMAP));
	unsigned long regionLimit = (U32) mmMap + mmMap->Size - mmMap->EntrySize;

{ // Setup Memory Statistics:
	PhysAddr mRegionSize;
	PhysAddr usableMemory = 0;
	PhysAddr totalMemory = 0;

	while((unsigned long) regionEntry < regionLimit)
	{
		mRegionSize = (PhysAddr) regionEntry->Length;
		totalMemory += mRegionSize;
		if(regionEntry->Type == MULTIBOOT_MEMORY_AVAILABLE)
			usableMemory += mRegionSize & ~((1 << FRSIZE_ORDER) - 1);

		regionEntry = (MULTIBOOT_MMAP_ENTRY*) ((U32) regionEntry + mmMap->EntrySize);
	}

	mmUsable = usableMemory;
	mmTotal = totalMemory & ~((1 << (FRSIZE_ORDER + MAX_FRAME_ORDER)) - 1);

	if(mmTotal < MB(128))
	{
		DbgLine(msgMemoryTooLow);
		while(TRUE) asm("nop");
	}

	pgUsable = usableMemory >> KPGOFFSET;
	pgTotal =  mmTotal >> KPGOFFSET;

	KiMapTables();
}

	// INIT coreEngine

	coreEngine.resetAllocator((BuddyBlock *) KFRAMEMAP, framePreferences, 3, frameZones, 5);
	ZoneAllocator::configureZones(sizeof(MMFRAME), MAX_FRAME_ORDER, allocatorVectors, allocatorLists, frameZones, 5);
	ZoneAllocator::configurePreference(frameZones, framePreferences, 1);
	ZoneAllocator::configurePreference(frameZones + 1, framePreferences + 1, 1);
	ZoneAllocator::configurePreference(frameZones + 2, framePreferences + 2, 3);
	memsetf((Void *) KFRAMEMAP, 0, pgTotal * sizeof(MMFRAME));

	// MAP ZONE BOUNDARIES

	// This section resets the memory boundaries of the frame-zones to the optimized
	// values. ZONE_DMA has a fixed address & size, while ZONE_DRIVER has a fixed
	// address (platform-specific)
	unsigned char *entTable = (unsigned char *) KFRAMEMAP;
	frameZones[ZONE_DMA].memoryAllocator.setEntryTable(entTable);
	frameZones[ZONE_DMA].memorySize = MB(16) >> KPGOFFSET;
	frameZones[ZONE_DRIVER].memoryAllocator.setEntryTable(reinterpret_cast<unsigned char *>(FRAME_AT(MB(16))));

	if(pgTotal < MB(3584) >> KPGOFFSET)
	{
		// When memory is less than 3.5 GB, it is divided into four chunks, where ZONE_DMA
		// and ZONE_DRIVER share a chunk, while others get a full chunk.
		unsigned long pgDomSize = (pgTotal / 4) & ~((1 << 9) - 1); // Align 2m, no. of pages for 4 zones

		frameZones[ZONE_DRIVER].memorySize = pgDomSize - (MB(16) >> KPGOFFSET);

		frameZones[ZONE_CODE].memoryAllocator.setEntryTable(reinterpret_cast<unsigned char *>(FROPAGE(pgDomSize)));
		frameZones[ZONE_CODE].memorySize = pgDomSize;

		frameZones[ZONE_DATA].memoryAllocator.setEntryTable(reinterpret_cast<unsigned char *>(FROPAGE(2 * pgDomSize)));
		frameZones[ZONE_DATA].memorySize = pgDomSize;

		frameZones[ZONE_KERNEL].memoryAllocator.setEntryTable(reinterpret_cast<unsigned char *>(FROPAGE(3 * pgDomSize)));
		frameZones[ZONE_KERNEL].memorySize = pgTotal - 3 * pgDomSize;
	}
	else
	{
		// When memory is more than 3.5 GB, ZONE_DMA + ZONE_DRIVER take up only MB(896). The
		// remaining memory is divided into three chunks for all zones.
		frameZones[ZONE_DRIVER].memorySize = MB(880) >> KPGOFFSET;
		unsigned long pgRemaining = pgTotal - (MB(896) >> KPGOFFSET);
		unsigned long pgDomSize = pgRemaining / 3;

		frameZones[ZONE_CODE].memoryAllocator.setEntryTable(reinterpret_cast<unsigned char *>(FROPAGE(896)));
		frameZones[ZONE_CODE].memorySize = pgDomSize;

		frameZones[ZONE_DATA].memoryAllocator.setEntryTable(reinterpret_cast<unsigned char *>(FROPAGE(MB(896) + pgDomSize)));
		frameZones[ZONE_DATA].memorySize = pgDomSize;

		frameZones[ZONE_KERNEL].memoryAllocator.setEntryTable(
						reinterpret_cast<unsigned char *>(FROPAGE(MB(896) + 2 * pgDomSize)));
		frameZones[ZONE_KERNEL].memorySize = pgRemaining - 2 * pgDomSize;
	}

	// All buddy-block's zone-index field should be mapped to the right zone.
	ZoneAllocator::configureZoneMappings(frameZones, 5);

	regionEntry = (MULTIBOOT_MMAP_ENTRY *) ((unsigned long) mmMap + sizeof(MULTIBOOT_TAG_MMAP));
{
	PhysAddr pRegionStart;
	PhysAddr pRegionEnd;

	unsigned long regionStartKFrame;
	unsigned long regionEndKFrame;

	while((unsigned long) regionEntry < regionLimit)
	{
		pRegionStart = regionEntry->Address;
		pRegionEnd = pRegionStart + regionEntry->Length;
	
		regionStartKFrame = pRegionStart >> 12;
		regionEndKFrame = pRegionEnd >> 12;

		#ifdef ADM_MULTIBOOT_MMAP_DEBUGGER
			Dbg("pRegion: "); DbgInt(regionStartKFrame); Dbg(", "); DbgInt(regionEndKFrame); Dbg(" $"); DbgInt(regionEntry->Type); DbgLine(";");
		#endif

		if(regionEntry->Type != MULTIBOOT_MEMORY_AVAILABLE){
			if(((U32) pRegionEnd) & (KPGSIZE -  1))
				++regionEndKFrame;
		} else if(((U32) pRegionStart) & (KPGSIZE - 1))
				++regionStartKFrame;

		TypifyMRegion(regionEntry->Type, regionStartKFrame, regionEndKFrame);
		regionEntry = (MULTIBOOT_MMAP_ENTRY *) ((unsigned long) regionEntry + mmMap->EntrySize);
	}
}
	unsigned long admMultibootTableStartKFrame = admMultibootTableStart >> KPGOFFSET;
	unsigned long admMultibootTableEndKFrame = ((admMultibootTableEnd % KPGSIZE) ?
							admMultibootTableEnd + KPGSIZE - admMultibootTableEnd % KPGSIZE :
							admMultibootTableEnd) >> KPGOFFSET;

	TypifyMRegion(MULTIBOOT_MEMORY_STRUCT, admMultibootTableStartKFrame, admMultibootTableEndKFrame);// Reserved multiboot struct
	TypifyMRegion(MULTIBOOT_MEMORY_STRUCT, MB(16), memFrameTableSize);

	KfReserveModules();// Reserve modules, as they are not in mmap
	KfParseMemory();// Free all available memory!!!

	coreEngine.resetStatistics();
}
