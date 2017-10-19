/**
 * Copyright (C) 2017 - Shukant Pal
 *
 * The buddy allocator presents a generic-interface to seperate the allocation
 * algorithms, layer-wise to organize and reduce code. The memory allocators
 * (physical and kernel memory) use this system to share the allocation code.
 * Actually, the buddy allocator is a layer beneath the ZONE allocator and
 * CACHE-based allocator.
 *
 * The buddy system has been modified in two crucial ways - 
 *
 * 1- Usage of superblocks - A superblock is memory chunk having a size
 * equal to the DIFFERENCE of two powers-of-two (2^k - 2^i). They are used
 * to reduce the number of splits and merges required.
 *
 * 2- Multiple lists per block size - For a block of size 2^k, (k+1) number of
 * (2^k - 2^i) superblocks are available. A seperate list is kept for each type,
 * arising the need for multiple block lists.
 *
 * The BDSYS type is used to represent the buddy system and its statistical
 * data. It requires the block lists, list infos, and the buddy info descriptors. This
 * is quite less for usage with PAGE_SIZE memory blocks.
 *
 * The BDINFO type is the core type used by the allocator. Its size is actually
 * modifiable for the system, by the DescriptorSize variable. The fields except
 * DescriptorFlags (only 2-bits used) are totally used by the system. They are
 * not supposed to be modified, or else it will lead to the failure of the buddy
 * system.
 *
 * @Version 1.1
 * @Since Circuit 2.03
 */

#ifndef MEMORY_BUDDY_MANAGER_H
#define MEMORY_BUDDY_MANAGER_H

#include <Util/LinkedList.h>
#include <TYPE.h>

/* Definitions */
#define MAX_BUDDY_ORDER 15

/* Flag offsets */
#define BD_FREE 0
#define BD_LINKED 1

#define BD_USED 0xBDF1
#define BD_ORDER_CORRUPT 0xBDF2
#define BD_MEMORY_LOW 0xBDA1
#define BD_MEMORY_OVERLOAD 0xBDA2 /* Not implemented */
#define BD_FRAGMENTATION 0xBDA3
#define BD_ERR_FREE 0xBDE1
#define BD_INTERNAL 0xBDE2
#define BD_EXTERNAL 0xBDE3
#define BD_SUCCESS 0xBDE

/* Flag readers */
#define TBDFREE(bInfo) ((bInfo->DescriptorFlags >> BD_FREE) & 1)
#define TBDLINKED(bInfo) (bInfo->DescriptorFlags >> BD_LINKED & 1)

/* Flag manipulators */
#define BLOCK_UNFREE(bInfo) (bInfo->DescriptorFlags &= ~(1 << BD_FREE)) /* Unset the free flag */
#define BLOCK_FREE(bInfo) (bInfo->DescriptorFlags |= (1 << BD_FREE)) /* Set the free flag */
#define BDUNLINK(bInfo) (bInfo->DescriptorFlags &= ~(1 << BD_LINKED)) /* Unset the linked flag */
#define BDLINK(bInfo) (bInfo->DescriptorFlags |= (1 << BD_LINKED)) /* Set the linked flag */

#define BDSYS_VECTORS(maxOrder) (maxOrder + 1) * (maxOrder + 4) / 2

/**
 * BDINFO - 
 *
 * Summary:
 * This type represents a memory descriptor, and is the smallest unit
 * allocable from the buddy system. Most of it is used by the buddy system
 * and should not be hampered with.
 *
 * Variables:
 * ListLinker - This is used to participate in block lists.
 * UpperOrder - This is the upper order of the super block.
 * LowerOrder - This is the lower order of the super block.
 * DescriptorFlags - Only 2-bits are used for keeping flags BD_FREE and BD_LINKED.
 * BdFree - If the buddy is free, or not
 * BdLinked - If the buddy is linked a buddy list, or not
 * ZnOffset - Reserved for the zone allocator (not used)
 * BdType - Client-driven value, for recognizing the block type (not used).
 *
 * @Version 1
 * @Since Circuit 2.03
 */
typedef
struct _BDINFO {
	LIST_ELEMENT ListLinker;
	UBYTE UpperOrder;
	UBYTE LowerOrder;
	union {
		USHORT DescriptorFlags;
		struct {
			USHORT BdFree:1;
			USHORT BdLinked:1;
			USHORT ZnOffset:11;
			USHORT BdType:3;
		};
	};
} BDINFO;

/**
 * BDSYS -
 *
 * Summary:
 * This type is used to contain data for the buddy system. It is read-only for external
 * code. The fields are filled only once and should not be changed during runtime. The
 * variables are strongly used by the allocator code.
 * 
 * Variables:
 * DescriptorSize - Size of the memory descriptor (BDINFO)
 * DescriptorTable - Table of memory descriptors (BDINFO)
 * HighestOrder - The highest order of allocation allowed (read-only)
 * ListInfo - The information array containing bits for each list
 * BlockLists - The array of lists used for containing free blocks
 * OpFree - Total number of free units in the system, currently
 * OpAllocated - Total number of allocated units in the system, from starting (modifiable to 0)
 *
 * @Version 1.1
 * @Since Circuit 2.03
 */
typedef
struct _BDSYS {
	ULONG DescriptorSize; /* Size of memory descriptor */
	UBYTE *DescriptorTable; /* Table of memory descriptors */
	ULONG HighestOrder; /* Max-block allocation order of this allocator */
	USHORT *ListInfo; /* List info contains bits for various PRESENT/NOT-PRESENT for each block type. */
	LINKED_LIST *BlockLists; /* Array of lists having atleast HighestOrder lists.*/
	ULONG OpFree; /* Currently free buddies in the system */
	ULONG OpAllocated; /* Total buddies allocated from the system */
} BDSYS;

/**
 * BdAllocateBlock() - 
 *
 * Summary:
 * This function looks for a superblock in two steps - 
 * 1- Search for superblock with a block of required size.
 * 2- If not found, then search for a superblock of size greater than
 *	  required size.
 * Once a superblock is found, it is split into upto three pieces and
 * the fitting block is returned. The other two pieces are added into
 * the buddy system's lists for use later.
 *
 * Args:
 * bOrder -  Order of the memory size required
 * bSys - Buddy System of the context
 *
 * Changes:
 * This function can change the buddy lists in the system given.
 *
 * Exceptions:
 * BD_MEMORY_LOW: Memory is too low to allocate the block.
 * BD_MEMORY_OVERLOAD: Rarely given, to show that allocation is greater than total memory.
 * BD_FRAGMENTATION: Memory is fragmented to allocate a contigous block so large.
 *
 * @Version 1
 * @Since Circuit 2.03
 */
BDINFO *BdAllocateBlock(
	ULONG bOrder,
	BDSYS *bSys
);

/**
 * BdFreeBlock() - 
 *
 * Summary:
 * This function simply uses the given block for merging and getting a
 * block as large as possible. The block is automatically added to the
 * buddy lists during merging. It is just made free by setting the BD_FREE
 * flag.
 *
 * Args:
 * BDiNFO *bInfo - The block to be freed
 * BDSYS *bdSys - The buddy system to be used
 *
 * Exceptions:
 * BD_USED: Linked flag is still set (not allowed)
 * BD_CORRUPT_ORDER: The orders of the block aren't correct
 *
 * Success Return:
 * BD_SUCCESS
 *
 * @Version 1
 * @Since Circuit 2.03
 */
ULONG BdFreeBlock(
	BDINFO *bInfo,
	BDSYS *bdSys
);

/**
 * BdExchangeBlock() - 
 *
 * Summary:
 * This function, simply, extends or exchanges the current block with a new block
 * by doubling it. If the block cannot be enlarged, then a new block of order+1
 * is allocated. It allows atomic extension of various data structures, and if the
 * block can be extended directly, it saves memory transfer overhead.
 *
 * Args:
 * bInfo - The original block, which needs extension
 * bdSys - The buddy system, being used
 * status - Return information, if the original block was freed or not
 *
 * Exceptions:
 * BD_FREE - If the block is not allocated, and you are trying to exchange it
 * 
 * Returns: The larger block, which is exchanged or allocated
 *
 * Changes:
 * status - BD_INTERNAL iff the org. block not extended, else BD_EXTERNAL
 *
 * @Version 1
 * @Since Circuit 2.03
 */
BDINFO *BdExchangeBlock(
	BDINFO *bInfo,
	BDSYS *bdSys,
	ULONG *status
);

#endif /* Memory/BuddyManager.h */
