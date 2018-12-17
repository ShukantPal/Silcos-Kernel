/**
 * @file BuddyAllocator.cpp
 *
 * The buddy-allocator, usually used as back-end to the
 * zone-allocator, is generically implemented to use
 * page-frame/page/buddy-block descriptors allocated by the client.
 * Having a fixed size, they start with a predefined <tt>BuddyBlock</tt>
 * header for this object.
 *
 * This implementation uses the recommended optimizations to the binary
 * buddy allocator - using the "superblock" concept, and using multiple
 * lists each for (lowerOrder, upperOrder) superblocks.
 *
 * NOTE:
 * Extensions to buddy-allocator are not supported yet (cv-2.03,++)
 *
 * Copyright (C) 2017 - Shukant Pal
 */
#include <Memory/Pager.h>
#include <Memory/Internal/BuddyAllocator.hpp>
#include <KERNEL.h>

using namespace Memory;
using namespace Memory::Internal;

/**
 * Initializes the buddy-allocator with the given arguments. This
 * doesn't render the allocator as usable, though, as all blocks
 * have to be freed after this to allow them to be allocated
 * afterwards.
 *
 * @param entrySize - size of the block-descriptors
 * @param entryTable - pointer to a table of block-descriptors
 * @param highestOrder - highest-order block which can be allocated
 * @param listInfo - array of U16 bitmaps for each buddy-list
 * @param buddyLists - array of linked-lists to store the block
 * 			descriptors, given by <tt>BDSYS_VECTORS</tt>
 * @version 1.0
 * @since Circuit 2.03++
 * @author Shukant Pal
 */
BuddyAllocator::BuddyAllocator(unsigned long entrySize,
		unsigned char *entryTable, unsigned long highestOrder,
		unsigned short *listInfo, LinkedList *buddyLists)
{
	this->entrySize = entrySize;
	this->entryTable = entryTable;
	this->highestOrder = highestOrder;
	this->listInfo = listInfo;
	this->blockLists = buddyLists;
	this->freeBuddies = 0;
	this->allocatedBuddies = 0;
}

/**
 * Used for getting the buddy of the given descriptor.
 *
 * @param blockOrder - order at which buddy is to be calculated
 * @param orgBlock - block for which the buddy is required
 * @return buddy-block of orgBlock
 */
inline BuddyBlock *BuddyAllocator::getBuddyBlock(unsigned long blockOrder,
		BuddyBlock *orgBlock)
{
	unsigned long descOffset = ((ADDRESS)orgBlock - (ADDRESS)entryTable) /
					entrySize;

	descOffset = FLIP_BIT(descOffset, blockOrder);

	return ((BuddyBlock *)(this->entryTable + descOffset*this->entrySize));
}

static char msgNoBuddyBlockAvail[] =
		"BuddyAllocator::getBuddyList - No block avail";
static char msgListRecordInternalErr[] =
		"BuddyAllocator - Impl error FIXME (getBuddyList)";

/**
 * Finds the superblock-chain having the best-fit for the requested allocation
 * order. It will try to return the smallest superblocks as feasible - which
 * in turn reduces the induced fragmentation.
 *
 * The superblocks returned need not integrally contain a block of the given
 * order - slicing may be required (cutting the smallest block in the
 * superblock) to get the request size.`
 *
 * @param optimalOrder - the order of the request memory block size
 */
LinkedList *BuddyAllocator::getBuddyList(unsigned long optimalOrder)
{
	// Find the list containing superblocks having a
	// upper order >= optimalOrder
	unsigned short mainVector = listInfo[LV_MAIN];
	while(optimalOrder <= highestOrder) {
		if(mainVector >> optimalOrder & 1) {
			break;
		}

		++(optimalOrder);
	}

	// Error: No block list is free containing a block
	// of this order
	if(optimalOrder > highestOrder) {
		DbgLine(msgNoBuddyBlockAvail);
		return (NULL);
	}

	// Find the list containing the smallest superblocks
	// of minimum lowest order (optimalOrder)
	unsigned short listVector = listInfo[LV_SUB(optimalOrder)];
	unsigned long sbUpperOrder = optimalOrder;
	while(sbUpperOrder <= highestOrder) {
		if(listVector >> sbUpperOrder & 1) {
			break;
		}
		++(sbUpperOrder);
	}

	// Error: Internal data was incorrect that a block list
	// is not-empty with a superblock of min-order (optimalOrder).
	// (Should not happen) [sbUpperOrder is always <= highestOrder]
	if(sbUpperOrder > highestOrder) {
		Dbg(msgListRecordInternalErr);
		return (NULL);
	}

	return (getBuddyList(optimalOrder, sbUpperOrder));
}

/**
 * Calculates the offset of the list which stores super-block of
 * [lower-order, upper-order] sizes.
 */
LinkedList *BuddyAllocator::getBuddyList(unsigned long lowerOrder,
		unsigned long upperOrder)
{
	unsigned long listOffset = (lowerOrder
			* (this->highestOrder + 1 - (lowerOrder - 1) / 2))
			+ upperOrder - lowerOrder;

	return (this->blockLists + listOffset);
}

/**
 * Returns the list in which, if this block were added, was supposed to
 * be stored.
 *
 * @param bBlock - the block for which the list is required
 * @return - the list in which the block, if added, is stored
 */
LinkedList *BuddyAllocator::getBuddyList(BuddyBlock *bBlock)
{
	return (getBuddyList(bBlock->LowerOrder, bBlock->UpperOrder));
}

/**
 * Adds the buddy-block to the allocator, into its proper list with the
 * [lower-order, upper-order] coordinates.
 */
void BuddyAllocator::addBuddyBlock(BuddyBlock *bBlock)
{
	AddElement((LinkedListNode *) bBlock, getBuddyList(bBlock));
	BDLINK(bBlock);

	listInfo[LV_MAIN] |= (1 << bBlock->LowerOrder);
	listInfo[LV_SUB(bBlock->LowerOrder)] |= (1 << bBlock->UpperOrder);
}

/**
 * This function will remove the buddy-block from the allocator's list,
 * and BDUNLINK will be done on the block. Before removal, the block-struct
 * must have the LINK flag set.
 */
void BuddyAllocator::removeBuddyBlock(BuddyBlock *bBlock)
{
	removeBuddyBlock(bBlock, getBuddyList(bBlock));
}

void BuddyAllocator::removeBuddyBlock(BuddyBlock *bBlock,
		LinkedList *containerList)
{
	RemoveElement((LinkedListNode *) bBlock, containerList);

	BDUNLINK(bBlock);
	if(containerList->count == 0) {
		listInfo[LV_SUB(bBlock->LowerOrder)] &=
					~(1 << bBlock->UpperOrder);

		if(listInfo[LV_SUB(bBlock->LowerOrder)] == 0) {
			listInfo[LV_MAIN] &=
					~(1 << bBlock->LowerOrder);
		}
	}
}

/**
 * Splits the given super-block such that a block of newOrder is
 * extracted from it. This may result into two or three fragments of
 * the super-block which are returned through double-pointers.
 *
 * NOTE:
 * 1. lower-superblock and/or upper-superblock may not exist
 * 2. lower-superblock, required-block & upper-superblock may not
 *  be in order physically
 *
 * This function assumes that the superblock has a order >= order of the
 * required block, and will crash on not having so.
 *
 * @param[in] newOrder - order of the block that is to be carved
 * 		out of the super-block passed
 * @param[in] orgSuperBlock - the super-block that is to be split
 * 		into two or three fragments
 * @param[out] lowerSuperBlock - double pointer to return the
 * 		resulting lower super-block
 * @param[out] upperSuperBlock - double pointer to return the
 * 		resulting upper super-block
 *
 * @return - pointer to the block that was carved out due to calling
 * 		this function. When three fragments form out of the
 * 		original super-block, then this always is in the middle.
 */
BuddyBlock *BuddyAllocator::splitSuperBlock(unsigned long newOrder,
		BuddyBlock *orgSuperBlock, BuddyBlock **lowerSuperBlock,
		BuddyBlock **upperSuperBlock)
{
	if(newOrder > orgSuperBlock->LowerOrder) {
		// The block of the required order is contained in the
		// superblock given and thus the structure is -
		// [lower super-block][block-required][upper super-block]

		BuddyBlock *newBlock = BlockAtOffsetOf(orgSuperBlock,
				SIZEOF_DIFF(newOrder,
						orgSuperBlock->LowerOrder));

		BuddyBlock *lowerChunk = orgSuperBlock;

		if(newOrder != orgSuperBlock->UpperOrder) {
			BuddyBlock *upperChunk = BlockAtOffsetOf(newBlock,
					SIZEOF_ORDER(newOrder));

			upperChunk->UpperOrder = orgSuperBlock->UpperOrder;
			upperChunk->LowerOrder = newOrder + 1;
			*upperSuperBlock = upperChunk;
		} else {
			*upperSuperBlock = NULL;
		}

		newBlock->UpperOrder = newBlock->LowerOrder = newOrder;
		lowerChunk->UpperOrder = newOrder - 1;

		*lowerSuperBlock = lowerChunk;
		return (newBlock);
	} else {
		// The block of the required order will be cut of the
		// smallest block in the super-blocks set & result -
		//      [block-required][lower chunk][upper chunk]

		bool orgIsBlock = (orgSuperBlock->LowerOrder ==
					orgSuperBlock->UpperOrder);
		if(orgSuperBlock->UpperOrder != newOrder) {
			BuddyBlock *upperChunk;

			if(orgIsBlock) {
				// newOrder < orgSuperBlock->LowerOrder, we know
				upperChunk = BlockAtOffsetOf(orgSuperBlock,
						SIZEOF_ORDER(newOrder));

				upperChunk->UpperOrder =
						orgSuperBlock->UpperOrder - 1;
				upperChunk->LowerOrder = newOrder;
			} else {
				// newOrder == orgSuperBlock->LowerOrder, we know
				upperChunk = BlockAtOffsetOf(orgSuperBlock,
						SIZEOF_ORDER(orgSuperBlock->
								LowerOrder));

				upperChunk->UpperOrder =
						orgSuperBlock->UpperOrder;

				upperChunk->LowerOrder =
						orgSuperBlock->LowerOrder + 1;
			}

			*upperSuperBlock = upperChunk;
		} else {
			*upperSuperBlock = NULL;
		}

		if(!orgIsBlock && newOrder != orgSuperBlock->LowerOrder) {
			BuddyBlock *lowerChunk =  BlockAtOffsetOf(orgSuperBlock,
					SIZEOF_ORDER(newOrder));

			lowerChunk->UpperOrder = orgSuperBlock->LowerOrder - 1;
			lowerChunk->LowerOrder = newOrder;
			*lowerSuperBlock = lowerChunk;
		} else {
			*lowerSuperBlock = NULL;
		}

		orgSuperBlock->LowerOrder = orgSuperBlock->UpperOrder = newOrder;
		return (orgSuperBlock);
	}
}

/**
 * Merges deallocated memory block with its subsequent buddies as
 * required - assuming that the originally deallocated block has not
 * been added to the buddy-lists.
 *
 * @param orgSuperBlock - the originally deallocated block
 * @param maxMergeOrder - an upper bound on the resulting block order,
 * 					after the merge operation
 */
BuddyBlock *BuddyAllocator::mergeSuperBlock(BuddyBlock *orgSuperBlock,
					unsigned long maxMergeOrder)
{
	// To merge buddy-blocks at each succesive order, this function uses
	// recursion. After merging two blocks, mergeSuperBlock() is again
	// called passing the next order.
	if(orgSuperBlock->UpperOrder < maxMergeOrder)
	{
		BuddyBlock *orgBuddyBlock;
		BuddyBlock *orgLeftBlock;
		BuddyBlock *orgRightBlock;

		orgBuddyBlock = getBuddyBlock(orgSuperBlock->Order, orgSuperBlock);

		if(TBDFREE(orgBuddyBlock))
		{
			// The left block always becomes the new parent-block.

			orgLeftBlock = ((unsigned long) orgBuddyBlock >
						(unsigned long) orgSuperBlock)
					? orgSuperBlock : orgBuddyBlock;

			orgRightBlock = (orgLeftBlock == orgSuperBlock)
					? orgBuddyBlock : orgSuperBlock;

			// Merging only occurs when both adjacent super-block of the
			// same lower-order.
			if(orgLeftBlock->LowerOrder ==
					orgRightBlock->LowerOrder)
			{
				if(TBDLINKED(orgSuperBlock))
					removeBuddyBlock(orgSuperBlock);

				removeBuddyBlock(orgBuddyBlock);

				++(orgRightBlock->UpperOrder);

				orgLeftBlock->UpperOrder =
						orgRightBlock->UpperOrder;

				orgLeftBlock->LowerOrder =
						orgRightBlock->UpperOrder;

				return (mergeSuperBlock(orgLeftBlock,
						maxMergeOrder));
			}
			else
			{
				return (orgSuperBlock);
			}
		}
		else
		{
			return (orgSuperBlock);
		}
	}

	return (orgSuperBlock);
}

/**
 * Allocates a fresh memory block of the given order, if available
 * in the buddy lists.
 *
 * @param blockOrder - order of the block to be allocated
 */
BuddyBlock *BuddyAllocator::allocateBlock(unsigned long blockOrder)
{
	if(freeBuddies < SIZEOF_ORDER(blockOrder))
		return ((BuddyBlock *)(BD_MEMORY_LOW));

	LinkedList *optimalList = getBuddyList(blockOrder);

	if(optimalList == NULL) {
		return ((BuddyBlock *)(BD_FRAGMENTATION));
	} else {
		BuddyBlock *paSuperBlock = (BuddyBlock*) optimalList->head;
		removeBuddyBlock(paSuperBlock, optimalList);


		BuddyBlock *upperPortion, *lowerPortion;
		BuddyBlock *allocBlock =
				splitSuperBlock(blockOrder, paSuperBlock,
						&lowerPortion, &upperPortion);

		if(upperPortion != NULL) addBuddyBlock(upperPortion);
		if(lowerPortion != NULL) addBuddyBlock(lowerPortion);

		BLOCK_UNFREE(allocBlock);// Make sure FREE flag is clear
		BDUNLINK(allocBlock);

		freeBuddies -= SIZEOF_ORDER(allocBlock->Order);
		allocatedBuddies += SIZEOF_ORDER(allocBlock->Order);

		return (allocBlock);
	}
}

/**
 * Deallocates the buddy-block given - assuming it was the one
 * returned while allocating the memory being freed. It fully takes
 * the memory, leaving none to the caller.
 *
 * @param blockGiven - descriptor of the block which is to be freed
 */
unsigned long BuddyAllocator::freeBlock(BuddyBlock *blockGiven)
{
	if(TBDLINKED(blockGiven))
	{
		return (BD_USED);
	}
	else if(blockGiven->Order != blockGiven->UpperOrder)
	{
		return (BD_ORDER_CORRUPT);
	}
	else
	{
		freeBuddies += SIZEOF_ORDER(blockGiven->Order);

		BuddyBlock *mergedBlock = mergeSuperBlock(blockGiven, highestOrder);
		BLOCK_FREE(mergedBlock);// Mark FREE flag
		addBuddyBlock(mergedBlock);

		return (BD_SUCCESS);
	}
}

/**
 * This function is used for expansion of data structures in memory
 * without the need for allocating a block of order(n + 1) before
 * transferring the data from the original order(n) block to the
 * larger block. It will check if the block's parent is free. If yes,
 * then the parent is returned else a new block is allocated.
 *
 * Note: The original block is never freed, because it will returned
 * in the parent block. *status will contain if the block returned is
 * the parent block or not.
 *
 * Note: Although the parent block may be free & returned, the client
 * must check whether the memory address of the order(n+1) block
 * matches the order(n) block, because it may be the right child of
 * the order(n+1) block. But this happens less (because superblocks
 * are located at right-side).
 *
 * @param[in] dataBlock - Block containing the data requiring expansion
 * @param[out] status - *status contains TRUE/FALSE whether parent block
 * 			was free/not.
 */
BuddyBlock *BuddyAllocator::exchangeBlock(BuddyBlock *orgBlock, unsigned long *statusRegister)
{
	if(TBDFREE(orgBlock))
	{
		return ((BuddyBlock *)(BD_ERR_FREE));
	}
	else if(orgBlock->Order != orgBlock->UpperOrder)
	{
		return ((BuddyBlock *)(BD_ORDER_CORRUPT));
	}

	BuddyBlock *orgBuddy = getBuddyBlock(orgBlock->Order, orgBlock);

	if(TBDFREE(orgBuddy) && orgBuddy->Order == orgBlock->Order)
	{
		// We know that the lower superblock won't form so we allow the
		// status register to be "garbaged".

		removeBuddyBlock(orgBuddy);
		BuddyBlock *upperPortion;
		BuddyBlock *buddySplitBlock = splitSuperBlock(orgBlock->Order,
				orgBuddy, (BuddyBlock **) statusRegister,
				&upperPortion);

		if(upperPortion != NULL)
		{
			BLOCK_FREE(upperPortion);
			addBuddyBlock(upperPortion);
		}

		BuddyBlock *mergedBlock = ((unsigned long) buddySplitBlock
				> (unsigned long) orgBlock)
				? orgBlock : buddySplitBlock;

		BLOCK_UNFREE(mergedBlock);
		++(mergedBlock->UpperOrder);
		++(mergedBlock->LowerOrder);

		*statusRegister = BD_EXTERNAL;
		return (mergedBlock);
	}
	else
	{
		*statusRegister = BD_INTERNAL;
		return (NULL);
	}
}
