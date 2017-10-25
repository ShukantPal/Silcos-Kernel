/*=++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
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
 *+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=*/

/** @Prefix m - Multiboot */

#define NS_KFRAMEMANAGER

#define NS_ADM
	#define NS_ADM_MULTIBOOT

#include <Circuit.h>
#include <HAL/Processor.h>
#include <Memory/KFrameManager.h>
#include <Util/CtPrim.h>
#include <Multiboot2.h>
#include <Synch/Spinlock.h>
#include <KERNEL.h>

#define MAX_FRAME_ORDER 9 /* MAX-2M blocks*/
#define FRAME_VECTORS BDSYS_VECTORS(MAX_FRAME_ORDER)

#define KFRAME_ZONE_COUNT	5

PADDRESS KiMapTables(VOID);
PADDRESS mmLow;
PADDRESS mmHigh;
PADDRESS mmUsable;
PADDRESS mmTotal;
ULONG pgUsable;
ULONG pgTotal;

ULONG memFrameTableSize;

USHORT znDMALiInfo[1 + FRAME_VECTORS];/* DMA zone */
LINKED_LIST znDMALi[FRAME_VECTORS];

USHORT znNormalLiInfo[1 + FRAME_VECTORS];/* KERNEL32 */
LINKED_LIST znNormalLi[FRAME_VECTORS];

USHORT znCodeLiInfo[1 + FRAME_VECTORS];/* CODE0 */
LINKED_LIST znCodeLi[FRAME_VECTORS];

USHORT znDataLiInfo[1 + FRAME_VECTORS];/* DATA0 */
LINKED_LIST znDataLi[FRAME_VECTORS];

USHORT znKernelLiInfo[1 + FRAME_VECTORS];/* KERNEL */
LINKED_LIST znKernelLi[FRAME_VECTORS];

SPIN_LOCK kfLock;

ZNINFO frameDomains[5];
//{
	//{
		//.MmManager = {
			//.DescriptorSize = sizeof(MMFRAME),
			//.HighestOrder = MAX_FRAME_ORDER,
			//.ListInfo = &znDMALiInfo[0],
			//.BlockLists = &znDMALi[0]
		//},
		//.MmSize = MB(16) / KB(4),
		//.MmReserved = MB(4) / KB(4),
		//.ZnPref = 0,
		//.Cache = { .ChMemoryOffset = FRCH_OFFSET }
	//}, {
		//.MmManager = {
			//.DescriptorSize = sizeof(MMFRAME),
			//.HighestOrder = MAX_FRAME_ORDER,
			//.ListInfo = &znNormalLiInfo[0],
			//.BlockLists = &znNormalLi[0]
		//},
		//.ZnPref = 1,
		//.Cache = { .ChMemoryOffset = FRCH_OFFSET + sizeof(CHREG) }
	//}, {
		//.MmManager = {
			//.DescriptorSize = sizeof(MMFRAME),
			//.HighestOrder = MAX_FRAME_ORDER,
			//.ListInfo = &znCodeLiInfo[0],
			//.BlockLists = &znCodeLi[0]
		//},
		//.ZnPref = 2,
		//.Cache = { .ChMemoryOffset = FRCH_OFFSET + 2 * sizeof(CHREG) }
	//}, {
		//.MmManager = {
			//.DescriptorSize = sizeof(MMFRAME),
			//.HighestOrder = MAX_FRAME_ORDER,
			//.ListInfo = &znDataLiInfo[0],
			//.BlockLists = &znDataLi[0]
		//},
		//.ZnPref = 2,
		//.Cache = { .ChMemoryOffset = FRCH_OFFSET + 3 * sizeof(CHREG) }
	//}, {
	//.MmManager = {
			//.DescriptorSize = sizeof(MMFRAME),
			//.HighestOrder = MAX_FRAME_ORDER,
			//.ListInfo = &znKernelLiInfo[0],
			//.BlockLists = &znKernelLi[0]
		//},
		//.ZnPref = 2,
	//	.Cache = { .ChMemoryOffset = FRCH_OFFSET + 4 * sizeof(CHREG) }
//	}
//};

ZNPREF framePreferences[3];
//{
	//{
	//	.ZnPref = 0
	//}, {
	//	.ZnPref = 1
	//}, {
	//	.ZnPref = 2
	//}
//};

ZNSYS frameAllocator;
//{/
	//.ZnPref = &(framePreferences[0]),
	//.ZnSet = &(frameDomains[0]),
	//.ZnPrefCount = 1,
	//.ZnPrefBase = 0,
	//.ZnCacheRefill = 6
//};

static void setupZones(void)
{
	struct Zone *pmZone = frameDomains;
	for(long zIndex = 0; zIndex < KFRAME_ZONE_COUNT; zIndex++){
		pmZone->MmManager.DescriptorSize = sizeof(MMFRAME);
		pmZone->MmManager.HighestOrder = MAX_FRAME_ORDER;
		pmZone->ZnPref = zIndex;
		pmZone->Cache.ChMemoryOffset = FRCH_OFFSET + zIndex * sizeof(CHREG);
		++(pmZone);
	}

	frameDomains[0].MmSize = MB(16) >> KPGOFFSET;
	frameDomains[0].MmReserved = MB(2) >> KPGOFFSET;

	frameDomains[0].MmManager.ListInfo = znDMALiInfo;
	frameDomains[0].MmManager.BlockLists = znDMALi;
	frameDomains[1].MmManager.ListInfo = znNormalLiInfo;
	frameDomains[1].MmManager.BlockLists = znNormalLi;
	frameDomains[2].MmManager.ListInfo = znCodeLiInfo;
	frameDomains[2].MmManager.BlockLists = znCodeLi;
	frameDomains[3].MmManager.ListInfo = znDataLiInfo;
	frameDomains[3].MmManager.BlockLists = znDataLi;
	frameDomains[4].MmManager.ListInfo = znKernelLiInfo;
	frameDomains[4].MmManager.BlockLists = znKernelLi;

	// Setup Preferences
	for(long pIndex = 0; pIndex < 3; pIndex++)
		framePreferences[pIndex].ZnPref = pIndex;

	frameAllocator.ZnPref = &(framePreferences[0]);
	frameAllocator.ZnSet = &(frameDomains[0]);
	frameAllocator.ZnPrefCount = 1;
	frameAllocator.ZnPrefBase = 0;
	frameAllocator.ZnCacheRefill = 6;
}

CHAR *msgSetupKFrameManager = "Setting up KFrameManager...";
CHAR *msgMemoryTooLow = "At least 128MB of memory is required to run the kernel.";

PADDRESS KeFrameAllocate(ULONG fOrder, ULONG prefZone, ULONG znFlags)
{
	SpinLock(&kfLock);
	ZNINFO *frDomain = &frameAllocator.ZnSet[prefZone];

	__INTR_OFF
	ULONG bInfo = (ULONG) ZnAllocateBlock(fOrder, 0, frDomain, znFlags, &frameAllocator);
	if(!FLAG_SET(znFlags, OFFSET_NOINTR)) { // Turn interrupts if allowed
		__INTR_ON
	}
	SpinUnlock(&kfLock);
	return (FRADDRESS(bInfo));
}

ULONG KeFrameFree(PADDRESS pAddress)
{
	SpinLock(&kfLock);
	ZnFreeBlock(FRAME_AT(pAddress), &frameAllocator);
	SpinUnlock(&kfLock);
	return (1);
}

void TypifyMRegion(ULONG typeValue, ULONG regionStartKFrame, ULONG regionEndKFrame)
{
	if(regionEndKFrame > pgTotal) regionEndKFrame = pgTotal;// BUG: pgTotal is less than memory
	BDINFO *pfCurrent = (BDINFO *) FROPAGE(regionStartKFrame);
	BDINFO *pfLimit = (BDINFO *) FROPAGE(regionEndKFrame);
	
	while((ULONG) pfCurrent < (ULONG) pfLimit){
		pfCurrent->BdType = typeValue;
		pfCurrent = (BDINFO *) ((ULONG ) pfCurrent + sizeof(MMFRAME));
	}
}

void KfReserveModules()
{
	MULTIBOOT_TAG_MODULE *muModule = SearchMultibootTag(MULTIBOOT_TAG_TYPE_MODULE);
	while(muModule != NULL){
		#ifdef ADM_MULTIBOOT_MODULE_DEBUGGER/* Debug *::Multiboot::Module Support */
			Dbg("Module ::ModuleStart "); DbgInt(muModule->ModuleStart); DbgLine(";"); 
		#endif
		TypifyMRegion(MULTIBOOT_MEMORY_MODULE, muModule->ModuleStart >> KPGOFFSET, muModule->ModuleEnd >> KPGOFFSET);
		muModule = SearchMultibootTagFrom(muModule, MULTIBOOT_TAG_TYPE_MODULE);
	}
}

/* Free all memory available by parsing the BdType field for every kfpage */
void KfParseMemory()
{
	BDINFO *kfPointer = (BDINFO *) FROPAGE(0);
	BDINFO *kfWall = (BDINFO *) FROPAGE(pgTotal);
	
	ULONG ptr_counter = 0;
	ULONG last_bdtype = 10101001;
	
	while((ADDRESS) kfPointer <= (ADDRESS) kfWall){
		if(kfPointer->BdType == MULTIBOOT_MEMORY_AVAILABLE)
			ZnFreeBlock(kfPointer, &frameAllocator);
		kfPointer = (BDINFO *) ((ULONG) kfPointer + sizeof(MMFRAME));
		
		if(last_bdtype != kfPointer->BdType){
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
	setupZones();
	DbgLine(msgSetupKFrameManager);

	MULTIBOOT_TAG_BASIC_MEMINFO *mInfo = SearchMultibootTag(MULTIBOOT_TAG_TYPE_BASIC_MEMINFO);
	MULTIBOOT_TAG_MMAP *mmMap = SearchMultibootTag(MULTIBOOT_TAG_TYPE_MMAP);

	mmLow = mInfo->MmLower;
	mmHigh = mInfo->MmUpper;

	MULTIBOOT_MMAP_ENTRY *regionEntry  = (MULTIBOOT_MMAP_ENTRY*) ((U32) mmMap + sizeof(MULTIBOOT_TAG_MMAP));
	ULONG regionLimit = (U32) mmMap + mmMap->Size - mmMap->EntrySize;

{ // Setup Memory Statistics:
	PADDRESS mRegionSize;
	PADDRESS usableMemory = 0;
	PADDRESS totalMemory = 0;

	while((ULONG) regionEntry < regionLimit) {
		mRegionSize = (PADDRESS) regionEntry->Length;
		totalMemory += mRegionSize;
		if(regionEntry->Type == MULTIBOOT_MEMORY_AVAILABLE)
			usableMemory += mRegionSize & ~((1 << FRSIZE_ORDER) - 1);

		regionEntry = (MULTIBOOT_MMAP_ENTRY*) ((U32) regionEntry + mmMap->EntrySize);
	}

	mmUsable = usableMemory;
	mmTotal = totalMemory & ~((1 << (FRSIZE_ORDER + MAX_FRAME_ORDER)) - 1);

	if(mmTotal < MB(128)) {
		DbgLine(msgMemoryTooLow);
		while(TRUE) asm("nop");
	}

	pgUsable = usableMemory >> KPGOFFSET;
	pgTotal =  mmTotal >> KPGOFFSET;

	KiMapTables();
}

{ // Setup Zone Preferences:
	ZNINFO *znCurrent = &frameDomains[0];
	ZNPREF *znpCurrent = &framePreferences[0];
	ClnInsert((CLNODE *) znCurrent, CLN_LAST, &znpCurrent->ZoneList);
	++znCurrent;
	++znpCurrent;
	ClnInsert((CLNODE *) znCurrent, CLN_LAST, &znpCurrent->ZoneList);
	++znCurrent;
	++znpCurrent;
	ClnInsert((CLNODE *) znCurrent, CLN_LAST, &znpCurrent->ZoneList);
	++znCurrent;
	ClnInsert((CLNODE *) znCurrent, CLN_LAST, &znpCurrent->ZoneList);
	++znCurrent;
	ClnInsert((CLNODE *) znCurrent, CLN_LAST, &znpCurrent->ZoneList);
}

	frameDomains[0].MmManager.DescriptorTable = (UBYTE *) KFRAMEMAP;
	frameDomains[1].MmManager.DescriptorTable = (UBYTE *) FRAME_AT(MB(16));

	if(pgTotal < (MB(3584) / KB(4))) {
		ULONG pgDomSize = (pgTotal / 4) & ~((1 << 9) - 1); // Align 2m
		frameDomains[1].MmSize = pgDomSize - (MB(16) / KB(4));
		frameDomains[2].MmManager.DescriptorTable = (UBYTE *) FROPAGE(pgDomSize);
		frameDomains[2].MmSize = pgDomSize;
		frameDomains[3].MmManager.DescriptorTable = (UBYTE *) FROPAGE(pgDomSize * 2);
		frameDomains[3].MmSize = pgDomSize;
		frameDomains[4].MmManager.DescriptorTable = (UBYTE *) FROPAGE(pgDomSize * 3);
		frameDomains[4].MmSize = pgTotal - (3 * pgDomSize);
	} else {
		frameDomains[1].MmSize = MB(880);
		ULONG pgRemaining = pgTotal - (MB(896) / KB(4));
		ULONG pgDomSize = pgRemaining / 3;
		frameDomains[2].MmManager.DescriptorTable = (UBYTE *) FROPAGE(MB(896));
		frameDomains[2].MmSize = pgDomSize;
		frameDomains[3].MmManager.DescriptorTable = (UBYTE *) FROPAGE(MB(896) + pgDomSize);
		frameDomains[3].MmSize = pgDomSize;
		frameDomains[4].MmManager.DescriptorTable = (UBYTE *) FROPAGE(MB(896) + 2 * pgDomSize);
		frameDomains[4].MmSize = pgRemaining - 2 * pgDomSize;
	}
	ZnReverseMap(0, frameDomains, &frameAllocator);

	USHORT znOffset;
	ZNINFO *znCurrent = &frameDomains[1];
	for(znOffset=1; znOffset<5; znOffset++) {
		znCurrent->MmReserved = znCurrent->MmSize / 32;
		ZnReverseMap(znOffset, znCurrent, &frameAllocator);
		++znCurrent;	
	}
	regionEntry = (MULTIBOOT_MMAP_ENTRY *) ((ULONG) mmMap + sizeof(MULTIBOOT_TAG_MMAP));
{
	PADDRESS pRegionStart;
	PADDRESS pRegionEnd;

	ULONG regionStartKFrame;
	ULONG regionEndKFrame;

	while((ULONG) regionEntry < regionLimit) {
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
		regionEntry = (MULTIBOOT_MMAP_ENTRY *) ((ULONG) regionEntry + mmMap->EntrySize);
	}
}
	ULONG admMultibootTableStartKFrame = admMultibootTableStart >> KPGOFFSET;
	ULONG admMultibootTableEndKFrame = ((admMultibootTableEnd % KPGSIZE) ? admMultibootTableEnd + KPGSIZE - admMultibootTableEnd % KPGSIZE : admMultibootTableEnd) >> KPGOFFSET;
	TypifyMRegion(MULTIBOOT_MEMORY_STRUCT, admMultibootTableStartKFrame, admMultibootTableEndKFrame);// Reserved multiboot struct
	KfReserveModules();// Reserve modules, as they are not in mmap
	KfParseMemory();// Free all available memory!!!

	znCurrent = &frameDomains[0];
	for(znOffset=0; znOffset<5; znOffset++) {
		znCurrent->MmAllocated = 0;
		znCurrent = &frameDomains[znOffset];
	}

	return;
}
