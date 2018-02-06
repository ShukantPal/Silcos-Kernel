/* Copyright (C) 2017 - Shukant Pal */

#define NS_KFRAMEMANAGER
#define NS_KMEMORYMANAGER

#include <Memory/KMemoryManager.h>
#include <Memory/KMemorySpace.h>
#include <Memory/KObjectManager.h>
#include <Memory/MemoryTransfer.h>
#include <KERNEL.h>

//! Size of buffer (object aligned to memory)
#define BUFFER_SIZE(ObSize, ObAlign) ((ObSize % ObAlign) ? \
			(ObSize + ObAlign - (ObSize % ObAlign)) : ObSize)

const char *nmObjectInfo = "@KObjectManager::ObjectInfo"; /* Name of ObjectInfo */
ObjectInfo tObjectInfo;
const char *nmSlab = "@KObjectManager::Slab"; /* Name of OBSLAB */
ObjectInfo tSlab;
ObjectInfo *tAVLNode;
char nmLinkedList[] = "LinkedList";
ObjectInfo *tLinkedList;

bool oballocNormaleUse = false;

void obSetupAllocator(Void)
{
	tObjectInfo.name = nmObjectInfo;
	tObjectInfo.rawSize = sizeof(ObjectInfo);
	tObjectInfo.colorScheme = 0;
	tObjectInfo.align = L1_CACHE_ALIGN;
	tObjectInfo.bufferSize = BUFFER_SIZE(sizeof(ObjectInfo),
			L1_CACHE_ALIGN);
	tObjectInfo.bufferPerSlab = (KPGSIZE - sizeof(Slab)) /
			BUFFER_SIZE(sizeof(ObjectInfo), L1_CACHE_ALIGN);
	tObjectInfo.bufferMargin = (KPGSIZE - sizeof(Slab)) %
			BUFFER_SIZE(sizeof(ObjectInfo), L1_CACHE_ALIGN);

	tSlab.name = nmSlab;
	tSlab.rawSize = sizeof(Slab);
	tSlab.colorScheme = 0;
	tSlab.align = NO_ALIGN;
	tSlab.bufferSize = sizeof(Slab);
	tSlab.bufferPerSlab = (KPGSIZE - sizeof(Slab)) /
			sizeof(Slab);
	tSlab.bufferMargin = (KPGSIZE - sizeof(Slab)) %
			sizeof(Slab);
}


void SetupPrimitiveObjects(void)
{
	if(!oballocNormaleUse)
		tLinkedList = KiCreateType(nmLinkedList, sizeof(LinkedList),
						NO_ALIGN, NULL, NULL);
}

CircularList tList; /* List of active kernel object managers */

/*!
 * Creates new slab, with all buffers linked and constructed objects. It also
 * writes the signature of the object into the page-forum. The Slab struct is
 * placed at the very end of the page to reduce TLB usage during allocation &
 * deallocation operations.
 *
 * @param metaInfo - information of the buffers to keep in the new slab
 * @param kmSleep - tells whether waiting for memory is allowed
 * @version 1.0
 * @since Circuit 2.03
 * @author Shukant Pal
 */
static Slab* createSlab(ObjectInfo *metaInfo, unsigned long kmSleep)
{
	ADDRESS pageAddress;
	Slab *newSlab;

	unsigned long slFlags = oballocNormaleUse ? (kmSleep)
			: (kmSleep | FLG_ATOMIC | FLG_NOCACHE | KF_NOINTR);
	pageAddress = KiPagesAllocate(0, ZONE_KOBJECT, slFlags);

	EnsureUsability(pageAddress, NULL, slFlags, KernelData);
	memsetf((void*) pageAddress, 0, KPGSIZE);

	newSlab = (Slab *) (pageAddress + KPGSIZE - sizeof(Slab));
	newSlab->colouringOffset = metaInfo->colorScheme;
	newSlab->bufferStack.Head = NULL;
	newSlab->freeCount = metaInfo->bufferPerSlab;

	unsigned long obPtr = pageAddress, bufferSize = metaInfo->bufferSize;
	unsigned long bufferFence = pageAddress + (KPGSIZE - sizeof(Slab)) - bufferSize;
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

	KPAGE *slabPage = (KPAGE*) KPG_AT(pageAddress);
	slabPage->HashCode = (unsigned long) metaInfo;

	return (newSlab);
}

/*!
 * Deletes a totally unused object-slab, by calling all object destructors and
 * by freeing the cached free-slab. This slab becomes the new cached free-slab
 * for the object meta-data.
 *
 * @param emptySlab - any unused slab, out of which no objects are allocated
 * @param metaInfo - information about the object & slab-caches
 * @version 1.1
 * @since Circuit 2.03
 * @author Shukant Pal
 */
static void destroySlab(Slab *emptySlab, ObjectInfo *metaInfo)
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
	EnsureFaulty(pageAddress, NULL);
	KiPagesFree(pageAddress);
}

/*!
 * This function finds a partially/fully free slab, that can be used to
 * allocate objects. It looks first into the partial list, otherwise an
 * empty slab is look for. If a empty slab is not available, one is created
 * and added to the partial list.
 *
 * @param metaInfo - Object-type info
 * @param kmSleep - To sleep, if a new slab is created
 * @return - a partially/fully free slab (linked in partial list)
 * @version 1.0
 * @since Circuit 2.03
 */
static Slab *findSlab(ObjectInfo *metaInfo, unsigned long kmSleep)
{
	if(metaInfo->partialList.count)
	{
		return (Slab *) (metaInfo->partialList.lMain);
	}
	else
	{
		Slab *emptySlab = metaInfo->emptySlab;
		if(emptySlab == NULL)
			emptySlab = createSlab(metaInfo, kmSleep);
		else
			metaInfo->emptySlab = NULL;

		AddCElement((CircularListNode*) emptySlab, CLAST, &metaInfo->partialList);
		return (emptySlab);
	}
}

/*!
 * After allocation, this function places the slab into its proper list. This
 * is required due to the probability that the slab has become full now and
 * has to be moved out.
 *
 * @param slab - the slab from which a object was unlinked (allocated)
 * @param metaInfo - meta-data for the object-type
 * @version 1.0
 * @since Circuit 2.03
 * @author Shukant Pal
 */
static void placeSlab(Slab *slab, ObjectInfo *metaInfo)
{
	if(slab->freeCount == 0)
	{
		RemoveCElement((CircularListNode *) slab, &metaInfo->partialList);
		AddCElement((CircularListNode *) slab, CFIRST, &metaInfo->fullList);
	}
}

/*!
 * After deallocation, this function checks if the slab is empty and if so
 * moves it out of the partial list and puts it into the empty-slab cache. But
 * if another empty-slab already lives there, then it first returns that to the
 * vmm.
 *
 * @param slab - the slab from which a object was linked (or freed)
 * @param metaInfo - meta-data for the object-type
 * @version 1.1
 * @since Circuit 2.03
 * @author Shukant Pal
 */
static void recheckSlab(Slab *slab, ObjectInfo *metaInfo)
{
	if(slab->freeCount == 1)
	{ // Came from full list
		RemoveCElement((CircularListNode *) slab, &metaInfo->fullList);
		AddCElement((CircularListNode *) slab, CFIRST, &metaInfo->partialList);
	}
	else if(slab->freeCount == metaInfo->bufferPerSlab)
	{
		RemoveCElement((CircularListNode *) slab, &metaInfo->partialList);

		Slab *oldEmptySlab = metaInfo->emptySlab;
		metaInfo->emptySlab = slab;
		if(oldEmptySlab != NULL)
			destroySlab(oldEmptySlab, metaInfo);
	}
}

/*!
 * Allocates an initialized object of the given type. It may sleep unless
 * kmSleep is passed as KM_NOSLEEP. It may be used in an interrupt context
 * also allowing interrupt-handlers to seamlessly use it.
 *
 * @param metaInfo - static & runtime information about the object
 * @param kmSleep - tells whether waiting for memory is allowed
 * @version 1.0
 * @since Circuit 2.03
 * @author Shukant Pal
 */
extern "C" void *KNew(ObjectInfo *metaInfo, unsigned long kmSleep)
{
	__cli
	SpinLock(&metaInfo->lock);
	Slab *freeSlab = findSlab(metaInfo, kmSleep);
	void *object = NULL;

	if(freeSlab != NULL)
	{
		void *freeObject = PopElement(&freeSlab->bufferStack);
		--(freeSlab->freeCount);
		placeSlab(freeSlab, metaInfo);
		object = freeObject;
	}
	
	SpinUnlock(&metaInfo->lock);

	if(oballocNormaleUse)
		__sti
	return (object);
}

/*!
 * Simply deallocates the object which means the caller should ensure no
 * pointers refer to it now. This is callable from an interrupt context.
 *
 * @param object - ptr to allocated object
 * @param metaInfo - runtime information about the object
 * @author Shukant Pal
 */
extern "C" void KDelete(void *object, ObjectInfo *metaInfo)
{
	__cli
	SpinLock(&metaInfo->lock);

	Slab *slab = (Slab *) (((unsigned long) object & ~(KPGSIZE - 1)) + (KPGSIZE - sizeof(Slab)));
	PushElement((STACK_ELEMENT *) object, &slab->bufferStack);
	++(slab->freeCount);
	recheckSlab(slab, metaInfo);

	SpinUnlock(&metaInfo->lock);
	if(oballocNormaleUse)
		__sti
}

/*!
 * Constructs the meta-info for an object type that can be later used for
 * allocating & deallocating objects of given size, alignment, and initial
 * state.
 *
 * @param name - pointer to memory where the name is permanently stored. This
 * 		 is usually static memory -
 * 			const char *declareName = "ObjectType";
 *
 * @param[in] size - byte-size of the object
 * @param[in] align - alignment constraints for the object
 * @param[in] ctor- a constructor for the object (C++ linkage)
 * @param[in] dtor - a destructor for the object (C++ linkage)
 * @version 1.1
 * @since Circuit 2.03
 * @author Shukant Pal
 */
decl_c ObjectInfo *KiCreateType(const char *tName, unsigned long tSize,
				unsigned long tAlign, void (*ctor) (void *),
				void (*dtor) (void *))
{
	unsigned long flgs = (oballocNormaleUse) ? KM_SLEEP : FLG_ATOMIC | FLG_NOCACHE | KF_NOINTR;
	ObjectInfo *typeInfo = (ObjectInfo*) KNew(&tObjectInfo, flgs);

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
	typeInfo->partialList.lMain = NULL;
	typeInfo->partialList.count = 0;
	typeInfo->fullList.lMain = NULL;
	typeInfo->fullList.count= 0;
	AddCElement((CircularListNode*) typeInfo, CLAST, &tList);

	return (typeInfo);
}

/*!
 * Removes the object & its allocator, if and only if no objects are currently
 * outside the allocator.
 *
 * @param obj - meta-data for the object
 * @return -
 * FALSE, if any objects are in circulation and its wasn't freed; TRUE, if it
 * is now freed, and further objects SHOULD NOT be allocated.
 * @author Shukant Pal
 */
decl_c unsigned long KiDestroyType(ObjectInfo *typeInfo)
{
	if(typeInfo->partialList.count != 0 ||
			typeInfo->fullList.count != 0)
	{
		return (false);
	}
	else
	{
		RemoveCElement((CircularListNode*) typeInfo, &tList);

		Slab *emptySlab = (Slab*) typeInfo->emptySlab;
		if(emptySlab != NULL)
			destroySlab(emptySlab, typeInfo);

		return (true);
	}
}
