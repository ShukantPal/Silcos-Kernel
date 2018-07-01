/**
 * @file PageFrame.cpp
 *
 * Implements an buddy-based zone-allocator for page-frame
 * management. It assigns the following zonal attributes on physical
 * page-frames: DMA-zone, driver-zone, code-zone, data-zone, and
 * the kernel-zone.
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

/** @Prefix m - Multiboot */

#define NS_KFRAMEMANAGER

#define NS_ADM
	#define NS_ADM_MULTIBOOT

#include <HardwareAbstraction/Processor.h>
#include <Memory/KFrameManager.h>
#include "../../../Interface/Utils/CtPrim.h"
#include <Multiboot2.h>
#include <Synch/Spinlock.h>
#include <Environment.h>
#include <KERNEL.h>
#include <Debugging.h>

using namespace Memory;
using namespace Memory::Internal;

#define MAX_FRAME_ORDER 9 /* MAX-2M blocks*/
#define FRAME_VECTORS BDSYS_VECTORS(MAX_FRAME_ORDER)

#define KFRAME_ZONE_COUNT	5

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
 * Allocates blocks of physical-memory to the caller, in the form of
 * multiple adjacent page-frames, in powers of two.
 *
 * @param fOrder
 * @param prefZone
 * @param znFlags
 * @return
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

/**
 * Fills the 'BdType' field of the page-frame entries, and is used before
 * freeing all available physical memory. This prevents dedicated kernel
 * memory from being allocated later on. Currently, only the customly defined
 * type MULTIBOOT_MEMORY_STRUCT is used for this purpose.
 *
 * @param typeValue
 * @param regionStartKFrame
 * @param regionEndKFrame
 */
void TypifyMRegion(unsigned long typeValue, unsigned long regionStartKFrame,
		unsigned long regionEndKFrame)
{
	if(regionEndKFrame > pgTotal)
		regionEndKFrame = pgTotal;// BUG: pgTotal is less than memory

	BuddyBlock *pfCurrent = (BuddyBlock *) FROPAGE(regionStartKFrame);
	BuddyBlock *pfLimit = (BuddyBlock *) FROPAGE(regionEndKFrame);
	
	while((unsigned long) pfCurrent < (unsigned long) pfLimit)
	{
		pfCurrent->BdType = typeValue;
		pfCurrent = (BuddyBlock *) ((unsigned long ) pfCurrent +
				sizeof(MMFRAME));
	}
}

/**
 * Reserves the memory regions that are occupied by the kernel module
 * files (copied from disk by bootloader). It is now not used, as the
 * Initor module already copies them into the kernel environment, but
 * may be used in the future (for advanced features like config files,
 * console-to-file output, etc.)
 *
 * @deprecated
 * @since Circuit 2.05
 */
void KfReserveModules()
{
	MultibootTagModule *muModule = (MultibootTagModule *)
			MultibootChannel::getFirstModule();

	while(muModule != NULL)
	{
		TypifyMRegion(MULTIBOOT_MEMORY_MODULE,
				muModule->moduleStart >> KPGOFFSET,
				muModule->moduleEnd >> KPGOFFSET);
		muModule = (MultibootTagModule *)
				MultibootChannel::getNextModule(muModule);
	}
}

/**
 * Serially frees all available page-frames by checking whether their
 * BdType signals that they are usable memory. This is a long loop
 * which takes significant time (for the CPU, user won't notice it)
 */
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
 * Initializes the page-frame allocator subsystem, setting free all
 * available system memory. This excludes the kernel-environment, ACPI
 * used memory, and bootloader data.
 *
 * It uses the multiboot-channel, which must be setup before page-frame
 * allocation initializes. Note that, before the permanent per-CPU structs
 * are placed in memory, page-frame allocation must be done with
 * special flags like (@code FLG_NOCACHE) and (@code KF_NOINTR) to prevent
 * any sort of access to the undefined pointers.
 *
 * All initialization parameters are build time - other than zonal sizes
 * and boundaries. Other than that, this function is **fragile** and any
 * modification must be done carefully.
 *
 * @version 7.05
 * @since Circuit 2.03
 * @author Shukant Pal
 */
void SetupKFrameManager()
{
	DbgLine(msgSetupKFrameManager);

	MultibootTagBasicMemInfo *mInfo =
			MultibootChannel::getBasicMemInfo();
	MultibootTagMMap *mmMap =
			MultibootChannel::getMMap();

	mmLow = mInfo->MmLower;
	mmHigh = mInfo->MmUpper;

	MultibootMMapEntry *regionEntry  = mmMap->getEntries();
	unsigned long regionLimit = (U32) mmMap + mmMap->size - mmMap->entrySize;

{ // Setup Memory Statistics:
	PhysAddr usableMemory = 0;
	PhysAddr foundMemory = 0;

	while(mmMap->inBounds(regionEntry))
	{
		foundMemory += regionEntry->length;

		if(regionEntry->type == MULTIBOOT_MEMORY_AVAILABLE) {
			usableMemory +=	// only whole page-frames are usable
					regionEntry->length & ~((1 << FRSIZE_ORDER) - 1);
		}

		regionEntry = regionEntry->next(mmMap->entrySize);
	}

	mmUsable = usableMemory;
	mmTotal = foundMemory & // only whole maximum sized blocks count!
			~((1 << (FRSIZE_ORDER + MAX_FRAME_ORDER)) - 1);

	if(mmTotal < MB(128))
	{
		DbgLine(msgMemoryTooLow);
		while(TRUE) asm("nop");
	}

	pgUsable = usableMemory >> KPGOFFSET;
	pgTotal =  mmTotal >> KPGOFFSET;

	Pager::mapAllHuge(KFRAMEMAP, kernelParams.pageframeEntries,
			(mmTotal >> 12) * sizeof(MMFRAME), null, KernelData);
}

	// INIT coreEngine

	coreEngine.resetAllocator((BuddyBlock *) KFRAMEMAP, framePreferences, 3, frameZones, 5);
	ZoneAllocator::configureZones(sizeof(MMFRAME), MAX_FRAME_ORDER,
			allocatorVectors, allocatorLists, frameZones, 5);
	ZoneAllocator::configurePreference(frameZones, framePreferences, 1);
	ZoneAllocator::configurePreference(frameZones + 1,
			framePreferences + 1, 1);
	ZoneAllocator::configurePreference(frameZones + 2,
			framePreferences + 2, 3);
	memsetf((Void *) KFRAMEMAP, 0, pgTotal * sizeof(MMFRAME));

	// MAP ZONE BOUNDARIES

	// This section resets the memory boundaries of the frame-zones to the optimized
	// values. ZONE_DMA has a fixed address & size, while ZONE_DRIVER has a fixed
	// address (platform-specific)
	unsigned char *entTable = (unsigned char *) KFRAMEMAP;
	frameZones[ZONE_DMA].memoryAllocator.setEntryTable(entTable);
	frameZones[ZONE_DMA].memorySize = MB(16) >> KPGOFFSET;
	frameZones[ZONE_DRIVER].memoryAllocator.setEntryTable((unsigned char *) FRAME_AT(MB(16)));

	if(pgTotal < MB(3584) >> KPGOFFSET)
	{
		// When memory is less than 3.5 GB, it is divided into four chunks, where ZONE_DMA
		// and ZONE_DRIVER share a chunk, while others get a full chunk.
		unsigned long pgDomSize = (pgTotal / 4) & ~((1 << 9) - 1); // Align 2m, no. of pages for 4 zones

		frameZones[ZONE_DRIVER].memorySize = pgDomSize - (MB(16) >> KPGOFFSET);

		frameZones[ZONE_CODE].memoryAllocator.setEntryTable(
				(unsigned char *) FROPAGE(pgDomSize));
		frameZones[ZONE_CODE].memorySize = pgDomSize;

		frameZones[ZONE_DATA].memoryAllocator.setEntryTable(
				(unsigned char *) FROPAGE(2 * pgDomSize));
		frameZones[ZONE_DATA].memorySize = pgDomSize;

		frameZones[ZONE_KERNEL].memoryAllocator.setEntryTable(
				(unsigned char*) FROPAGE(3 * pgDomSize));
		frameZones[ZONE_KERNEL].memorySize = pgTotal - 3 * pgDomSize;
	}
	else
	{
		// When memory is more than 3.5 GB, ZONE_DMA + ZONE_DRIVER take up only MB(896). The
		// remaining memory is divided into three chunks for all zones.
		frameZones[ZONE_DRIVER].memorySize = MB(880) >> KPGOFFSET;
		unsigned long pgRemaining = pgTotal - (MB(896) >> KPGOFFSET);
		unsigned long pgDomSize = pgRemaining / 3;

		frameZones[ZONE_CODE].memoryAllocator.setEntryTable(
				(unsigned char*) FROPAGE(896));
		frameZones[ZONE_CODE].memorySize = pgDomSize;

		frameZones[ZONE_DATA].memoryAllocator.setEntryTable((unsigned char*)
				FROPAGE(MB(896) + pgDomSize));
		frameZones[ZONE_DATA].memorySize = pgDomSize;

		frameZones[ZONE_KERNEL].memoryAllocator.setEntryTable(
						(unsigned char*) FROPAGE(MB(896) + 2 * pgDomSize));
		frameZones[ZONE_KERNEL].memorySize = pgRemaining - 2 * pgDomSize;
	}

	// All buddy-block's zone-index field should be mapped to the right zone.
	ZoneAllocator::configureZoneMappings(frameZones, 5);

	regionEntry = mmMap->getEntries();
{
	PhysAddr pRegionStart;
	PhysAddr pRegionEnd;

	unsigned long regionStartKFrame;
	unsigned long regionEndKFrame;

	while((unsigned long) regionEntry < regionLimit)
	{
		pRegionStart = regionEntry->address;
		pRegionEnd = pRegionStart + regionEntry->length;
	
		regionStartKFrame = pRegionStart >> 12;
		regionEndKFrame = pRegionEnd >> 12;

		#ifdef ADM_MULTIBOOT_MMAP_DEBUGGER
			Dbg("pRegion: "); DbgInt(regionStartKFrame); Dbg(", "); DbgInt(regionEndKFrame); Dbg(" $"); DbgInt(regionEntry->type); DbgLine(";");
		#endif

		if(regionEntry->type != MULTIBOOT_MEMORY_AVAILABLE){
			if(((U32) pRegionEnd) & (KPGSIZE -  1))
				++regionEndKFrame;
		} else if(((U32) pRegionStart) & (KPGSIZE - 1))
				++regionStartKFrame;

		TypifyMRegion(regionEntry->type, regionStartKFrame, regionEndKFrame);
		regionEntry = regionEntry->next(mmMap->entrySize);
	}
}

	/*
	 * Prevent allocation of dedicated kernel memory in the physical
	 * address space. This includes the multiboot-information table, the
	 * kernel environment and the adjacent page-frame entries.
	 *
	 * Since the Initor module has come into use, the need for reserving
	 * modules is nil. But, in the future, when configuration files,
	 * kernel-based scripts, etc. advanced features come into use, it may
	 * become a requirement to again reserved their files. Therefore the
	 * KfReserveModules() call, is commented out only!
	 */
	TypifyMRegion(MULTIBOOT_MEMORY_STRUCT, kernelParams.multibootTable,
			MultibootChannel::getMultibootTableSize());// Reserved multiboot struct
	TypifyMRegion(MULTIBOOT_MEMORY_STRUCT, kernelParams.loadAddress,
			kernelParams.pageframeEntries - kernelParams.loadAddress
			+ pgTotal * sizeof(MMFRAME));

//	KfReserveModules();// Reserve modules, as they are not in mmap
	KfParseMemory();// Free all available memory!!!

	coreEngine.resetStatistics();
}
