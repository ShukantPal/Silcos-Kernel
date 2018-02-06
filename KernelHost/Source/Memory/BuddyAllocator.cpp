/**
 * File: BuddyAllocator.cpp
 *
 * Summary:
 * The buddy-allocator is the back-end for the commonly-used zone allocator, and
 * its core is implemented in this file. It works in a customizable fashion as -
 *
 * Buddy-allocation uses a table of block-descriptors. These descriptors record the
 * status of a memory block. Although the size of these entries are variable, they
 * always start with the (BuddyBlock) block-header. The core buddy-allocator
 * only uses the header.
 *
 * NOTE:
 * Extensions to buddy-allocator are not supported yet (cv-2.03,++)
 *
 * Copyright (C) 2017 - Shukant Pal
 */
#include <Memory/BuddyManager.h>
#include <Memory/Pager.h>
#include <KERNEL.h>

using namespace Memory;
using namespace Memory::Internal;

BuddyAllocator::BuddyAllocator()
{
	this->entrySize = 0;
	this->entryTable = NULL;
	this->highestOrder = 0;
	this->listInfo = NULL;
	this->blockLists = NULL;
	this->freeBuddies =
			this->allocatedBuddies = 0;
}

BuddyAllocator::BuddyAllocator(unsigned long entrySize, unsigned char *entryTable, unsigned long highestOrder,
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

BuddyBlock *BuddyAllocator::getBuddyBlock(unsigned long blockOrder, BuddyBlock *orgBlock)
{
	unsigned long descOffset = ((ADDRESS) orgBlock - (ADDRESS) this->entryTable) / this->entrySize;
	descOffset = FLIP_BIT(descOffset, blockOrder);
	return (BuddyBlock *) (this->entryTable + descOffset * this->entrySize);
}

static char msgNoBuddyBlockAvail[] = "BuddyAllocator::getBuddyList - No block avail";
static char msgListRecordInternalErr[] = "BuddyAllocator - Impl error FIXME (getBuddyList)";

/**
 * Function: BuddyAllocator::getBuddyList
 *
 * Summary:
 * This function is used for getting the list containing the superblocks that
 * best fit the need for getting a buddy-block of optimalOrder. It will try to
 * get the lists containing a block of this order, then of higher orders. This
 * is required to reduce fragmentation.
 *
 * NOTE: This list returned need not contain superblocks containing this order
 * block.`
 *
 * Args:
 * unsigned long optimalOrder - The order of the block which is required.
 *
 * Version: 1.1
 * Since: Circuit 2.03++
 * Author: Shukant Pal
 */
LinkedList *BuddyAllocator::getBuddyList(unsigned long optimalOrder)
{
	// Find the list containing superblocks having a
	// upper order >= optimalOrder
	unsigned short mainVector = listInfo[LV_MAIN];
	while(optimalOrder <= highestOrder)
	{
		if(mainVector >> optimalOrder & 1)
			break;
		++(optimalOrder);
	}

	// Error: No block list is free containing a block
	// of this order
	if(optimalOrder > highestOrder)
	{
		DbgLine(msgNoBuddyBlockAvail);
		return (NULL);
	}

	// Find the list containing the smallest superblocks
	// of minimum lowest order (optimalOrder)
	unsigned short listVector = listInfo[LV_SUB(optimalOrder)];
	unsigned long sbUpperOrder = optimalOrder;
	while(sbUpperOrder <= highestOrder)
	{
		if(listVector >> sbUpperOrder & 1)
			break;
		++(sbUpperOrder);
	}

	// Error: Internal data was incorrect that a block list
	// is not-empty with a superblock of min-order (optimalOrder).
	// (Should not happen) [sbUpperOrder is always <= highestOrder]
	if(sbUpperOrder > highestOrder)
	{
		Dbg(msgListRecordInternalErr);
		return (NULL);
	}

	return getBuddyList(optimalOrder, sbUpperOrder);
}

/**
 * Function: BuddyAllocator::getBuddyList
 *
 * Summary:
 * This function will return the list containing superblock of type
 * (lowerOrder, upperOrder).
 */
LinkedList *BuddyAllocator::getBuddyList(unsigned long lowerOrder, unsigned long upperOrder)
{
	unsigned long listOffset = (lowerOrder
			* (this->highestOrder + 1 - (lowerOrder - 1) / 2))
			+ upperOrder - lowerOrder;
	return (this->blockLists + listOffset);
}

/**
 * Function: BuddyAllocator::getBuddyList
 *
 * Summary:
 * This function will return the list in which this buddy block should
 * be contained, but it is not necessary that the buddy-block given is
 * in the list.
 *
 * Version: 1.1
 * Since: Circuit 2.03++
 * Author: Shukant Pal
 */
LinkedList *BuddyAllocator::getBuddyList(BuddyBlock *bBlock)
{
	return getBuddyList(bBlock->LowerOrder, bBlock->UpperOrder);
}

/**
 * Function: BuddyAllocator::addBuddyBlock
 *
 * Summary:
 * This function will add the buddy-block to the allocator's lists, and
 * before doing so a BDLINK check will be done on the struct.
 *
 * Version: 1.1.2
 * Since: Circuit 2.03++
 * Author: Shukant Pal
 */
void BuddyAllocator::addBuddyBlock(BuddyBlock *bBlock)
{
	AddElement((LinkedListNode *) bBlock, getBuddyList(bBlock));
	BDLINK(bBlock);

	listInfo[LV_MAIN] |= (1 << bBlock->LowerOrder);
	listInfo[LV_SUB(bBlock->LowerOrder)] |= (1 << bBlock->UpperOrder);
}

/**
 * Function: BuddyAllocator;:removeBuddyBlock
 *
 * Summary:
 * This function will remove the buddy-block from the allocator's list,
 * and BDUNLINK will be done on the block. Before removal, the block-struct
 * must have the LINK flag set.
 *
 * Version: 1.1.2
 * Since: Circuit 2.03++
 * Author: Shukant Pal
 */
void BuddyAllocator::removeBuddyBlock(BuddyBlock *bBlock)
{
	removeBuddyBlock(bBlock, getBuddyList(bBlock));
}

void BuddyAllocator::removeBuddyBlock(BuddyBlock *bBlock, LinkedList *containerList)
{
	RemoveElement((LinkedListNode *) bBlock, containerList);

	BDUNLINK(bBlock);
	if(containerList->count == 0)
	{
		listInfo[LV_SUB(bBlock->LowerOrder)] &= ~(1 << bBlock->UpperOrder);
		// No superblocks for given lower-order exist then
		if(listInfo[LV_SUB(bBlock->LowerOrder)] == 0)
			listInfo[LV_MAIN] &= ~(1 << bBlock->LowerOrder);
	}
}

/**
 * Function: BuddyAllocator::splitSuperBlock
 *
 * Summary:
 * This function is used for extraction of a block-of-order-given from a superblock
 * of larger lower-order. It (generally) splits the superblock into three pieces -
 * lower-superblock, block-required, & upper-superblock. Pointers to these pieces are
 * given to the client.
 *
 * NOTE:
 * 1/ lower-superblock and/or upper-superblock may not exist
 * 2/ lower-superblock, required-block & upper-superblock mayn't be in order physically
 *
 * Assumptions:
 * This function assumes that the superblock has a order >= order of the required
 * block, and will crash on not having so.
 *
 * Args:
 * unsigned long newOrder - Order of required-block
 * BuddyBlock *orgSuperBlock - Original super-block to be split
 * BuddyBlock **lowerSuperBlock - Pointer to pointer of lower superblock
 * BuddyBlock **upperSuperBlock - Pointer to pointer of upper superblock
 *
 * Returns: A pointer to the required-block
 * Version: 1.1.0
 * Since: Circuit 2.03,++
 * Author: Shukant Pal
 */
BuddyBlock *BuddyAllocator::splitSuperBlock(unsigned long newOrder, BuddyBlock *orgSuperBlock, BuddyBlock **lowerSuperBlock,
						BuddyBlock **upperSuperBlock)
{
	if(newOrder > orgSuperBlock->LowerOrder)
	{
		// The block of the required order is contained in the superblock given and
		// thus the structure is - [lower super-block][block-required][upper super-block]
		BuddyBlock *newBlock = BlockAtOffsetOf(orgSuperBlock, SIZEOF_DIFF(newOrder, orgSuperBlock->LowerOrder));
		BuddyBlock *lowerChunk = orgSuperBlock;

		if(newOrder != orgSuperBlock->UpperOrder)
		{
			// Here, we know that a upper super-block will be created
			BuddyBlock *upperChunk =
					BlockAtOffsetOf(newBlock, SIZEOF_ORDER(newOrder));
			upperChunk->UpperOrder = orgSuperBlock->UpperOrder;
			upperChunk->LowerOrder = newOrder + 1;
			*upperSuperBlock = upperChunk;
		}
		else
		{
			// No upper super-block, cause block required is the largest
			*upperSuperBlock = NULL;
		}

		newBlock->UpperOrder = newBlock->LowerOrder = newOrder;
		lowerChunk->UpperOrder = newOrder - 1;

		*lowerSuperBlock = lowerChunk;
		return (newBlock);
	} else {
		// The block of the required order will be cut of the smallest block in the
		// super-blocks set & result - [block-required][lower chunk][upper chunk]
		bool orgIsBlock = (orgSuperBlock->LowerOrder == orgSuperBlock->UpperOrder);
		if(orgSuperBlock->UpperOrder != newOrder)
		{
			BuddyBlock *upperChunk;
			if(orgIsBlock)
			{
				// newOrder < orgSuperBlock->LowerOrder (must be satisfied)
				upperChunk = BlockAtOffsetOf(orgSuperBlock, SIZEOF_ORDER(newOrder));
				upperChunk->UpperOrder = orgSuperBlock->UpperOrder - 1;
				upperChunk->LowerOrder = newOrder;
			}
			else
			{
				// newOrder = orgSuperBlock->LowerOrder (must be correct)
				upperChunk = BlockAtOffsetOf(orgSuperBlock, SIZEOF_ORDER(orgSuperBlock->LowerOrder));
				upperChunk->UpperOrder = orgSuperBlock->UpperOrder;
				upperChunk->LowerOrder = orgSuperBlock->LowerOrder + 1;
			}
			*upperSuperBlock = upperChunk;
		}
		else
		{
			*upperSuperBlock = NULL;
		}

		if(!orgIsBlock && newOrder != orgSuperBlock->LowerOrder)
		{
			BuddyBlock *lowerChunk =
					BlockAtOffsetOf(orgSuperBlock, SIZEOF_ORDER(newOrder));
			lowerChunk->UpperOrder = orgSuperBlock->LowerOrder - 1;
			lowerChunk->LowerOrder = newOrder;
			*lowerSuperBlock = lowerChunk;
		}
		else
		{
			*lowerSuperBlock = NULL;
		}

		orgSuperBlock->LowerOrder = orgSuperBlock->UpperOrder = newOrder;
		return (orgSuperBlock);
	}
}

/**
 * Function: BuddyAllocator::mergeSuperBlock
 *
 * Summary:
 * While a external block is freed, it is combined with other blocks (not
 * superblocks) to form larger blocks. This function fulfills this functionality
 * and merges blocks (even superblocks are allowed, but not tested yet).
 *
 * Assumptions:
 * The original superblock MUST be off the lists.
 *
 * Effects:
 * If the block is merged, its buddies are subsequently removed from the buddy
 * lists.
 *
 * Version: 1.1
 * Since: Circuit 2.03,++
 * Author: Shukant Pal
 */
BuddyBlock *BuddyAllocator::mergeSuperBlock(BuddyBlock *orgSuperBlock, unsigned long maxMergeOrder)
{
	// Recursion is being used here
	if(orgSuperBlock->UpperOrder < maxMergeOrder)
	{
		BuddyBlock *orgBuddyBlock;
		BuddyBlock *orgLeftBlock;
		BuddyBlock *orgRightBlock;

		orgBuddyBlock = getBuddyBlock(orgSuperBlock->Order, orgSuperBlock);

		if(TBDFREE(orgBuddyBlock))
		{
			// Remember, the left block will become the parent block.
			orgLeftBlock = ((unsigned long) orgBuddyBlock > (unsigned long) orgSuperBlock)
					? orgSuperBlock : orgBuddyBlock;
			orgRightBlock = (orgLeftBlock == orgSuperBlock)
					? orgBuddyBlock : orgSuperBlock;

			// Merging only occurs when both adjacent super-block of the
			// same lower-order.
			if(orgLeftBlock->LowerOrder == orgRightBlock->LowerOrder)
			{
				if(TBDLINKED(orgSuperBlock))
					removeBuddyBlock(orgSuperBlock);

				removeBuddyBlock(orgBuddyBlock);// Buddy-block must be freed

				++(orgRightBlock->UpperOrder);
				orgLeftBlock->UpperOrder =
						orgLeftBlock->LowerOrder =
								orgRightBlock->UpperOrder;

				return mergeSuperBlock(orgLeftBlock, maxMergeOrder);
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
 * Function: BuddyAllocator::allocateBlock
 *
 * Summary:
 * This function will allocate a buddy-block from the allocators buddy-lists and
 * return its descriptor. It tries to use the most optimal superblock-list to
 * reduce fragmentation in the memory-subsystem.
 *
 * Args:
 * unsigned long blockOrder - Order of the block required (power-of-2)
 *
 * Version: 1.1.1
 * Since: Circuit 2.03++
 * Author: Shukant Pal
 */
BuddyBlock *BuddyAllocator::allocateBlock(unsigned long blockOrder)
{
	if(freeBuddies < SIZEOF_ORDER(blockOrder))
		return (BuddyBlock *) (BD_MEMORY_LOW);// Package err to front-end

	LinkedList *optimalList = getBuddyList(blockOrder);

	if(optimalList == NULL)
	{
		return (BuddyBlock *) (BD_FRAGMENTATION);
	}
	else
	{
		BuddyBlock *paSuperBlock = (BuddyBlock*) optimalList->head;
		removeBuddyBlock(paSuperBlock, optimalList);


		BuddyBlock *upperPortion, *lowerPortion;
		BuddyBlock *allocBlock =
				splitSuperBlock(blockOrder, paSuperBlock, &lowerPortion, &upperPortion);

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
 * Function: BuddyAllocator::freeBlock
 *
 * Summary:
 * This function will free a previously allocated block by first merging it with
 * other super-blocks to form larger blocks (not super-blocks) and then will add
 * the merged block to the buddy-lists.
 *
 * Args:
 * BuddyBlock *blockGiven - Pointer to descriptor of the given block
 *
 * Version: 1.1
 * Since: Circuit 2.03++
 * Author: Shukant Pal
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
 * Function: BuddyAllocator::exchangeBlock
 *
 * Summary:
 * This function is used for expansion of data structures in memory without
 * the need for allocating a block of order(n + 1) before transferring the
 * data from the original order(n) block to the larger block. It will check
 * if the block's parent is free. If yes, then the parent is returned else
 * a new block is allocated.
 *
 * Note: The original block is never freed, because it will returned in the
 * parent block. *status will contain if the block returned is the parent
 * block or not.
 *
 * Note: Although the parent block may be free & returned, the client must
 * check whether the memory address of the order(n+1) block matches the order(n)
 * block, because it may be the right child of the order(n+1) block. But this
 * happens less (because superblocks are located at right-side).
 *
 * Args:
 * BuddyInfo *dataBlock - Block containing the data requiring expansion
 * unsigned long *status - *status contains TRUE/FALSE whether parent block was free/not.
 *
 * Version: 1.1
 * Since: Circuit 2.03++
 * Author: Shukant Pal
 */
BuddyBlock *BuddyAllocator::exchangeBlock(BuddyBlock *orgBlock, unsigned long *statusRegister)
{
	if(TBDFREE(orgBlock))
		return (BuddyBlock *) (BD_ERR_FREE);
	else if(orgBlock->Order != orgBlock->UpperOrder)
		return (BuddyBlock *) (BD_ORDER_CORRUPT);
	else
	{
		BuddyBlock *orgBuddy = getBuddyBlock(orgBlock->Order, orgBlock);

		if(TBDFREE(orgBuddy) && orgBuddy->Order == orgBlock->Order)
		{
			// Here, the lower super-block is not added, cause it doesn't exist. But
			// we don't pass a NULL pointer to splitSuperBlock() and instead pass the
			// statusRegister* allowing it to have a garbage value.
			removeBuddyBlock(orgBuddy);
			BuddyBlock *upperPortion;
			BuddyBlock *buddySplitBlock = splitSuperBlock(orgBlock->Order, orgBuddy,
									(BuddyBlock **) statusRegister, &upperPortion);

			if(upperPortion != NULL)
			{
				BLOCK_FREE(upperPortion);
				addBuddyBlock(upperPortion);
			}

			BuddyBlock *mergedBlock = ((unsigned long) buddySplitBlock > (unsigned long) orgBlock) ?
							orgBlock : buddySplitBlock;
			BLOCK_UNFREE(mergedBlock);
			++(mergedBlock->UpperOrder);
			++(mergedBlock->LowerOrder);

			// Expansion has occurred externally, in the allocator itself
			*statusRegister = BD_EXTERNAL;
			return (mergedBlock);
		}
		else
		{
			// Expansion will occur internally, in the client by copying
			*statusRegister = BD_INTERNAL;
			return (NULL);
		}
	}
}