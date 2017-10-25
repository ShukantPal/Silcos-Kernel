/** 
 * Copyright (C) 2017 - Shukant Pal
 */

#define NAMESPACE_KFRAME_MANAGER

#include <Memory/KFrameManager.h>
#include <Memory/ZoneManager.h>
#include <KERNEL.h>

#define ZONE_ALLOCABLE 10
#define ZONE_RESERVE_ONLY 11
#define ZONE_BARRIER_ONLY 12
#define ZONE_OVERLOAD 101
#define ZONE_LOADED 102
typedef ULONG ZNALLOC; /* Internal type - Used for testing if zone is allocable from */

/**
 * ZnTest() - 
 *
 * Summary:
 * This function (internal), tests the state of the zone. It returns the status after testing
 * for various conditions, with the memory order requested. It has no relation with flags
 * and thus, further analysis with allocation-controlling flags is required.
 *
 * Arguments:
 * bOrder - Memory order requested
 * znInfo - Zone being tested 
 *
 * Changes:
 * None
 *
 * Returns:
 * ZONE_ALLOCABLE: Memory can be allocated from the zone
 * ZONE_RESERVED_ONLY: Memory can be allocated, from reserves only
 * ZONE_BARRIER_ONLY: Memory can be allocated, from 1/8 reserves only
 * ZONE_OVERLOAD: Memory requested is greater than overall zone size
 * ZONE_LOADED: Memory isn't available enough
 *
 * @Version 1
 * @Since Circuit 2.03
 * @Author Shukant Pal
 */
static
ZNALLOC ZnTest(ULONG bOrder, ZNINFO *znInfo){
	ULONG bRequired = (1 << bOrder);
//	if(bRequired >= znInfo->MmSize)
//		return (ZONE_OVERLOAD);
//	else {
		ULONG mmFree = znInfo->MmSize - znInfo->MmAllocated;
		if(bRequired >= mmFree)
			return (ZONE_LOADED);

		ULONG mmNonReserved = mmFree - znInfo->MmReserved;
		if(bRequired <= mmNonReserved)
			return (ZONE_ALLOCABLE);
		else {
			mmNonReserved += 7 * znInfo->MmReserved / 8;
			if(bRequired <= mmNonReserved)
				return (ZONE_RESERVE_ONLY);
			else
				return (ZONE_BARRIER_ONLY);
		}
//	}
}

#define ZONE_ALLOCATE 0xF1
#define ZONE_SWITCH 0xF2
#define ZONE_FAILURE 0xFF
typedef ULONG ZNACTION;

/**
 * ZnAction() - 
 *
 * Summary:
 * This function returns the action associated with the allocation. It tests the zone using
 * ZnTest() and checks the flags, returning what to do, further in the allocation.
 *
 * Args:
 * bOrder - Memory order requested
 * znFlags - Flags controlling allocation
 * znInfo - Required zone
 *
 * Changes:
 * None
 *
 * Returns:
 * ZONE_ALLOCATE: Allocate from this zone
 * ZONE_SWITCH: Try another zone
 * ZONE_FAILURE: Show failure status
 *
 * @Version 1
 * @Since Circuit 2.03
 * @Author Shukant Pal
 */
static
ZNACTION ZnAction(ULONG bOrder, ZNFLG znFlags, ZNINFO *znInfo){
	ZNALLOC znState = ZnTest(bOrder, znInfo);
	if(znState != ZONE_ALLOCABLE) {
		if(znState < ZONE_OVERLOAD) {
			if(znState == ZONE_RESERVE_ONLY && (FLAG_SET(znFlags, ATOMIC) || FLAG_SET(znFlags, NO_FAILURE)))
				return (ZONE_ALLOCATE);
			else if(znState == ZONE_BARRIER_ONLY && (FLAG_SET(znFlags, NO_FAILURE)))
				return (ZONE_ALLOCATE);
		}
	} else
		return (ZONE_ALLOCATE);

	if(FLAG_SET(znFlags, ZONE_REQUIRED))
		return (ZONE_FAILURE);
	else
		return (ZONE_SWITCH);
}

/**
 * ZnChoose() - 
 * 
 * Summary:
 * This function is used for picking the correct zone for allocation for the given paramaters
 * of the client (mm-mgr). 
 *
 * Args:
 * ULONG bOrder - Memory order required 
 * znBasePref - Minimum zone preference
 * znFlags - Allocation-controlling flags
 * znInfo - Zone most preferred
 * znSys - Zoning system being used 
 *
 * Changes:
 * None
 * 
 * Returns:
 * Zone from which memory is to be allocated OR NULL.
 *
 * @Version 1
 * @Since Circuit 2.03
 * @Author Shukant Pal
 */
static
ZNINFO *ZnChoose(ULONG bOrder, ULONG znBasePref, ZNFLG znFlags, ZNINFO *preferredZone, ZNSYS *znSys){
	ULONG znPref = preferredZone->ZnPref;
	ZNACTION znAction;
	ZNINFO *trialZone = preferredZone;
	ZNINFO *headZone = (ZNINFO *) trialZone;// Don't try the preferred zone twice, foolishly
	while(znPref >= znBasePref){
		do {
			SpinLock(&trialZone->Lock);
			znAction = ZnAction(bOrder, znFlags, trialZone);

			if(znAction != ZONE_ALLOCATE && znAction != ZONE_FAILURE) {
				trialZone = (ZNINFO*) (trialZone->RightLinker);
				SpinUnlock(&trialZone->Lock);
			} else
				return (trialZone);// Keep the zone locked, to prevent allocator's function
		} while(trialZone != headZone);

		--znPref;
		trialZone = (ZNINFO*) znSys->ZnPref[znPref].ZoneList.ClnMain;
		headZone = trialZone;
	}

	return (NULL);
}

BDINFO *ZnAllocateBlock(ULONG bOrder, ULONG znBasePref, ZNINFO *znInfo, ZNFLG znFlags, ZNSYS *znSys){
	ZNINFO *znAllocator = ZnChoose(bOrder, znBasePref, znFlags, znInfo, znSys);

	if(znAllocator == NULL)
		return (NULL);
	else {
		if(!FLAG_SET(znFlags, ZONE_NO_CACHE) && znSys->ZnCacheRefill != 0 && bOrder == 0) {
			SpinUnlock(&znAllocator->Lock);
			ULONG chStatus;
			LINODE *chData = ChDataAllocate(&znAllocator->Cache, &chStatus);

			if(FLAG_SET((ULONG) chStatus, CH_POPULATE)) {
				SpinLock(&znAllocator->Lock);
				chStatus &= ~1;
				CHREG *chReg = (CHREG *) chStatus;
				ULONG chDataSize = znAllocator->MmManager.DescriptorSize;

				BDINFO *chPopulater = BdAllocateBlock(znSys->ZnCacheRefill, &znAllocator->MmManager);
				if(chPopulater != NULL) {
					for(ULONG dOffset=0; dOffset<(1 << znSys->ZnCacheRefill); dOffset++) {
						chPopulater->UpperOrder = 0;
						chPopulater->LowerOrder = 0;
						PushHead((LINODE*) chPopulater, &chReg->DList);
						chPopulater = (BDINFO *) ((ULONG) chPopulater + chDataSize);
					}
				}

				SpinUnlock(&znAllocator->Lock);
			}

			if(chData != NULL) {
				return (BDINFO*) (chData);
			} else {
				return (BDINFO*) (ChDataAllocate(&znAllocator->Cache, &chStatus));
			}
		}

		znAllocator->MmAllocated += (1 << bOrder);
		BDINFO *allocatedBlock = BdAllocateBlock(bOrder, &znAllocator->MmManager);
		SpinUnlock(&znAllocator->Lock);
		return (allocatedBlock);
	}
}

VOID ZnFreeBlock(BDINFO *bdInfo, ZNSYS *znSys){
	ZNINFO *znInfo = znSys->ZnSet + bdInfo->ZnOffset;
	znInfo->MmAllocated -= (1 << bdInfo->UpperOrder);
	BdFreeBlock(bdInfo, &znInfo->MmManager);
}

BDINFO *ZnExchangeBlock(BDINFO *bdInfo, ULONG *status, ULONG znBasePref, ULONG znFlags, ZNSYS *znSys){
	ZNINFO *zone = &znSys->ZnSet[bdInfo->ZnOffset];
	SpinLock(&zone->Lock);
	BDINFO *ebInfo = BdExchangeBlock(bdInfo, &(zone->MmManager), status); // @Prefix eb - Exchanged Block
	SpinUnlock(&zone->Lock);
	if(ebInfo == NULL) {
		return (ZnAllocateBlock(bdInfo->LowerOrder + 1, znBasePref, zone, znFlags, znSys));
	} else if((ULONG) ebInfo == BD_ERR_FREE) {
		return (NULL);
	} else {
		return (ebInfo);
	}
}

VOID ZnReverseMap(USHORT znOffset, ZNINFO *znInfo, ZNSYS *znSys){
	ULONG biSize = znInfo->MmManager.DescriptorSize;
	BDINFO *bInfo;
	ULONG bTable = (ULONG) znInfo->MmManager.DescriptorTable;
	ULONG bFence = bTable + znInfo->MmSize * biSize;

	while(bTable < bFence) {
		bInfo = (BDINFO*) bTable;
		bInfo->ZnOffset = znOffset;
		bTable += biSize;
	}
}
