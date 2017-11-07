/* Copyright (C) 2017 - Shukant Pal */

#define NS_KFRAMEMANAGER
#define NS_KMEMORYMANAGER

#include <Memory/KMemoryManager.h>
#include <Memory/KMemorySpace.h>
#include <Memory/KObjectManager.h>
#include <Memory/MemoryTransfer.h>
#include <KERNEL.h>

/*
 * Size of buffer (object aligned to memory)
 */
#define BUFFER_SIZE(ObSize, ObAlign) ((ObSize % ObAlign) ? (ObSize + ObAlign - (ObSize % ObAlign)) : ObSize)

CHAR nmOBINFO[7] = "OBINFO"; /* Name of OBINFO */

/*
 * Object-types, for alloc
 */
OBINFO tOBINFO;

CHAR nmOBSLAB[7] = "OBSLAB"; /* Name of OBSLAB */

/*
 * Slab descriptors, for large object sizes, to implement them in a hash tree.
 */
OBINFO tOBSLAB;

ObjectInfo *tAVLNode;

CHAR nmLinkedList[] = "LinkedList";

/*
 * Linked List for general purpose use (prim. type), and also the
 * GenericLinkedListNode can be allocated from this for temporary usage.
 */
ObjectInfo *tLinkedList;

void obSetupAllocator(Void)
{
	tOBINFO.ObName = nmOBINFO;
	tOBINFO.ObSize = sizeof(OBINFO);
	tOBINFO.ObColor = 0;
	tOBINFO.ObAlign = L1_CACHE_ALIGN;
	tOBINFO.BufferSize = BUFFER_SIZE(sizeof(OBINFO), L1_CACHE_ALIGN);
	tOBINFO.BufferPerSlab = (KPGSIZE - sizeof(OBSLAB)) / BUFFER_SIZE(sizeof(OBINFO), L1_CACHE_ALIGN);
	tOBINFO.BufferMargin = (KPGSIZE - sizeof(OBSLAB)) % BUFFER_SIZE(sizeof(OBINFO), L1_CACHE_ALIGN);

	tOBSLAB.ObName = nmOBSLAB;
	tOBSLAB.ObSize = sizeof(OBSLAB);
	tOBSLAB.ObColor = 0;
	tOBSLAB.ObAlign = NO_ALIGN;
	tOBSLAB.BufferSize = sizeof(OBSLAB);
	tOBSLAB.BufferPerSlab = (KPGSIZE - sizeof(OBSLAB)) / sizeof(OBSLAB);
	tOBSLAB.BufferMargin = (KPGSIZE - sizeof(OBSLAB)) % sizeof(OBSLAB);
}

void SetupPrimitiveObjects(void)
{
	tLinkedList = KiCreateType(nmLinkedList, sizeof(LinkedList), NO_ALIGN, NULL, NULL);
}

CLIST tList; /* List of active kernel object managers */

/**
 * ObCreateSlab() - 
 *
 * Summary:
 * This function is used for making a new slab. It uses the general (CacheRegister)
 * slab cache to store recently used mapped slabs. If the cache is not currently free,
 * then a new page-frame and a new kernel page is allocated and mapped. Then, it
 * is setup properly and returned.
 *
 * The caching technique allows better TLB usage.
 *
 * Args:
 * metaInfo - Object-type info 
 * kmSleep - Whether to sleep in page-frame or page allocation
 *
 * Changes: NONE
 *
 * Returns: VOID
 *
 * @Version 1
 * @Since Circuit 2.03
 */
static
OBSLAB *ObCreateSlab(OBINFO *metaInfo, ULONG kmSleep){
	ADDRESS pageAddress;
	OBSLAB *newSlab;

	ULONG slFlags = (kmSleep) ? (FLG_ATOMIC) : (FLG_NONE);
	pageAddress = KiPagesAllocate(0, ZONE_KOBJECT, slFlags);
	EnsureUsability(pageAddress, NULL, slFlags, KernelData);

	newSlab = (OBSLAB *) (pageAddress + KPGSIZE - sizeof(OBSLAB));
	newSlab->ObColor = metaInfo->ObColor;
	newSlab->ObStack.Head = NULL;
	newSlab->ObCount = metaInfo->BufferPerSlab;

	ULONG obPtr = pageAddress;
	STACK *bufferStack = &newSlab->ObStack;
	ULONG bufferSize = metaInfo->BufferSize;
	ULONG bufferFence = pageAddress + ((bufferSize > (KPGSIZE / 8)) ? KPGSIZE : (KPGSIZE - sizeof(OBSLAB))) - bufferSize;
	VOID (*objectConstructor) (VOID *) = metaInfo->ObConstruct;
	while(obPtr < bufferFence) {
		if(objectConstructor != NULL)
			objectConstructor(obPtr);
		PushElement((STACK_ELEMENT *) obPtr, bufferStack);
		obPtr += bufferSize;
	}

	KPAGE *slabPage = KPG_AT(pageAddress);
	slabPage->HashCode = (ULONG) metaInfo;

	return (newSlab);
}

static
VOID ObDestroySlab(OBSLAB *emptySlab, OBINFO *metaInfo){
	if(metaInfo->ObDestruct != NULL) {
		STACK_ELEMENT *objectPtr = emptySlab->ObStack.Head;
		VOID (*destructObject) (VOID *)  = metaInfo->ObDestruct;
		while(objectPtr != NULL) {
			destructObject((VOID *) objectPtr);
			objectPtr = objectPtr->Next;
		}
	}

	ADDRESS pageAddress;
	if(metaInfo->ObSize <= (KPGSIZE / 8)) {
		pageAddress = (ADDRESS) emptySlab & ~((1 << KPGOFFSET) - 1);
	} else {
		// TODO : Get pageAddress using self-scaling hash tree
		pageAddress = 0;
	}

	MMFRAME *mmFrame = GetFrames(pageAddress, 1, NULL);
	KeFrameFree(FRADDRESS(mmFrame));
	KiPagesFree(pageAddress);
}

/**
 * ObFindSlab() - 
 *
 * Summary:
 * This function finds a partially/fully free slab, that can be used to allocate objects. It
 * looks first into the partial list, otherwise a empty slab is look for. If a empty slab is not
 * available, one is created and added to the partial list.
 *
 * Args:
 * metaInfo - Object-type info
 * kmSleep - To sleep, if a new slab is created
 *
 * Changes: To the partial list
 *
 * Returns: A partially/fully free slab (linked in partial list)
 *
 * @Version 1
 * @Since Circuit 2.03
 */
static
OBSLAB *ObFindSlab(OBINFO *metaInfo, ULONG kmSleep){
	if(metaInfo->PartialList.ClnCount){
		return (OBSLAB *) (metaInfo->PartialList.ClnMain);
	} else {
		OBSLAB *emptySlab = metaInfo->EmptySlab;
		if(emptySlab == NULL)
			emptySlab = ObCreateSlab(metaInfo, kmSleep);

		ClnInsert((CLNODE*) emptySlab, CLN_LAST, &metaInfo->PartialList);
		return (emptySlab);
	}
}

/**
 * ObPlaceSlab() -
 *
 * Summary:
 * This function places a slab (from which a object was allocated), into the required
 * list.
 *
 * Args:
 * slab - The slab, just from which a object was allocated
 * metaInfo - Object-type info
 *
 * Changes: To partial list and others
 *
 * Returns: VOID
 *
 * @Version 1
 * @Since Circuit 2.03
 */
static
VOID ObPlaceSlab(OBSLAB *slab, OBINFO *metaInfo){
	if(slab->ObCount == 0) {
		ClnRemove((CLNODE *) slab, &metaInfo->PartialList);
		ClnInsert((CLNODE *) slab, CLN_FIRST, &metaInfo->FullList);
	}
}

/******************************************************************************
 * ObRecheckSlab() - 
 *
 * Summary: This function is used to position the slab, after deallocation of an object from
 * the the allocator. It will place a slab, which is empty now but was not before freeing, to
 * the partial list and it will remove a slab, which was full before, from the full list and move
 * it to the partial force.
 *
 * If the slab cache (EmptySlab), is not free, the the slab placed before is destroyed.
 *
 * Args:
 * slab - The slab, to which a object was just freed
 * metaInfo - Object-type info
 *
 * Changes: To partial list and others
 *
 * Returns: VOID
 *
 * @Version 1
 * @Since Circuit 2.03
 ******************************************************************************/
static
VOID ObRecheckSlab(OBSLAB *slab, OBINFO *metaInfo){
	if(slab->ObCount == 1) { // Came from full list
		ClnRemove((CLNODE *) slab, &metaInfo->FullList);
		ClnInsert((CLNODE *) slab, CLN_FIRST, &metaInfo->PartialList);
	} else if(slab->ObCount == metaInfo->BufferPerSlab) {
		ClnRemove((CLNODE *) slab, &metaInfo->PartialList);

		OBSLAB *oldEmptySlab = metaInfo->EmptySlab;
		metaInfo->EmptySlab = slab;
		if(oldEmptySlab != NULL)
			ObDestroySlab(oldEmptySlab, metaInfo);
	}
}

VOID *KNew(OBINFO *metaInfo, ULONG kmSleep){
	SpinLock(&metaInfo->ObLock);
	OBSLAB *freeSlab = ObFindSlab(metaInfo, kmSleep);
	VOID *object = NULL;

	if(freeSlab != NULL) {
		VOID *freeObject = PopElement(&freeSlab->ObStack);
		--(freeSlab->ObCount);
		ObPlaceSlab(freeSlab, metaInfo);
		object = freeObject;
	}
	
	SpinUnlock(&metaInfo->ObLock);
	return (object);
}

VOID KDelete(VOID *object, OBINFO *metaInfo){
	SpinLock(&metaInfo->ObLock);

	OBSLAB *slab = (OBSLAB *) (((ULONG) object & ~(KPGSIZE - 1)) + (KPGSIZE - sizeof(OBSLAB)));
	PushElement((STACK_ELEMENT *) object, &slab->ObStack);
	++(slab->ObCount);
	ObRecheckSlab(slab, metaInfo);
	
	SpinUnlock(&metaInfo->ObLock);
}

OBINFO *KiCreateType(CHAR *tName, ULONG tSize, ULONG tAlign, VOID (*tConstruct) (VOID *), VOID (*tDestruct) (VOID *)){
	OBINFO *typeInfo = KNew(&tOBINFO, 1);
	if(typeInfo == NULL) DbgLine("NULLIFED");
	typeInfo->ObName = tName;
	typeInfo->ObSize = tSize;
	typeInfo->ObAlign = tAlign;
	typeInfo->BufferSize = (tSize % tAlign) ? (tSize + tAlign - tSize % tAlign) : (tSize);
	typeInfo->BufferPerSlab = (KPGSIZE - sizeof(OBSLAB)) / typeInfo->BufferSize;
	typeInfo->BufferMargin = (KPGSIZE - sizeof(OBSLAB)) % typeInfo->BufferSize;
	typeInfo->ObConstruct = tConstruct;
	typeInfo->ObDestruct = tDestruct;
	typeInfo->CallCount = 0;
	typeInfo->EmptySlab = NULL;
	typeInfo->PartialList.ClnMain = NULL;
	typeInfo->PartialList.ClnCount = 0;
	typeInfo->FullList.ClnMain = NULL;
	typeInfo->FullList.ClnCount= 0;
	ClnInsert((CLNODE*) typeInfo, CLN_LAST, &tList);
	return (typeInfo);
}

ULONG KiDestroyType(OBINFO *typeInfo){
	if(typeInfo->PartialList.ClnCount != 0 || typeInfo->FullList.ClnCount != 0) {
		return (FALSE);
	} else {
		ClnRemove((CLNODE*) typeInfo, &tList);

		OBSLAB *emptySlab = (OBSLAB*) typeInfo->EmptySlab;
		if(emptySlab != NULL)
			ObDestroySlab(emptySlab, typeInfo);

		return (TRUE);
	}
}
