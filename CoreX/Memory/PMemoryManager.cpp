/* Copyright (C) 2017 - Shukant Pal */

#define NS_KMEMORYMANAGER

#include <Memory/KMemoryManager.h>
#include <Memory/KObjectManager.h>
#include <Memory/PMemoryManager.h>
#include <KERNEL.h>

char *nmPSMM_MANAGER = "PSMM_MANAGER";
char *nmPSMM_REGION = "PSMM_REGION";

ObjectInfo *tPSMM_MANAGER;
ObjectInfo *tPSMM_REGION;

void PMgrConstruct(void *pmgrPtr){
	PSMM_MANAGER *mgr = pmgrPtr;
	MM_MANAGER *mm = &mgr->MmMgr;

	mm->MgrName = nmPSMM_MANAGER;
	mm->MgrTp = PSMM_TP;
	mm->ReferCount = 0;
	mgr->PmFlags = 0;
	mm->CODE = NULL; mm->DATA = NULL; mm->BSS = NULL; mm->HEAP = NULL; mm->STACK = NULL;
	mm->RegionList.Head = NULL; mm->RegionMap.Root = NULL;
	mm->MmInsertRegion = &PInsertRegion;
	mm->MmRemoveRegion = &PRemoveRegion;
	mm->MmExtendRegion = &PExtendRegion;
	mm->MmValidateAddress = &PValidateAddress;

	memsetf(&mgr->LastUsed, 0, sizeof(PSMM_MANAGER) - sizeof(MM_MANAGER));
}

void PRegionConstruct(void *prgPtr){
	memsetf(prgPtr, 0, sizeof(MM_REGION));
}

/******************************************************************************
 * Function: PMgrCreate()
 *
 * Summary: This function allocates a new PSMM manager from the slab cache given. If the
 * slab cache is yet not initialized, it does so and also initializes the memory region cache. Then
 * it puts the values of the bounds given into the new manager. Also, the new manager will
 * have a new reference count of 1.
 *
 * Args:
 * unsigned long codeBounds[2] -
 * unsigned long dataBounds[2] -
 * unsigned long bssBounds[2] -
 *
 * @Version 1
 * @Since Circuit 2.03
 ******************************************************************************/
PSMM_MANAGER *PMgrCreate(unsigned long codeBounds[2], unsigned long dataBounds[2], unsigned long bssBounds[2]){
	if(tPSMM_MANAGER == NULL)
		tPSMM_MANAGER = KiCreateType(nmPSMM_MANAGER, sizeof(PSMM_MANAGER), sizeof(unsigned long), &PMgrConstruct, NULL);

	if(tPSMM_REGION == NULL)
		tPSMM_REGION = KiCreateType(nmPSMM_REGION, sizeof(MM_REGION), sizeof(unsigned long), &PRegionConstruct, NULL);

	PSMM_MANAGER *mgr = KNew(tPSMM_MANAGER, KM_SLEEP);
	mgr->CodeBounds[0] = codeBounds[0]; mgr->CodeBounds[1] = codeBounds[1];
	mgr->DataBounds[0] = dataBounds[0]; mgr->DataBounds[1] = dataBounds[1];
	mgr->BSSBounds[0] = bssBounds[0]; mgr->BSSBounds[1] = bssBounds[1];
	mgr->MmMgr.ReferCount = 1;
	return (mgr);
}

/******************************************************************************
 * Function: PFindRegion()
 *
 * Summary: This function searchs for a memory region which contains the given
 * address by doing a BST-search through the region map, if the MAP flag is set, else,
 * it serially goes through the region list and tests each region, whether or not it contains
 * the given address. If a region is found containing the address, it is immediately returned.
 *
 * Args:
 * unsigned long rgAddress - Address to be searched for
 * MM_MANAGER *mgr - Memory manager
 *
 * Returns: This function returns the memory region unchanged.
 *
 * @Version 1
 * @Since Circuit 2.03
 ******************************************************************************/
static
MM_REGION *PFindRegion(unsigned long rgAddress, PSMM_MANAGER *mgr){
	unsigned long nodeAddress;
	unsigned long nodeLimit;
	if(FLAG_SET(mgr->PmFlags, PM_MAP)){ /* BST-search for the region */
		AVLTREE *regionTree = &mgr->MmMgr.RegionMap;
		AVLNODE *regionNode = regionTree->Root;

		while(regionNode != NULL){
			nodeAddress = ((MM_REGION*) regionNode)->Address;
			nodeLimit = ((MM_REGION*) regionNode)->Size + nodeAddress;

			if(nodeAddress >= rgAddress && rgAddress < nodeLimit)
				return ((MM_REGION*) regionNode);
			else if(nodeAddress<rgAddress)
				regionNode = regionNode->Right;
			else if(nodeAddress>rgAddress)
				regionNode = regionNode->Left;
		}

		return (NULL);
	} else { /* Serial-search for the region */
		LinkedList *regionList = &mgr->MmMgr.RegionList;
		MM_REGION *regionInfo = NULL;
		LinkedListNode *regionLinker = regionList->Head;

		while(regionLinker != NULL){
			regionInfo = (MM_REGION*) ((unsigned long) regionLinker - sizeof(AVLNODE));
			nodeAddress = regionInfo->Address; nodeLimit = regionInfo->Size + nodeAddress;
			if(nodeAddress >= rgAddress && rgAddress < nodeLimit)
				return (regionInfo);
			else
				regionLinker = regionLinker->Next;
		}
		return (NULL);
	}
}

static
void PDeleteRegion(MM_REGION *region, MM_MANAGER *pmgr){
	AVLDelete(region->Address, &pmgr->RegionMap);
	RemoveElement(&region->LiLinker, &pmgr->RegionList);
}

static
unsigned long PSplitRegion(unsigned long rgAddress, unsigned long rgLimit, MM_REGION *region, MM_MANAGER *pmgr){
	unsigned long olAddress = region->Address;
	unsigned long olLimit = olAddress + region->Size;

	unsigned long leftMargin = (rgAddress > olAddress) ? (rgAddress - olAddress) : 0;
	unsigned long rightMargin = (rgLimit < olLimit) ? (olLimit - rgLimit) : 0;

	PDeleteRegion(region, pmgr);

	if(leftMargin)
		PInsertRegion(olAddress, leftMargin, region->Flags, region->PagerFlags, pmgr);
	if(rightMargin)
		PInsertRegion(olLimit - rightMargin, rightMargin, region->Flags, region->PagerFlags, pmgr);

	if(leftMargin || rightMargin)
		return (RG_UNDERLAP);
	else
		return (RG_REMOVE);
}

/******************************************************************************
 * Function : PInsertRegion()
 *
 * Summary: This function adds the memory region into the record of the PSMM manager
 * given. If the MAP flags is set, then it also adds it to the region map, otherwise, it is inserted
 * into the ordered position in the region list. In case, the count of regions increased to 32
 * then the MAP flag is set, and then the region-map is built.
 * 
 * A failure can occur in the two cases -
 * 1. Memory region is not allocable at the moment - 
 * 2. Given region boundaries overlap with another region - RG_FAILURE
 *
 * Args:
 * unsigned long rgAddress - The initial address of the region, which is decided by the client only
 * unsigned long rgSize - The size of the region, which is decided by the client only
 * unsigned long rgFlags - This contains the region's type and its control flags, partially given by the kernel
 * PAGE_ATTRIBTUTES pgFlags - Mapping-flags for the region, decided by the kernel
 * MM_MANAGER *pmgr - PSMM-compatible memory manager
 *
 * Returns:
 * 1. RG_INSERT - This indicates the region was sucessfully inserted
 * 2. RG_FAILURE - This indicates the region was not inserted due to some normal error
 *
 * @Version 1
 * @Since Circuit 2.03
 ******************************************************************************/
unsigned long PInsertRegion(unsigned long rgAddress, unsigned long rgSize, unsigned long rgFlags, PAGE_ATTRIBUTES pgFlags, MM_MANAGER *pmgr){
	MM_REGION *overlapRegion = (MM_REGION *) PFindRegion(rgAddress, (convert_psmm) pmgr);

	if(overlapRegion == NULL){
		MM_REGION *newRegion = KNew(tPSMM_REGION, KM_SLEEP);
		newRegion->Address = rgAddress;
		newRegion->Size = rgSize;
		newRegion->Flags = rgFlags;
		newRegion->PagerFlags = pgFlags;

		AVLInsert(&newRegion->Node, &pmgr->RegionMap);

		MM_REGION *nextRegion = (MM_REGION *) AVLFindGTE(rgAddress+rgSize, &pmgr->RegionMap);
		if(nextRegion == NULL)
			AddElement(&newRegion->LiLinker, &pmgr->RegionList);
		else
			InsertElementBefore(&nextRegion->LiLinker, &newRegion->LiLinker, &pmgr->RegionList);

		return (RG_INSERT);
	}

	return (RG_FAILURE);
}

unsigned long PRemoveRegion(unsigned long rgAddress, unsigned long rgSize, unsigned short rgType, MM_MANAGER *pmgr){
	MM_REGION *container = (MM_REGION *) PFindRegion(rgAddress, (convert_psmm) pmgr);

	if(container == NULL)
		return (RG_FAILURE);

	return (PSplitRegion(rgAddress, rgSize, container, pmgr));
}

unsigned long PExtendRegion(unsigned long rgAddress, unsigned long exAddress, MM_MANAGER *pmgr){
	MM_REGION *region = (MM_REGION *) PFindRegion(rgAddress, (convert_psmm) pmgr);

	rgAddress = region->Address;
	unsigned long rgLimit = rgAddress + region->Size;

	if(rgAddress <= exAddress && exAddress < rgLimit)
		return (RG_FAILURE);

	MM_REGION *adjRegion;
	if(exAddress < rgAddress){
		adjRegion = region->PreviousLinker;
		if(exAddress > adjRegion->Address + adjRegion->Size){
			region->Size += rgAddress - exAddress;
			region->Address = exAddress;
		}
	} else {
		adjRegion = region->NextLinker;
		if(exAddress < adjRegion->Address){
			unsigned long oldSize = region->Size;
			region->Size += exAddress - rgAddress - oldSize;
		}
	}

	return (RG_EXTEND);
}

MM_REGION *PValidateAddress(unsigned long address, MM_MANAGER *pmgr){
	return (MM_REGION *) (PFindRegion(address, (convert_psmm) pmgr));
}

void PMgrDispose(PSMM_MANAGER *mgr){
	--(mgr->MmMgr.ReferCount);
	if(mgr->MmMgr.ReferCount == 0){
		KDelete(mgr, tPSMM_MANAGER);
	}
}
