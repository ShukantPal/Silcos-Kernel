/**
 * Copyright (C) 2017 - Shukant Pal
 *
 * The cache register is a data structure used in the Processor to list (LIFO)
 * zero-order data (pages). It is an extension inside the zone allocator.
 */
#ifndef __MEMORY_CACHE_REGISTER_H
#define __MEMORY_CACHE_REGISTER_H

#include <TYPE.h>
#include "../Utils/LinkedList.h"

/**
 * CHREG - 
 *
 * Summary:
 * This type is used to maintain the per-CPU cache register. It represent one
 * CPU cache listing the pages for that CPU. It is kept seperate for each CPU.
 *
 * Variables:
 *
 * @Version 1
 * @Since Circuit 2.03
 */
typedef
struct CacheRegister
{
	union
	{
		struct LinkedList DList;
		unsigned long DCount;
	};
} CHREG;

#define CH_POPULATE 0 /* Populate the cache, because 0 pages are left */

/**
 * CHSYS -
 *
 * Summary:
 * This type represents a collection of caches for every CPU. It represent all
 * caches and their settings.
 *
 * Variables:
 * ChOffset - The offset of the cache registers in the per-CPU data
 * ChPrefSize - Preferred size for the cache
 * ChSize - Cache size (max. data to be cached)
 *
 * @Version 1
 * @Since Circuit 2.03
 */
typedef
struct _CHSYS {
	unsigned long ChMemoryOffset;
} CHSYS;

/**
 * CacheAllocate() - 
 *
 * Summary:
 * This function allocates data from the per-CPU cache register (for the current
 * CPU), and returns its LIST_ELEMENT * header. It may also return  NULL, in case
 * of a empty cache.
 *
 * Args:
 * chInfo - The caching system being used
 * statusFilter - Used for getting cache register notifications
 *
 * Returns:
 * The LIELEM header of the data block.
 *
 * @Version 1
 * @Since Circuit 2.03
 */
LinkedListNode *ChDataAllocate(
	CHSYS *chInfo,
	unsigned long *statusFilters
);

LinkedListNode *ChDataFree(
	LinkedListNode *dInfo,
	CHSYS *chInfo
);

#endif /* Memory/CacheRegister.h */
