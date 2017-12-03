/**
 * Copyright (C) 2017 - Shukant Pal
 */
#ifndef __MEMORY_BUDDY_MANAGER_H__
#define __MEMORY_BUDDY_MANAGER_H__

#include <Util/LinkedList.h>
#include <TYPE.h>

namespace Memory
{

/* Definitions */
#define MAX_BUDDY_ORDER 15

namespace Internal
{

/* Flag offsets */
#define BD_FREE 0
#define BD_LINKED 1

/* Flag readers */
#define TBDFREE(bInfo) ((bInfo->DescriptorFlags >> BD_FREE) & 1)
#define TBDLINKED(bInfo) (bInfo->DescriptorFlags >> BD_LINKED & 1)

/* Flag manipulators */
#define BLOCK_UNFREE(bInfo) (bInfo->DescriptorFlags &= ~(1 << BD_FREE)) /* Unset the free flag */
#define BLOCK_FREE(bInfo) (bInfo->DescriptorFlags |= (1 << BD_FREE)) /* Set the free flag */
#define BDUNLINK(bInfo) (bInfo->DescriptorFlags &= ~(1 << BD_LINKED)) /* Unset the linked flag */
#define BDLINK(bInfo) (bInfo->DescriptorFlags |= (1 << BD_LINKED)) /* Set the linked flag */

#define BDSYS_VECTORS(maxOrder) (maxOrder + 1) * (maxOrder + 4) / 2

#define LV_MAIN 0
#define LV_SUB(n) (1 + n)

enum BuddyResult
{
	BD_USED 			= 0xBDF1,
	BD_ORDER_CORRUPT 	= 0xBDF2,
	BD_MEMORY_LOW 		= 0xBDA1,
	BD_MEMORY_OVERLOAD 	= 0xBDA2, /* Not implemented */
	BD_FRAGMENTATION 	= 0xBDA3,
	BD_ERR_FREE 		= 0xBDE1,
	BD_INTERNAL 		= 0xBDE2,
	BD_EXTERNAL 		= 0xBDE3,
	BD_SUCCESS 			= 0xBDE
};

/**
 * Struct: BuddyBlock
 *
 * Summary:
 * This type represents a memory descriptor, and is the smallest unit
 * allocable from the buddy system. Most of it is used by the buddy system
 * and should not be hampered with.
 *
 * Version: 1
 * Since: Circuit 2.03++
 * Author: Shukant Pal
 */
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
		USHORT DescriptorFlags;
		struct
		{
			USHORT BdFree:1;
			USHORT BdLinked:1;
			USHORT ZnOffset:11;
			USHORT BdType:3;
		};
	};
};


/**
 * Class: BuddyAllocator
 *
 * Summary:
 * BuddyAllocator implements the buddy allocation algorithm and is the backend
 * for the zone allocator. It uses 'buddy descriptors' which can be of various
 * sizes with a header at the start of type 'struct BuddyBlock'.
 *
 * The buddy allocator is internal to the memory subsystem and is not required
 * by code external the Core::Memory.
 *
 * Version: 1.1 (before struct BuddyAllocator)
 * Since: Circuit 2.03++
 * Author: Shukant Pal
 */
class BuddyAllocator final
{
public:
	BuddyAllocator();
	BuddyAllocator(ULONG entrySize, UBYTE *entryTable, ULONG highestOrder, USHORT *listInfo, LINKED_LIST *blockLists);
	struct BuddyBlock *allocateBlock(unsigned long blockOrder);
	unsigned long freeBlock(struct BuddyBlock *);
	struct BuddyBlock *exchangeBlock(struct BuddyBlock *dataBlock, ULONG *status);

	inline ULONG getEntrySize(){ return entrySize; }
	inline void setEntrySize(ULONG entrySize){ this->entrySize = entrySize; }
	inline UBYTE *getEntryTable(){ return entryTable; }
	inline void setEntryTable(UBYTE *entryTable){ this->entryTable = entryTable; }
private:
	#define SIZEOF_ORDER(n) (unsigned long)(1 << n) // Size of order-block n
	#define SIZEOF_DIFF(u, l) (SIZEOF_ORDER(u) - SIZEOF_ORDER(l)) // Diff. b/w two blocks of order u,l
	#define BlockAtOffsetOf(orgBlock, offsetValue) ((struct BuddyBlock *) ((ULONG) orgBlock + offsetValue * entrySize))

	unsigned long entrySize;// Size of total size of block-descriptor (including BuddyBlock)
	unsigned char *entryTable;// Table containing entries of block-descriptions
	unsigned long highestOrder;// Highest order that can be allocated
	unsigned short *listInfo;// List bit-field having capacity till highestOrder (supplied by client)
	LinkedList *blockLists;// Lists for containing block-descriptions (supplied by client)
	unsigned long freeBuddies;// Blocks available in the allocator
	unsigned long allocatedBuddies;// Blocks that have been pushed out of the allocator

	struct BuddyBlock *getBuddyBlock(ULONG blockOrder, struct BuddyBlock *);
	struct LinkedList *getBuddyList(ULONG optimalOrder);
	struct LinkedList *getBuddyList(ULONG lowerOrder, ULONG upperOrder);
	struct LinkedList *getBuddyList(struct BuddyBlock *);
	Void addBuddyBlock(struct BuddyBlock *);
	Void removeBuddyBlock(struct BuddyBlock *);
	Void removeBuddyBlock(struct BuddyBlock *, struct LinkedList *);
	struct BuddyBlock *splitSuperBlock(ULONG newOrder, struct BuddyBlock *bInfo, struct BuddyBlock **lowerSuperBlock, struct BuddyBlock **upperSuperBlock);
	struct BuddyBlock *mergeSuperBlock(struct BuddyBlock *, ULONG maxBlockOrder);
};

} // namespace Internal

} // namespace Memory

#endif /* Memory/BuddyManager.h */
