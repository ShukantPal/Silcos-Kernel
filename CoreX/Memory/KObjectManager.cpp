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

const char *nmObjectInfo = "@KObjectManager::ObjectInfo"; /* Name of ObjectInfo */
ObjectInfo tObjectInfo;
const char *nmSlab = "@KObjectManager::Slab"; /* Name of OBSLAB */
ObjectInfo tSlab;
ObjectInfo *tAVLNode;
char nmLinkedList[] = "LinkedList";
ObjectInfo *tLinkedList;

void obSetupAllocator(Void)
{
	tObjectInfo.name = nmObjectInfo;
	tObjectInfo.rawSize = sizeof(ObjectInfo);
	tObjectInfo.colorScheme = 0;
	tObjectInfo.align = L1_CACHE_ALIGN;
	tObjectInfo.bufferSize = BUFFER_SIZE(sizeof(ObjectInfo), L1_CACHE_ALIGN);
	tObjectInfo.bufferPerSlab = (KPGSIZE - sizeof(Slab)) / BUFFER_SIZE(sizeof(ObjectInfo), L1_CACHE_ALIGN);
	tObjectInfo.bufferMargin = (KPGSIZE - sizeof(Slab)) % BUFFER_SIZE(sizeof(ObjectInfo), L1_CACHE_ALIGN);

	tSlab.name = nmSlab;
	tSlab.rawSize = sizeof(Slab);
	tSlab.colorScheme = 0;
	tSlab.align = NO_ALIGN;
	tSlab.bufferSize = sizeof(Slab);
	tSlab.bufferPerSlab = (KPGSIZE - sizeof(Slab)) / sizeof(Slab);
	tSlab.bufferMargin = (KPGSIZE - sizeof(Slab)) % sizeof(Slab);
}

void SetupPrimitiveObjects(void)
{
	tLinkedList = KiCreateType(nmLinkedList, sizeof(LinkedList), NO_ALIGN, NULL, NULL);
}

CLIST tList; /* List of active kernel object managers */

/**
 * Function: ObCreateSlab
 *
 * Summary:
 * Creates new slab, with all buffers linked and constructed objects. It also
 * writes the signature of the object into the page-forum. The Slab struct is
 * placed at the very end of the page to reduce TLB usage during allocation &
 * deallocation operations.
 *
 * Args:
 * ObjectInfo* metaInfo - information of the buffers to keep in the new slab
 * unsigned long kmSleep - tells whether waiting for memory is allowed
 *
 * Version: 1
 * Since: Circuit 2.03
 * Author: Shukant Pal
 */
static Slab* ObCreateSlab(ObjectInfo *metaInfo, unsigned long kmSleep)
{
	ADDRESS pageAddress;
	Slab *newSlab;

	unsigned long slFlags = (kmSleep) ? (FLG_ATOMIC) : (FLG_NONE);
	pageAddress = KiPagesAllocate(0, ZONE_KOBJECT, slFlags);
	EnsureUsability(pageAddress, NULL, slFlags, KernelData);

	newSlab = (Slab *) (pageAddress + KPGSIZE - sizeof(Slab));
	newSlab->colouringOffset = metaInfo->colorScheme;
	newSlab->bufferStack.Head = NULL;
	newSlab->freeCount = metaInfo->bufferPerSlab;

	unsigned long obPtr = pageAddress, bufferSize = metaInfo->bufferSize;
	unsigned long bufferFence =
				pageAddress + ((bufferSize > (KPGSIZE / 8)) ?
					KPGSIZE :
					(KPGSIZE - sizeof(Slab))) - bufferSize;
	Stack *bufferStack = &newSlab->bufferStack;
	void (*ctor) (void *) = metaInfo->ctor;

	if(ctor != NULL)
	{
		while(obPtr < bufferFence)
		{
			ctor((void*) obPtr);
			PushElement((STACK_ELEMENT *) obPtr, bufferStack);
			obPtr += bufferSize;
		}
	}
	else
	{
		while(obPtr < bufferFence)
		{
			PushElement((StackElement*) obPtr, bufferStack);
			obPtr += bufferSize;
		}
	}

	KPAGE *slabPage = KPG_AT(pageAddress);
	slabPage->HashCode = (unsigned long) metaInfo;

	return (newSlab);
}

/**
 * Function: ObDestroySlab
 *
 * Summary:
 * Deletes a totally unused object-slab, by calling all object destructors and
 * by freeing the cached free-slab. This slab becomes the new cached free-slab
 * for the object meta-data.
 *
 * Args:
 * Slab* emptySlab - any unused slab, out of which no objects are allocated
 * ObjectInfo* metaInfo - information about the object & slab-caches
 *
 * Version: 1.1
 * Since: Circuit 2.03
 * Author: Shukant Pal
 */
static void ObDestroySlab(Slab *emptySlab, ObjectInfo *metaInfo)
{
	if(metaInfo->dtor != NULL)
	{
		STACK_ELEMENT *objectPtr = emptySlab->bufferStack.Head;
		void (*destructObject)(void*)  = metaInfo->dtor;
		while(objectPtr != NULL)
		{
			destructObject((void *) objectPtr);
			objectPtr = objectPtr->Next;
		}
	}

	ADDRESS pageAddress;
	if(metaInfo->rawSize <= (KPGSIZE / 8))
	{
		pageAddress = (ADDRESS) emptySlab & ~((1 << KPGOFFSET) - 1);
	}
	else
	{
		// TODO : Get pageAddress using self-scaling hash tree
		pageAddress = 0;
	}

	MMFRAME *mmFrame = GetFrames(pageAddress, 1, NULL);
	KeFrameFree(FRADDRESS(mmFrame));
	KiPagesFree(pageAddress);
}

/**
 * Function: ObFindSlab
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
static Slab *ObFindSlab(ObjectInfo *metaInfo, unsigned long kmSleep)
{
	if(metaInfo->partialList.ClnCount)
	{
		return (Slab *) (metaInfo->partialList.ClnMain);
	}
	else
	{
		Slab *emptySlab = metaInfo->emptySlab;
		if(emptySlab == NULL)
			emptySlab = ObCreateSlab(metaInfo, kmSleep);

		ClnInsert((CircularListNode*) emptySlab, CLN_LAST, &metaInfo->partialList);
		return (emptySlab);
	}
}

/**
 * Function: ObPlaceSlab
 *
 * Summary:
 * This function places a slab (from which a object was allocated), into the required
 * list.
 *
 * Args:
 * Slab* slab - the slab from which a object was unlinked (allocated)
 * ObjectInfo* metaInfo - meta-data for the object-type
 *
 * Changes: To partial list and others
 *
 * Version: 1
 * Since: Circuit 2.03
 * Author: Shukant Pal
 */
static void ObPlaceSlab(Slab *slab, ObjectInfo *metaInfo)
{
	if(slab->freeCount == 0) {
		ClnRemove((CLNODE *) slab, &metaInfo->partialList);
		ClnInsert((CLNODE *) slab, CLN_FIRST, &metaInfo->fullList);
	}
}

/**
 * Function: ObRecheckSlab
 *
 * Summary:
 * This function is used to position the slab, after deallocation of an object
 * from the the allocator. It will place a slab, which is empty now but was not
 * before freeing, to the partial list and it will remove a slab, which was
 * full before, from the full list and move it to the partial force.
 *
 * If the slab cache (EmptySlab), is not free, the the slab placed before is
 * destroyed.
 *
 * Args:
 * Slab* slab - the slab from which a object was linked (or freed)
 * ObjectInfo* metaInfo - meta-data for the object-type
 *
 * Changes: To partial list and others
 *
 * Version: 1.1
 * Since: Circuit 2.03
 * Author: Shukant Pal
 */
static void ObRecheckSlab(Slab *slab, ObjectInfo *metaInfo)
{
	if(slab->freeCount == 1)
	{ // Came from full list
		ClnRemove((CLNODE *) slab, &metaInfo->fullList);
		ClnInsert((CLNODE *) slab, CLN_FIRST, &metaInfo->partialList);
	}
	else if(slab->freeCount == metaInfo->bufferPerSlab)
	{
		ClnRemove((CLNODE *) slab, &metaInfo->partialList);

		Slab *oldEmptySlab = metaInfo->emptySlab;
		metaInfo->emptySlab = slab;
		if(oldEmptySlab != NULL)
			ObDestroySlab(oldEmptySlab, metaInfo);
	}
}

/**
 * Function: KNew
 *
 * Summary:
 * Allocates a constructed object, given its static & runtime information forum
 * and has capability to wait until memory is available.
 *
 * Args:
 * ObjectInfo* metaInfo - static & runtime information about the object
 * unsigned long kmSleep - tells whether waiting for memory is allowed
 *
 * See: KM_SLEEP & KM_NOSLEEP
 * Version: 1.0
 * Since: Circuit 2.03
 * Author: Shukant Pal
 */
extern "C" void *KNew(ObjectInfo *metaInfo, unsigned long kmSleep)
{
	SpinLock(&metaInfo->lock);
	Slab *freeSlab = ObFindSlab(metaInfo, kmSleep);
	void *object = NULL;

	if(freeSlab != NULL)
	{
		void *freeObject = PopElement(&freeSlab->bufferStack);
		--(freeSlab->freeCount);
		ObPlaceSlab(freeSlab, metaInfo);
		object = freeObject;
	}
	
	SpinUnlock(&metaInfo->lock);
	return (object);
}

/**
 * Function: KDelete
 *
 * Summary:
 * Simply deallocates the object and allows it to be allocated later, given its
 * runtime information.
 *
 * Args:
 * void* object - ptr to allocated object
 * ObjectInfo* metaInfo - runtime information about the object
 *
 * Author: Shukant Pal
 */
extern "C" void KDelete(void *object, ObjectInfo *metaInfo)
{
	SpinLock(&metaInfo->lock);

	Slab *slab = (Slab *) (((unsigned long) object & ~(KPGSIZE - 1)) + (KPGSIZE - sizeof(Slab)));
	PushElement((STACK_ELEMENT *) object, &slab->bufferStack);
	++(slab->freeCount);
	ObRecheckSlab(slab, metaInfo);
	
	SpinUnlock(&metaInfo->lock);
}

/**
 * Function: KiCreateType
 *
 * Summary:
 * Creates a new-object information struct, given its initial parameters, and
 * returns it for usage.
 *
 * Args:
 * const char *name - pointer to a static memory location containing the name
 * unsigned long size - byte-size of the object
 * unsigned long align - alignment constraints for the object
 * void (*ctor)(void*) - a constructor for the object (C++ linkage)
 * void (*dtor)(void*) - a destructor for the object (C++ linkage)
 *
 * Version: 1.1
 * Since: Circuit 2.03
 * Author: Shukant Pal
 */
extern "C" ObjectInfo *KiCreateType(const char *tName, unsigned long tSize, unsigned long tAlign,
					void (*ctor) (void *), void (*dtor) (void *))
{
	ObjectInfo *typeInfo = (ObjectInfo*) KNew(&tObjectInfo, 1);

	if(typeInfo == NULL) DbgLine("NULLIFED");
	typeInfo->name = tName;
	typeInfo->rawSize = tSize;
	typeInfo->align = tAlign;
	typeInfo->bufferSize = (tSize % tAlign) ? (tSize + tAlign - tSize % tAlign) : (tSize);
	typeInfo->bufferPerSlab = (KPGSIZE - sizeof(Slab)) / typeInfo->bufferSize;
	typeInfo->bufferMargin = (KPGSIZE - sizeof(Slab)) % typeInfo->bufferSize;
	typeInfo->ctor = ctor;
	typeInfo->dtor = dtor;
	typeInfo->callCount = 0;
	typeInfo->emptySlab = NULL;
	typeInfo->partialList.ClnMain = NULL;
	typeInfo->partialList.ClnCount = 0;
	typeInfo->fullList.ClnMain = NULL;
	typeInfo->fullList.ClnCount= 0;
	ClnInsert((CLNODE*) typeInfo, CLN_LAST, &tList);

	return (typeInfo);
}

/**
 * Function: KiDestroyType
 *
 * Summary:
 * Removes the object & its allocator, if and only if no objects are currently
 * outside the allocator.
 *
 * Args:
 * ObjectInfo* obj - meta-data for the object
 *
 * Returns:
 * FALSE, if any objects are in circulation and its wasn't freed; TRUE, if it
 * is now freed, and further objects SHOULD NOT be allocated.
 *
 * Author: Shukant Pal
 */
extern "C" unsigned long KiDestroyType(ObjectInfo *typeInfo)
{
	if(typeInfo->partialList.ClnCount != 0 || typeInfo->fullList.ClnCount != 0)
	{
		return (FALSE);
	}
	else
	{
		ClnRemove((CLNODE*) typeInfo, &tList);

		Slab *emptySlab = (Slab*) typeInfo->emptySlab;
		if(emptySlab != NULL)
			ObDestroySlab(emptySlab, typeInfo);

		return (TRUE);
	}
}
