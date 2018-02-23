///
/// @file ZoneAllocator.hpp
/// -------------------------------------------------------------------
/// This program is free software: you can redistribute it and/or modify
/// it under the terms of the GNU General Public License as published by
/// the Free Software Foundation, either version 3 of the License, or
/// (at your option) any later version.
///
/// This program is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
/// GNU General Public License for more details.
///
/// You should have received a copy of the GNU General Public License
/// along with this program.  If not, see <http://www.gnu.org/licenses/>
///
/// Copyright (C) 2017 - Shukant Pal
///
#ifndef MEMORY_ZONE_MANAGER_H
#define MEMORY_ZONE_MANAGER_H

#include "BuddyAllocator.hpp"
#include "CacheRegister.h"
#include <Synch/Spinlock.h>
#include <Utils/CircularList.h>
#include <Utils/Stack.h>

typedef unsigned long ZNPNO;

///
/// A number of zones can belong to the same preference level. If a zone is
/// in the OutOfMemory state then memory can be allocated from the zone of the
/// same preference. But if all zones of same preference cannot allocate then
/// lower-preference zones are checked. Higher preference zones are never
/// checked in this case.
///
/// @version 1.0
/// @since Circuit 2.03++
/// @author Shukant Pal
///
typedef
struct ZonePreference
{
	LinkedListNode LiLinker;
	ZNPNO ZnPref;
	CircularList ZoneList;
} ZNPREF;

#define ATOMIC	0
#define NO_FAILURE	1
#define ZONE_REQUIRED	2
#define ZONE_NO_CACHE	3
#define BD_RESULT	31

#define FLG_ATOMIC	(1 << ATOMIC)
#define FLG_NO_FAIL	(1 << NO_FAILURE)
#define FLG_ZN_REQUIRED	(1 << ZONE_REQUIRED)
#define FLG_NOCACHE	(1 << ZONE_NO_CACHE)
#define FLG_NONE 	0

typedef unsigned long ZNFLG;

namespace Memory
{

namespace Internal
{

typedef unsigned long ZoneControl;

///
/// Represents a division in memory that is separately allocated. Dividing
/// address-spaces into zones increases the scalability of allocation and
/// reduces Spinlock usage.
///
/// @version 1.0
/// @since Circuit 2.03++
/// @author Shukant Pal
///
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
	Spinlock controlLock;
};


///
/// This class is used as the backend for memory-allocators based on the zone
/// allocation algorithm & buddy-algorithm. It uses of the concept of memory
/// zones which contain memory for separate purposes.
///
/// To use the ZoneAllocator you must provide -
///
/// 1. Block Descriptor Table - There must be an array of Internal::BuddyBlock
/// entries for the whole memory managed by the ZoneAllocator. This is
/// internally split between the zones.
///
/// 2. Zone Preference Table - An array of ZonePreference with each element
/// having a circular-buffer of zones that are inter-allocable.
///
/// 3. Zone Table - An array of zones that is used to split the memory. These
/// are also linked in circular buffers in ZonePreference elements, but note
/// that zones in the same preference need not be adjacent.
///
/// @version 1.2
/// @since Circuit 2.03++
/// @author Shukant Pal
///
class ZoneAllocator final
{
public:
	void resetAllocator(BuddyBlock *entryTable, ZonePreference *prefTable,
			unsigned long prefCount, Zone *zoneTable,
			unsigned long zoneCount);
	void resetStatistics();
	BuddyBlock *allocateBlock(unsigned long requiredOrder,
			unsigned long prefBase, Zone *prefZone,
			ZNFLG allocFlags);
	void freeBlock(BuddyBlock *givenBlock);
	BuddyBlock *exchangeBlock(BuddyBlock *orgBlock,
			unsigned long *statusReg, unsigned long prefBase,
			ZNFLG allocFlags);
	static void configureZones(unsigned long entrySize,
			unsigned long highestOrder, unsigned short *listInfo,
			LinkedList *listArray, Zone *zoneTable,
			unsigned long count);
	static void configurePreference(Zone *zoneArray, ZonePreference *pref,
			unsigned int count);
	static void configureZoneMappings(Zone *zoneTable,
			unsigned long zoneCount);
private:
	BuddyBlock *descriptorTable;
	ZonePreference *prefTable;
	unsigned long prefCount;
	Zone *zoneTable;
	unsigned long zoneCount;

	Zone *getZone(unsigned long blockOrder, unsigned long basePref,
			ZNFLG allocFlags, Zone *prefZone) kxhide;
};

}
}

#endif /* Memory/ZoneManager.h */
