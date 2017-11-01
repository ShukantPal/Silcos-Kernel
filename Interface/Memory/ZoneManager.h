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

/**
 * Type: ZNINFO 
 *
 * Summary:
 * This type defines the parameters identifying a zone with its allocation system. The zone
 * always uses a associated buddy manager along with a cache register. The buddy system is
 * used to allocate pages from the main BDINFO table. The cache register uses the per-CPU
 * dynamic variables to manage caches for each CPU.
 * 
 * A memory zone is actually a abstract idea of an prefered memory region from which the pages
 * must be/should be allocated from. It allows the organization of memory types like CODE, DATA,
 * and KERNEL memory. Also, DMA memory must be kept seperate for legacy devices.
 *
 * Variables:  
 * LiLinker - Used for participating in the list
 * MmManager - Buddy system for the zone memory
 * ZnPref - Number identified the preference field
 * MmSize - Size of the zone's memory
 * MmReserved - Size of reserved memory
 * MmAllocated - Size of the allocated memory, currently
 * Cache - Caching System
 * Lock - 
 *
 * @Version 2.1 (earlier ZONE)
 * @Since Circuit 2.03
 */
typedef
struct Zone_
{
	union {
		CLNODE LiLinker;
		struct {
			struct Zone_ *RightLinker;
			struct Zone_ *LeftLinker;
		};
	};
	struct BuddyAllocator_ MmManager;
	ULONG ZnPref;
	ULONG MmSize;
	ULONG MmReserved;
	ULONG MmAllocated;
	CHSYS Cache;
	SPIN_LOCK Lock;
} ZNINFO;

typedef ULONG ZNPNO;

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

/**
 * Type: ZNSYS
 *
 * Summary:
 * This replaces the whole zone allocator. It contains the data for maintaining a zoning system.
 *
 * Variables:
 * ZnPref - ZNPREF array
 * ZnPrefCount - No. of zone preferences
 * ZnPrefBase - Offset of the first zone
 *
 * @Deprecated
 * @Version 1
 * @Since Circuit 2.03
 */
typedef
struct ZoneAllocator_
{
	BDINFO *BdITable;
	ZNPREF *ZnPref;
	ZNINFO *ZnSet;
	ULONG ZnPrefCount;
	ULONG ZnPrefBase;
	ULONG ZnCacheRefill;
} ZNSYS;

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

typedef ULONG ZNFLG;

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
	void resetAllocator(struct BuddyBlock *entryTable, struct ZonePreference *prefTable, ULONG prefCount, struct Zone *zoneTable, unsigned long zoneCount);
	void resetStatistics();

	// Allocator function
	struct BuddyBlock *allocateBlock(ULONG requiredOrder, ULONG prefBase, struct Zone *prefZone, ZNFLG allocFlags);

	// 'Free' the block function
	Void freeBlock(struct BuddyBlock *givenBlock);

	// Exchange-frontend for buddy-systems
	struct BuddyBlock *exchangeBlock(struct BuddyBlock *orgBlock, ULONG *statusReg, ULONG prefBase, ZNFLG allocFlags);

	static void configureZones(ULONG entrySize, ULONG highestOrder, USHORT *listInfo, LINKED_LIST *listArray, struct Zone *zoneTable, ULONG count);
	static void configurePreference(struct Zone *zoneArray, struct ZonePreference *pref,  UINT count);
	static void configureZoneMappings(struct Zone *zoneTable, unsigned long zoneCount);

private:
	struct BuddyBlock *descriptorTable;
	struct ZonePreference *prefTable;
	unsigned long prefCount;
	struct Zone *zoneTable;
	unsigned long zoneCount;

	struct Zone *getZone(ULONG blockOrder, ULONG basePref, ZNFLG allocFlags, struct Zone *prefZone);
};

}
}

/**
 * Function: ZnAllocateBlock()
 *
 * Summary:
 * This function tries to allocate memory from the specific memory zone. If it is not possible, then
 * it tries to go to other zones in order of the preference levels. It also uses the minimum preference
 * level, to limit zone switching. Flags are provided to manipulate zone level allocation.
 *
 * Args:
 * bOrder - Order of the memory block required 
 * znBasePref - Minimum zone preference level
 * znInfo - Zone from which allocation is preferred first
 * znFlags - Flags controlling the allocation
 * znSys - The zoning system being used 
 *
 * Changes:
 * This function changes the zone's memory status, from which memory is allocated OR it may take a
 * shortcut and use the per-CPU caching system.
 *
 * @Version 1
 * @Since Circuit 2.03
 * @See ZNCFLG
 * @Author Shukant Pal
 */
decl_c BDINFO *ZnAllocateBlock(
	ULONG bOrder,
	ULONG znBasePref,
	ZNINFO *znInfo,
	ZNFLG znFlags,
	ZNSYS *znSys
);

/**
 * Function: ZnFreeBlock()
 *
 * Summary:
 * This function frees the memory allocated using its buddy info. It finds the corresponding zone by the
 * supplementary field (ZnOffset). No zone switching is actually required.
 *
 * Args:
 * bInfo - The buddy info returned while allocating
 * znSys - The zone system being used 
 *
 * Changes:
 * The zone memory status.
 *
 * @Version 1
 * @Since Circuit 2.03
 * @Author Shukant Pal
 */
decl_c
VOID ZnFreeBlock(
	BDINFO *bInfo,
	ZNSYS *znSys
);

/**
 * Function: ZnExchangeBlock()
 *
 * Summary:
 * This function implements the exchanging functionality of the buddy manager. It will allocate from the zoning
 * system, if the original block is not extended.
 *
 * Args:
 * bInfo - The block to extend
 * status - Status info ptr
 * znBasePref - Base preference for allocation
 * znFlags - Zone-allocation flags
 * zone - Zone to use
 * znSys - Zoning system being used
 *
 * Changes:
 * It changes the buddy system's of zone used for extension/allocation.
 *
 * @Version 1
 * @Since Circuit 2.03
 * @Author Shukant Pal
 */
decl_c
BDINFO *ZnExchangeBlock(
	BDINFO *bInfo,
	ULONG *status,
	ULONG znBasePref,
	ULONG znFlags,
	ZNSYS *znSys
);

/**
 * Function: ZnReverseMap()
 *
 * Summary:
 * This is required for setting the zone offset field of every buddy info, coming in the memory zone given. It uses
 * the offset, size, and address of the zone to technically reverse map the zone.
 *
 * Args:
 * znOffset - Offset of the zone
 * znInfo - Required zone
 * znSys - The zoning system being used 
 *
 * Changes: 
 * The buddy info(s).
 *
 * @Version 1
 * @Since Circuit 2.03
 * @Author Shukant Pal
 */
decl_c
VOID ZnReverseMap(
	USHORT znOffset,
	ZNINFO *znInfo,
	ZNSYS *znSys
);

#endif /* Memory/ZoneManager.h */
