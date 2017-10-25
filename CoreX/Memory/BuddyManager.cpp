/* Copyright (C) 2017 - Shukant Pal */

#include <Memory/BuddyManager.h>
#include <Memory/Pager.h>
#include <KERNEL.h>

ULONG bdSizeOf[16] = {
	1,
	2,
	4,
	8,
	16,
	32,
	64,
	128,
	256,
	512,
	1024,
	2048,
	4096,
	8192,
	16384,
	32768
};  /* Power-of-two correspondance */

/**
 * GetBuddyOf() - 
 *
 * Summary:
 * This function is used to get the buddy of a block. It calculates by mere addition/subtraction
 * of pointers. It is a manual function, and uses a given order instead of reading the block
 * info.
 *
 * Args:
 * blockOrder - Order at which the buddies are
 * bdInfo - Block whose buddy is needed
 * bdSys - The buddy system being used 
 *
 * Changes:
 * No state changes
 *
 * @Version 1
 * @Since Circuit 2.03
 */
static /* Calculate the buddy descriptor, assuming the buddies are stored in a continous array. */
BDINFO *GetBuddyOf(ULONG blockOrder, BDINFO *bdInfo, BDSYS *bdSys){
	ULONG iSize = bdSys->DescriptorSize;
	UBYTE *descriptorTable = bdSys->DescriptorTable;
	ULONG blockOffset = ((ADDRESS) bdInfo - (ADDRESS) descriptorTable) / iSize;
	blockOffset = FLIP_BIT(blockOffset, blockOrder);
	return (BDINFO *) (descriptorTable + blockOffset * iSize);
}

/**
 * BdGetListByOrder() -
 *
 * Summary:
 * This function calculates the location of the required list by lower and upper
 * orders. It uses the formula - 
 *
 * List Offset = lOrder x (mOrder + 1) - (Sum of first (lOrder-1) natural no.s) + uOrder  - lOrder
 *
 * The list at this offset is returned, and is used for adding and removing blocks of
 * the corresponding sizes.
 *
 * Args:
 * lOrder - LowerOrder of the list's blocks
 * uOrder - UpperOrder of the list's blocks
 * bdSys - The buddy system being used
 *
 * Changes:
 * This function does not change any state data of the buddy system.
 *
 * @Version 1
 * @Since Circuit 2.03
 */
LINKED_LIST *BdGetListByOrder(ULONG lOrder, ULONG uOrder, BDSYS *bdSys){
	ULONG mOrder = bdSys->HighestOrder;
	ULONG liOffset = lOrder * (mOrder + 1 - (lOrder - 1) / 2);
	liOffset += uOrder - lOrder;
	return (bdSys->BlockLists + liOffset);
}

/**
 * BdAddBlock() - 
 *
 * Summary:
 * This function adds the block into the corresponding list. The list is found
 * naturally, by looking into the block's info. It also sets the flag BD_LINKED
 * and updates the ListInfo vector.
 *
 * Args:
 * bInfo - Block information of the one to be added
 * bdSys - The buddy system to be used
 *
 * Changes:
 * 1- The function changes the state of the required list.
 * 2- The corresponding ListInfo bits are set.
 *
 * @Version 1
 * @Since Circuit 2.03
 */
static
VOID BdAddBlock(BDINFO *bInfo, BDSYS *bdSys){
	ULONG uOrder = bInfo->UpperOrder;
	ULONG lOrder = bInfo->LowerOrder;
	LINKED_LIST *blockList = BdGetListByOrder(lOrder, uOrder, bdSys);
	AddElement((LIST_ELEMENT*) bInfo, blockList);
	BDLINK(bInfo);

	bdSys->ListInfo[0] |= (1 << lOrder);
	bdSys->ListInfo[1 + lOrder] |= (1 << uOrder);
}

/**
 * BdRemoveBlock() - 
 *
 * Summary:
 * This function removes the block from the corresponding list ASSUMING that
 * the block is NOT LINKED. It also updates the ListInfo bits and clears the
 * flag BD_LINKED.
 *
 * Args:
 * bInfo - Block information of the one to be removed
 * bdSys - The buddy system to be used
 *
 * Changes:
 * 1- This function changes the state of the required list.
 * 2- The corresponding ListInfo bits are cleared.
 *
 * @Version 1
 * @Since Circuit 2.03
 */
static
VOID BdRemoveBlock(BDINFO *bInfo, BDSYS *bdSys){
	ULONG uOrder = bInfo->UpperOrder;
	ULONG lOrder = bInfo->LowerOrder;
	LINKED_LIST *blockList = BdGetListByOrder(lOrder, uOrder, bdSys);
	RemoveElement((LIST_ELEMENT*) bInfo, blockList);
	BDUNLINK(bInfo);

	if(blockList->Count == 0) {
		bdSys->ListInfo[1 + lOrder] &= ~(1 << uOrder);
		if(bdSys->ListInfo[1 + lOrder] == 0) {
			bdSys->ListInfo[0] &= ~(1 << lOrder);
		}
	}
}

/**
 * BdGetList() -
 *
 * Summary:
 * This function is used for getting a block list having a block of the required order. It searches the
 * bits of ListInfo to get to the required list.
 *
 * Args:
 * bOrder - Order of the block needed
 * bdSys - The buddy system used for getting the block
 *
 * Changes:
 * This function makes no changes to the state of the system.
 *
 * @Version 1
 * @Since Circuit 2.03
 */
static
LINKED_LIST *BdGetList(ULONG bOrder, BDSYS *bdSys){
	ULONG mOrder = bdSys->HighestOrder + 1;
	USHORT loVector = bdSys->ListInfo[0];

	while(bOrder < mOrder) {
		if(loVector >> bOrder & 1)
			break;
		++bOrder;
	}
	if(bOrder == mOrder) { DbgLine("ERRO:LOWE"); while(TRUE); return (NULL); }

	USHORT uoVector = bdSys->ListInfo[1 + bOrder];
	ULONG buOrder = bOrder;

	while(buOrder < mOrder) {
		if(uoVector >> buOrder & 1)
			break;
		++buOrder;
	}
	if(buOrder == mOrder) { DbgLine("BDERRO: TRUE"); while(TRUE); }
	
	return (BdGetListByOrder(bOrder, buOrder, bdSys));
}

/**
 * BdSplitBlock() -
 *
 * Summary:
 * This function splits the given block into upto three pieces - required block, lower super block
 * and the upper super block. The required block's order is given by the caller, whereas the lower
 * super block and upper super block are returned by passing double pointers. The function
 * ASSUMES that required order is less than that of parent superblock. The implementation is
 * structurally simple, and is just changing a few values of the pointers.
 *
 * Args:
 * newOrder - Required order of the returned block
 * bInfo - Parent superblock
 * lowerSuperBlock - Double pointer to store lower superblock
 * upperSuperBlock - Double pointer to store upper superblock
 * bdSys - The buddy system being used
 *
 * Changes:
 * This function does not change the state of the system, but does change the state of the concerned
 * block infos.
 *
 * @Version 1
 * @Since Circuit 2.03
 */
static
BDINFO *BdSplitBlock(ULONG newOrder, BDINFO *bInfo, BDINFO **lowerSuperBlock, BDINFO **upperSuperBlock, BDSYS *bdSys){
	ULONG iSize = bdSys->DescriptorSize;
	ULONG uOrder = bInfo->UpperOrder;
	ULONG lOrder = bInfo->LowerOrder;
	BOOL iBlock = (uOrder == lOrder);

	BDINFO *lowerSB;
	BDINFO *upperSB;
	if(newOrder > lOrder) {
		BDINFO *nInfo = (BDINFO*) ((UBYTE*) bInfo + (bdSizeOf[newOrder] - bdSizeOf[lOrder]) * iSize);
		lowerSB = bInfo;
		upperSB = nInfo + iSize * bdSizeOf[newOrder];

		if(newOrder != uOrder) {
			upperSB->UpperOrder = uOrder;
			upperSB->LowerOrder = newOrder + 1;
			*upperSuperBlock = upperSB;
		} else
		 	*upperSuperBlock = NULL;

		nInfo->UpperOrder = newOrder;
		nInfo->LowerOrder = newOrder;
		lowerSB->UpperOrder = newOrder - 1;

		*lowerSuperBlock = lowerSB;
		return (nInfo);
	} else {
		if(uOrder != newOrder) {
			if(iBlock) {
				upperSB = (BDINFO *) ((UBYTE *) bInfo + iSize * (1 << newOrder));
				upperSB->UpperOrder = uOrder - 1;
				upperSB->LowerOrder = newOrder;
			} else {
				upperSB = (BDINFO*) ((UBYTE*) bInfo + iSize * bdSizeOf[bInfo->LowerOrder]);
				upperSB->UpperOrder = uOrder;
				upperSB->LowerOrder = lOrder + 1;
			}
			*upperSuperBlock = upperSB;
		} else
 			 *upperSuperBlock = NULL;

		if(!iBlock && newOrder != lOrder) {
			lowerSB= (BDINFO*) ((UBYTE*) bInfo + iSize * bdSizeOf[newOrder]);
			lowerSB->UpperOrder = lOrder - 1;
			lowerSB->LowerOrder = newOrder;
			*lowerSuperBlock = lowerSB;
		} else
			*lowerSuperBlock = NULL;

		bInfo->UpperOrder = newOrder;
		bInfo->LowerOrder = newOrder;

		return (bInfo);
	}
}

/**
 * BdMergeBlock() - 
 *
 * Summary:
 * As you can see, this function is singular. It required one block to merge, because the other is found
 * by getting the buddy block. The adjacent superblock/block is merged only iff they form a block. No
 * superblock is formed after merging.
 *
 * Args:
 * bInfo - The block to merge
 * maxMergeOrder - Maximum order to which the block should be merged.
 * bdSys - The system being used
 *
 * Changes:
 * Blocks are continuously removed from lists, if they are merged.
 *
 * @Version 1
 * @Since Circuit 2.03
 */
static
BDINFO *BdMergeBlock(BDINFO *bInfo, ULONG maxMergeOrder, BDSYS *bdSys){
	if(bInfo->UpperOrder < maxMergeOrder) {
		ULONG lOrder = bInfo->LowerOrder;
		BDINFO *bdInfo = GetBuddyOf(lOrder, bInfo, bdSys);

		if(TBDFREE(bdInfo)) {
			BDINFO *leftBlock = ((ADDRESS) bdInfo > (ADDRESS) bInfo) ? bInfo : bdInfo;
			ULONG llOrder = leftBlock->LowerOrder;

			BDINFO *rightBlock = (leftBlock == bInfo) ? bdInfo : bInfo;
			ULONG rlOrder = rightBlock->LowerOrder;
			ULONG ruOrder = rightBlock->UpperOrder;

			if(llOrder == rlOrder) {
				if(TBDLINKED(bInfo)) BdRemoveBlock(bInfo, bdSys);
				BdRemoveBlock(bdInfo, bdSys); /* bdInfo must be in the lists */

				++ruOrder;
				leftBlock->UpperOrder = ruOrder;
				leftBlock->LowerOrder = ruOrder;
				return (BdMergeBlock(leftBlock, maxMergeOrder, bdSys));
			} else
				return (bInfo);
		} else
			return (bInfo);
	}

	return (bInfo);
}

BDINFO *BdAllocateBlock(ULONG blockOrder, BDSYS *bdSys){
	if(bdSys->OpFree < bdSizeOf[blockOrder])
		return (BDINFO*) (BD_MEMORY_LOW);

	LINKED_LIST *blockList = BdGetList(blockOrder, bdSys);
	if(blockList == NULL)
		return (BDINFO*) (BD_FRAGMENTATION);
	else {
		BDINFO *bInfo = (BDINFO*) blockList->Head;
		BdRemoveBlock(bInfo, bdSys);
		if(bInfo->UpperOrder >= blockOrder) {
			BDINFO *uSB = NULL;
			BDINFO *lSB = NULL;
			BDINFO *splitBlock = BdSplitBlock(blockOrder, bInfo, &lSB, &uSB, bdSys);

			if(uSB != NULL)
				BdAddBlock(uSB, bdSys);
			if(lSB != NULL)
				BdAddBlock(lSB, bdSys);
			bInfo = splitBlock;
		}

		BLOCK_UNFREE(bInfo);
		BDUNLINK(bInfo);
		bdSys->OpFree -= bdSizeOf[bInfo->UpperOrder];
		bdSys->OpAllocated += bdSizeOf[bInfo->UpperOrder];
		return (bInfo);
	}
}

ULONG BdFreeBlock(BDINFO *bInfo, BDSYS *bdSys){
	if(TBDLINKED(bInfo)) return (BD_USED);
	else if(bInfo->LowerOrder != bInfo->UpperOrder) return (BD_ORDER_CORRUPT);
	else {
		bdSys->OpFree += (1 << bInfo->UpperOrder);
		BDINFO *mInfo = BdMergeBlock(bInfo, bdSys->HighestOrder, bdSys);
		BLOCK_FREE(mInfo);
		BdAddBlock(mInfo, bdSys);
		return (BD_SUCCESS);
	}
}

BDINFO *BdExchangeBlock(BDINFO *bInfo, BDSYS *bdSys, ULONG *status){
	if(TBDFREE(bInfo)) { return ((BDINFO*) BD_ERR_FREE); }
	else {
		BDINFO *bdInfo = GetBuddyOf(bInfo->UpperOrder, bInfo, bdSys);
		if(TBDFREE(bdInfo) && bdInfo->LowerOrder == bInfo->UpperOrder) {
			/* Here, we are splitting the block, and garbaging the lSB, because we already know that
			 * the lower orders are equal.
			 */
			BdRemoveBlock(bdInfo, bdSys);
			BDINFO *uSB;
			BDINFO *splitBlock = BdSplitBlock(bInfo->UpperOrder, bdInfo, (BDINFO**) status, &uSB, bdSys);

			if(uSB != NULL) {
				BLOCK_FREE(uSB);
				BdAddBlock(uSB, bdSys);
			}

			BDINFO *mInfo = ((ULONG) splitBlock > (ULONG) bInfo) ? bInfo : splitBlock;
			BLOCK_UNFREE(mInfo);
			++(mInfo->UpperOrder); ++(mInfo->LowerOrder);
			*status = BD_EXTERNAL;
			return (mInfo);
		} else {
			*status = BD_INTERNAL;
			return (NULL);
		}
	}
}
