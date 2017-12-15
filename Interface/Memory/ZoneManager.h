/**
 * File: ZoneManager.h
 *
 * Summary:
 * The zone manager is another layer ontop of the buddy manager. It is used for the
 * organization of memory into seperate partitions from which memory may be forced
 * to be allocated. It allows seperation of DMA, Normal Memory and High Memory. Also,
 * CODE, DATA and KERNEL zones are used for allocation of specific memory needs. This
 * allows better cache utilization for CODE and DATA.
 *
 * The zone manager uses three types - ZNINFO, ZNPREF, and ZNSYS. The ZNINFO type
 * represents the zone, whereas ZNPREF represents a collection of ZONEs of equal preference,
 * and ZNSYS represents the whole zone system. An example of ZONE preferences is that
 * CODE and DATA can be mixed and are of the same preference, but DMA and NORMAL memory
 * cannot be mixed (if allocating for DMA device driver).
 *
 * DMA - 0 to 16 MB
 * NORMAL - 16 MB to 4GB (not a zone)
 * HIGH - 4GB n above (not a zone)
 *
 * One constraint of the zone allocator, is that is required explicit zone specification, during both
 * freeing and allocating. This is done by the ZnOffset field of the buddy info type.
 *
 * Copyright (C) 2017 - Shukant Pal
 */
#ifndef MEMORY_ZONE_MANAGER_H
#define MEMORY_ZONE_MANAGER_H

#include "BuddyManager.h"
#include "CacheRegister.h"
#include <Synch/Spinlock.h>
#include <Util/Stack.h>
#include <Util/CircularList.h>

typedef unsigned long ZNPNO;

/**
 * Type: ZNPREF 
 *
 * Summary:
 * The ZNPREF type represent the zone preference feature of the zone manager. Zones are kept in
 * collections of same preferences. Zones of the same preference (e.g. CODE and DATA) allow mixed
 * allocation. But zones of higher preference can allocated from zones of lower preference (e.g. NORMAL
 * memory can allocate from DMA) but not vice versa.
 *
 * This is an extra feature and is not a part of the linux kernel zone allocator.
 *
 * Variables:
 * LiLinker - Used for participating in the list
 * ZnPref - Zone preference field (and also its index in the array)
 * ZnStack - Stack of zones of this preference
 *
 * @Version 1
 * @Since Circuit 2.03
 */
typedef
struct ZonePreference
{
	LIST_ELEMENT LiLinker;
	ZNPNO ZnPref;
	CLIST ZoneList;
} ZNPREF;

#define ATOMIC 0
#define NO_FAILURE 1
#define ZONE_REQUIRED 2
#define ZONE_NO_CACHE 3
#define BD_RESULT 31

#define FLG_ATOMIC (1 << ATOMIC)
#define FLG_NO_FAIL (1 << NO_FAILURE)
#define FLG_ZN_REQUIRED (1 << ZONE_REQUIRED)
#define FLG_NOCACHE (1 << ZONE_NO_CACHE)
#define FLG_NONE 0

typedef unsigned long ZNFLG;

namespace Memory
{

namespace Internal
{

typedef unsigned long ZoneControl;

struct Zone
{
	union
	{
		struct CircularListNode liLinker;
		struct
		{
			struct Zone *nextZone;
			struct Zone *previousZone;
		};
	};
	class BuddyAllocator memoryAllocator;
	unsigned long preferenceIndex;
	unsigned long memorySize;
	unsigned long memoryReserved;
	unsigned long memoryAllocated;
	SPIN_LOCK controlLock;
};


/**
 * Class: ZoneAllocator
 * Attributes: final
 *
 * Summary:
 * This class is used as the backend for memory-allocators based on the zone
 * allocation algorithm & buddy-algorithm. It uses of the concept of memory
 * zones which contain memory for separate purposes.
 *
 * ZoneAllocator has the following prerequisites for its functional implementation -
 *
 * 1. Buddy-Block Entry Table - A table which contains a table of entries, each
 * containing information about a memory-block (e.g. Page) which is a unit of
 * allocation.
 *
 * 2. Zone Preference Table - A table containing ZonePreference entries which
 * contain a circular list of memory-zones descriptors that are have highly
 * mix-able memory. If a zone is low on memory, then first a zone of same
 * preference will be tried for allocation, then the allocator will go to lower
 * preferences.
 *
 * 3. Zone Table - A table containing zone descriptor entries. It is a good practice
 * to have zones containing memory-adjacent to each other to be adjacent to each other
 * in the zone-table.
 *
 * Each memory-zone has a separate buddy-allocator associated with it, which is used
 * for allocating memory in it.
 *
 * Version: 1.2
 * Since: Circuit 2.03++
 * Author: Shukant Pal
 */
class ZoneAllocator final
{
public:
	void resetAllocator(struct BuddyBlock *entryTable, struct ZonePreference *prefTable, unsigned long prefCount, struct Zone *zoneTable, unsigned long zoneCount);
	void resetStatistics();

	// Allocator function
	struct BuddyBlock *allocateBlock(unsigned long requiredOrder, unsigned long prefBase, struct Zone *prefZone, ZNFLG allocFlags);

	// 'Free' the block function
	Void freeBlock(struct BuddyBlock *givenBlock);

	// Exchange-frontend for buddy-systems
	struct BuddyBlock *exchangeBlock(struct BuddyBlock *orgBlock, unsigned long *statusReg, unsigned long prefBase, ZNFLG allocFlags);

	static void configureZones(unsigned long entrySize, unsigned long highestOrder, unsigned short *listInfo, LinkedList *listArray, struct Zone *zoneTable, unsigned long count);
	static void configurePreference(struct Zone *zoneArray, struct ZonePreference *pref,  unsigned int count);
	static void configureZoneMappings(struct Zone *zoneTable, unsigned long zoneCount);

private:
	struct BuddyBlock *descriptorTable;
	struct ZonePreference *prefTable;
	unsigned long prefCount;
	struct Zone *zoneTable;
	unsigned long zoneCount;

	struct Zone *getZone(unsigned long blockOrder, unsigned long basePref, ZNFLG allocFlags, struct Zone *prefZone);
};

}
}

#endif /* Memory/ZoneManager.h */
