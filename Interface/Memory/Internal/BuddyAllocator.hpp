///
/// @file BuddyManager.h
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

#ifndef __MEMORY_BUDDY_MANAGER_H__
#define __MEMORY_BUDDY_MANAGER_H__

#include <KERNEL.h>
#include <Utils/LinkedList.h>

namespace Memory
{

/* Definitions */
#define MAX_BUDDY_ORDER 15

namespace Internal
{

#define BD_FREE 0
#define BD_LINKED 1

#define TBDFREE(bInfo) ((bInfo->DescriptorFlags >> BD_FREE) & 1)
#define TBDLINKED(bInfo) (bInfo->DescriptorFlags >> BD_LINKED & 1)

//! Declares the block as allocated allowing it to be given out!
#define BLOCK_UNFREE(bInfo)(bInfo->DescriptorFlags &= ~(1 << BD_FREE))

//! Declares the block as free allowing it to be taken in!
#define BLOCK_FREE(bInfo)(bInfo->DescriptorFlags |= (1 << BD_FREE))

//! Declares the block as "unlinked" and not occupied in any list.
#define BDUNLINK(bInfo)(bInfo->DescriptorFlags &= ~(1 << BD_LINKED))

//! Declares the block as "linked" and therefore owned by a list.
#define BDLINK(bInfo)(bInfo->DescriptorFlags |= (1 << BD_LINKED))

#define BDSYS_VECTORS(maxOrder) (maxOrder + 1) * (maxOrder + 4) / 2

#define LV_MAIN 0
#define LV_SUB(n) (1 + n)

///
/// Error-codes that are exposed by the BuddyAllocator to indicate
/// that an allocation/deallocation failure has occured.
///
enum BuddyResult
{
	BD_USED		= 0xBDF1,//!< BD_USED - block was occupied by another
	 	 	 	 //! list and thus it couldn't be freed.

	BD_ORDER_CORRUPT= 0xBDF2,//!< BD_ORDER_CORRUPT - the orders of the
	 	 	 	 //! given allocated-block didn't match and
	 	 	 	 //! they are corrupt stopping it from
	 	 	 	 //! being freed

	BD_MEMORY_LOW	= 0xBDA1,//!< BD_MEMORY_LOW - the allocator cannot
	 	 	 	 //! give a block of the order requested due to
	 	 	 	 //! low-on-memory conditions

	BD_MEMORY_OVERLOAD= 0xBDA2,//!< BD_MEMORY_OVERLOAD - the size of the
	 	 	 	 //! block request exceeds that of the whole
	 	 	 	 //! allocator's memory size

	BD_FRAGMENTATION= 0xBDA3,//!< BD_FRAGMENTATION - the allocation request
	 	 	 	 //! wasn't fulfilled due to fragmentation
	 	 	 	 //! rather than having less memory

	BD_ERR_FREE 	= 0xBDE1,//!< BD_ERR_FREE - the given block for
	 	 	 	 //! exchange wasn't allocated in the first
	 	 	 	 //! place

	BD_INTERNAL 	= 0xBDE2,//!< BD_INTERNAL - (not error) the block was
	 	 	 	 //! not exchanged by the allocator. A larger
	 	 	 	 //! block must be allocated

	BD_EXTERNAL 	= 0xBDE3,//!< BD_EXTERNAL - (not error) the block was
	 	 	 	 //! exchanged by the allocator and therefore
	 	 	 	 //! the caller need not ask for a new block

	BD_SUCCESS 	= 0xBDE  //!< BD_SUCCESS - (not error) operation done!
};

///
/// This type represents a memory descriptor, and is the smallest unit
/// allocable from the buddy system. Most of it is used by the buddy system
/// and should not be hampered with.
///
/// @version 1
/// @since Circuit 2.03++
/// @author Shukant Pal
///
struct BuddyBlock
{
	struct LinkedListNode ListLinker;
	UBYTE UpperOrder;
	union
	{
		UBYTE LowerOrder;
		UBYTE Order;
	};
	union
	{
		unsigned short DescriptorFlags;
		struct
		{
			unsigned short BdFree:1;
			unsigned short BdLinked:1;
			unsigned short ZnOffset:11;
			unsigned short BdType:3;
		};
	};
};


///
/// BuddyAllocator implements the buddy allocation algorithm and is the backend
/// for the zone allocator. It uses 'buddy descriptors' which can be of various
/// sizes with a header at the start of type 'struct BuddyBlock'.
///
/// The buddy allocator is internal to the memory subsystem and is not required
/// by code external the Core::Memory.
///
/// Version: 1.1 (before struct BuddyAllocator)
/// Since: Circuit 2.03++
/// Author: Shukant Pal
///
class BuddyAllocator final
{
public:
	BuddyAllocator() // @suppress("Class members should be properly initialized")
	{}

	BuddyAllocator(unsigned long entrySize, UBYTE *entryTable,
			unsigned long highestOrder, unsigned short *listInfo,
			LinkedList *blockLists);
	BuddyBlock *allocateBlock(unsigned long blockOrder);
	unsigned long freeBlock(BuddyBlock *);
	BuddyBlock *exchangeBlock(BuddyBlock *dataBlock,
			unsigned long *status);

	inline unsigned long getEntrySize(){ return (entrySize); }
	inline void setEntrySize(unsigned long entrySize){ this->entrySize = entrySize; }
	inline UBYTE *getEntryTable(){ return (entryTable); }
	inline void setEntryTable(UBYTE *entryTable){ this->entryTable = entryTable; }
private:

//! Size of block having the given order
#define SIZEOF_ORDER(n)(unsigned long)(1 << n)

//! Difference of the sizes of blocks having the given orders
#define SIZEOF_DIFF(u, l)(SIZEOF_ORDER(u) - SIZEOF_ORDER(l))

//! Pointer to the buddy-block descriptor at the given offset
#define BlockAtOffsetOf(orgBlock, offsetValue)((BuddyBlock *)((unsigned long) \
					orgBlock + offsetValue*entrySize))

	unsigned long entrySize;//!< Size of total size of the block-descriptor
				//!< provided by client

	unsigned char *entryTable;//!< Table containing entries of
	 	 	 	  //!< block-descriptors

	unsigned long highestOrder;//!< Highest order that can be allocated

	unsigned short *listInfo;//!< List bit-field having capacity till
	 	 	 	 //!< highestOrder (supplied by client)

	LinkedList *blockLists;//!< Array of linked-list for storing
			       //!< buddy blocks; given by client

	unsigned long freeBuddies;//!< Number of blocks free in this allocator

	unsigned long allocatedBuddies;//!< Number of allocated blocks

	BuddyBlock *getBuddyBlock(unsigned long blockOrder, struct BuddyBlock *) kxhide;
	LinkedList *getBuddyList(unsigned long optimalOrder) kxhide;
	LinkedList *getBuddyList(unsigned long lowerOrder, unsigned long upperOrder) kxhide;
	LinkedList *getBuddyList(BuddyBlock *) kxhide;
	Void addBuddyBlock(BuddyBlock *) kxhide;
	Void removeBuddyBlock(BuddyBlock *) kxhide;
	Void removeBuddyBlock(BuddyBlock *, LinkedList *) kxhide;
	BuddyBlock *splitSuperBlock(unsigned long newOrder, BuddyBlock *bInfo, BuddyBlock **lowerSuperBlock, BuddyBlock **upperSuperBlock) kxhide;
	BuddyBlock *mergeSuperBlock(BuddyBlock *, unsigned long maxBlockOrder) kxhide;
};

}// namespace Internal

}// namespace Memory

#endif/* Memory/BuddyManager.h */
