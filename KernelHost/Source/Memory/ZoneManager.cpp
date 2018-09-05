/**
 * @file ZoneManager.cpp
 *
 * ToDo: CHANGE FILENAME TO ZONEALLOCATOR.CPP
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
#define NAMESPACE_KFRAME_MANAGER

#include <Memory/KFrameManager.h>
#include <Memory/Internal/ZoneAllocator.hpp>
#include <Utils/CircularList.h>
#include <KERNEL.h>

using namespace Memory;
using namespace Memory::Internal;

#define ZONE_ALLOCABLE 10
#define ZONE_RESERVE_ONLY 11
#define ZONE_BARRIER_ONLY 12
#define ZONE_OVERLOAD 101
#define ZONE_LOADED 102

void ZoneAllocator::resetAllocator(BuddyBlock *entryTable,
		ZonePreference *prefTable, unsigned long prefCount,
		Zone *zoneTable, unsigned long zoneCount)
{
	this->descriptorTable = entryTable;
	this->prefTable = prefTable;
	this->prefCount = prefCount;
	this->zoneTable = zoneTable;
	this->zoneCount = zoneCount;
}

void ZoneAllocator::resetStatistics()
{
	Zone *zone = zoneTable;
	for(unsigned long index = 0; index < zoneCount; index++){
		zone->memoryAllocated = 0;
		++(zone);
	}
}

/*
 * Indicates the state of an allocation for a zone. Based on these values, the
 * allocation is done.
 */
enum ZoneState
{
	ALLOCABLE = 10,			// Zone is allocable for general-use memory
	RESERVE_OVERLAP = 11,	// Zone is allocable for ATOMIC operations only
	BARRIER_OVERLAP = 12,	// Zone is allocable for emergency operations only
	LOW_ON_MEMORY = 1001	// Zone is low-only-memory for required allocation
};

typedef unsigned long ZNALLOC; /* Internal type - Used for testing if zone is allocable from */

/**
 * Finds the status for a particular allocation scenario for a zone. This
 * depends on the size of the request memory.
 *
 * @param requiredMemory - size of the request memory
 * @param searchZone - zone in which the allocation is being tested
 * @version 1.1.9
 * @since Circuit 2.03,++
 * @author Shukant Pal
 */
static enum ZoneState getStatus(unsigned long requiredMemory, Zone *searchZone)
{
	unsigned long generalMemoryAvail = searchZone->memorySize - searchZone->memoryAllocated;

	if(requiredMemory > searchZone->memorySize - searchZone->memoryAllocated) {
		return (LOW_ON_MEMORY);
	} else {
		// Memory excluding reserved amount
		generalMemoryAvail -= searchZone->memoryReserved;

		if(requiredMemory <= generalMemoryAvail) {
			return (ALLOCABLE);
		}
		else {
			// Memory including reserved amount (minus emergency amount)
			generalMemoryAvail += (7 * searchZone->memoryReserved) >> 3;

			if(requiredMemory <= generalMemoryAvail)
				return (RESERVE_OVERLAP);// Only allow for ATOMIC allocations
			else
				return (BARRIER_OVERLAP);// Only allow for emergency operations
		}
	}
}

#define ZONE_ALLOCATE 0xF1
#define ZONE_SWITCH 0xF2
#define ZONE_FAILURE 0xFF
typedef unsigned long ZNACTION;

/*
 * These values tell what to do to allocate memory in a zone.
 */
enum AllocationAction
{
	ALLOCATE = 0x100A1,	// Allocate directly now
	GOTO_NEXT = 0x100A2,// Try another zone
	RET_FAIL = 0x100FF	// Return failure (for only first zone, other
						// -wise same as GOTO_NEXT)
};

/**
 * Tells what to do next to allocate the memory, based on the status of the
 * allocation.
 *
 * @version 1.1
 * @since Circuit 2.03,++
 * @author Shukant Pal
 */
static AllocationAction getAction(ZoneState allocState, ZNFLG allocFlags)
{
	switch(allocState) {
	case ALLOCABLE:
		return (ALLOCATE);

	case RESERVE_OVERLAP:
		if(FLAG_SET(allocState, ATOMIC) || FLAG_SET(allocState, NO_FAILURE))
			return (ALLOCATE);
		break;

	case BARRIER_OVERLAP:
		if(FLAG_SET(allocFlags, NO_FAILURE))
			return (ALLOCATE);
		break;
	default:
		break;
	}

	if(FLAG_SET(allocFlags, ZONE_REQUIRED))
		return (RET_FAIL);
	else
		return (GOTO_NEXT);
}

/**
 * Returns the zone which the allocator should use to gain free memory and
 * return the requester of memory.
 *
 * @param blockOrder - Order of block being requested for allocation
 * @param basePref - Minimal preference-index of the allocation
 * @param allocFlags - Flags for allocating the block
 * @param zonePref - Preferred zone of allocation
 * @version 1.1
 * @since Circuit 2.03,++
 * @author Shukant Pal
 */
Zone *ZoneAllocator::getZone(unsigned long blockOrder,
		unsigned long basePref, ZNFLG allocFlags, Zone *zonePref)
{
	enum ZoneState testState;
	enum AllocationAction testAction;
	Zone *trialZone = zonePref;
	Zone *trialZero = zonePref;// Head of circular-list

	unsigned long testPref = zonePref->preferenceIndex;
	while(testPref >= basePref) {
		do {
			SpinLock(&trialZone->controlLock);

			testState =  getStatus(blockOrder, trialZone);
			testAction = getAction(testState, allocFlags);
			switch(testAction)
			{
			case ALLOCATE:
				return (trialZone);
			case RET_FAIL:
				return (NULL);
			case GOTO_NEXT:
				break;
			}

			SpinUnlock(&trialZone->controlLock);
			trialZone = trialZone->nextZone;
		} while(trialZone != trialZero);

		--(testPref);
		trialZone = (Zone *) (prefTable[testPref].ZoneList.lMain);
		trialZero = trialZone;
	}

	return (NULL);// Rarely used
}

/**
 * Allocates the request amount of memory, by searching in all zones in the
 * preferential manner - checking the given preferred zone, trying to allocate
 * from a zone of the same preference, then going down the preference table.
 *
 * Not implemented yet:
 * A cache is also implemented on top of each zone to reduce the overall
 * latency of lower-end allocations.
 *
 * @param blockOrder - the order of the request memory block
 * @param basePref - the minimum preference level of the zone from which
 * 						allocation should be done!
 * @param zonePref - preferred zone for allocation, for the client
 * @param allocFlags - the flags given for allocation
 * @version 1.1.1
 * @since Circuit 2.03++
 * @author Shukant Pal
 */
BuddyBlock *ZoneAllocator::allocateBlock(unsigned long blockOrder, unsigned long basePref, Zone *zonePref,
						ZNFLG allocFlags)
{
	Zone *allocatingZone = getZone(blockOrder, basePref, allocFlags, zonePref);

	if(allocatingZone == NULL) {
		return (NULL);
	} else {
		// Try allocating from its cache, for order(0) allocations
		// TODO: Implement the ZCache extension for alloc-internal

		BuddyBlock *blockRequired =
				allocatingZone->memoryAllocator.allocateBlock(blockOrder);
		allocatingZone->memoryAllocated += SIZEOF_ORDER(blockOrder);

		SpinUnlock(&allocatingZone->controlLock);// getZone gives the zone in a locked state
		return (blockRequired);
	}
}


/*
extern "C"
BDINFO *ZnAllocateBlock(unsigned long bOrder, unsigned long znBasePref, ZNINFO *znInfo, ZNFLG znFlags, ZNSYS *znSys){
	ZNINFO *znAllocator = ZnChoose(bOrder, znBasePref, znFlags, znInfo, znSys);

	if(znAllocator == NULL)
		return (NULL);
	else {
		if(!FLAG_SET(znFlags, ZONE_NO_CACHE) && znSys->ZnCacheRefill != 0 && bOrder == 0) {
			SpinUnlock(&znAllocator->Lock);
			unsigned long chStatus;
			LinkedListNode *chData = ChDataAllocate(&znAllocator->Cache, &chStatus);

			if(FLAG_SET((unsigned long) chStatus, CH_POPULATE)) {
				SpinLock(&znAllocator->Lock);
				chStatus &= ~1;
				CHREG *chReg = (CHREG *) chStatus;
				unsigned long chDataSize = znAllocator->MmManager.DescriptorSize;

				BDINFO *chPopulater = BdAllocateBlock(znSys->ZnCacheRefill, &znAllocator->MmManager);
				if(chPopulater != NULL) {
					for(unsigned long dOffset=0; dOffset<(1 << znSys->ZnCacheRefill); dOffset++) {
						chPopulater->UpperOrder = 0;
						chPopulater->LowerOrder = 0;
						PushHead((LinkedListNode*) chPopulater, &chReg->DList);
						chPopulater = (BDINFO *) ((unsigned long) chPopulater + chDataSize);
					}
				}

				SpinUnlock(&znAllocator->Lock);
			}

			if(chData != NULL) {
				return (BDINFO*) (chData);
			} else {
				return (BDINFO*) (ChDataAllocate(&znAllocator->Cache, &chStatus));
			}
		}

		znAllocator->MmAllocated += (1 << bOrder);
		BDINFO *allocatedBlock = BdAllocateBlock(bOrder, &znAllocator->MmManager);
		SpinUnlock(&znAllocator->Lock);
		return (allocatedBlock);
	}
}*/

/**
 * Deallocates the memory allocated at the given buddy-block.
 *
 * @version 1.1.0
 * @since Circuit 2.03++
 * @author Shukant Pal
 */
void ZoneAllocator::freeBlock(BuddyBlock *blockGiven)
{
	Zone *owner = zoneTable + blockGiven->ZnOffset;
	owner->memoryAllocated -= SIZEOF_ORDER(blockGiven->Order);
	owner->memoryAllocator.freeBlock(blockGiven);
}

/**
 * Calls the constructors for the buddy-systems to configure this
 * zoning-system uniformly with the given arguments. After this, the
 * client must also define the zonal boundaries.
 *
 * @param entrySize - the size of the buddy-block descriptors
 * @param highestOrder - the max. allocation order supported by the allocator
 * @param listInfo - the array of "short" to store list meta-data
 * @param listArray - the array of linked-lists to store buddy-block chains
 * @param zoneTable - the array of zone-descriptors used in the zone-system
 * @param count - the no. of zones in this system
 * @version 1.0
 * @since Circuit 2.03++
 * @author Shukant Pal
 */
void ZoneAllocator::configureZones(unsigned long entrySize,
		unsigned long highestOrder, unsigned short *listInfo,
		LinkedList *listArray, Zone *zoneTable, unsigned long count)
{
	Zone *zone = zoneTable;
	class BuddyAllocator *buddySys = &zone->memoryAllocator;

	unsigned long liCount = BDSYS_VECTORS(highestOrder);
	unsigned long liSize = liCount + 1;

	for(unsigned long zoneIndex = 0; zoneIndex < count; zoneIndex++) {
		(void) new (buddySys) BuddyAllocator(entrySize, NULL,
				highestOrder, listInfo, listArray);
		listInfo += liSize;
		listArray += liCount;
		++(zone);
		buddySys = &zone->memoryAllocator;
	}
}

/**
 * Adds "count" zones from the zonal array given to the preference.
 * @param zoneArray
 * @param pref
 * @param count
 */
void ZoneAllocator::configurePreference(Zone *zoneArray,
		ZonePreference *pref, unsigned int count)
{
	unsigned int index = 0;
	while(index < count) {
		AddCElement((CircularListNode *) zoneArray,
				CLAST, &pref->ZoneList);
		++(index);
		++(zoneArray);
	}
}

/**
 * Configures the buddy-allocators used in the zones to sharply obey the
 * zonal boundaries presented by the client.
 *
 * @param zoneArray - array of zones used in the system
 * @param count - the no. of zones whose boundaries are to be defined
 */
void ZoneAllocator::configureZoneMappings(Zone *zoneArray, unsigned long count)
{
	unsigned char *blockPtr;
	unsigned long blockIndex;
	unsigned long blockCount;
	unsigned long blockEntrySz = zoneArray->memoryAllocator.getEntrySize();

	for(unsigned long index = 0; index < count; index++)
	{
		blockCount = zoneArray->memorySize;
		blockPtr = (UBYTE *) zoneArray->memoryAllocator.getEntryTable();
		for(blockIndex = 0; blockIndex < blockCount; blockIndex++)
		{
			((BuddyBlock *) blockPtr)->ZnOffset = index;
			blockPtr += blockEntrySz;
		}

		++(zoneArray);
	}
}

/*
extern "C"
BDINFO *ZnExchangeBlock(BDINFO *bdInfo, unsigned long *status, unsigned long znBasePref, unsigned long znFlags, ZNSYS *znSys){
	ZNINFO *zone = &znSys->ZnSet[bdInfo->ZnOffset];
	SpinLock(&zone->Lock);
	BDINFO *ebInfo = BdExchangeBlock(bdInfo, &(zone->MmManager), status); // @Prefix eb - Exchanged Block
	SpinUnlock(&zone->Lock);
	if(ebInfo == NULL) {
		return (ZnAllocateBlock(bdInfo->LowerOrder + 1, znBasePref, zone, znFlags, znSys));
	} else if((unsigned long) ebInfo == BD_ERR_FREE) {
		return (NULL);
	} else {
		return (ebInfo);
	}
}*/
