/**
 * Copyright (C) 2017 - Shukant Pal
 *
 * The KObjectManager is responsible for the allocation and deallocation of kernel objects in
 * lue of heavily-used data structures. These data structures are allocated in slabs (pages) all
 * together. Then, one by one they are given to the kernel-based client. It also looks into
 * slab-colouring, object-alignment, object-construction and destruction, slab caching, and
 * per-type statistics.
 *
 * OBJECT_INFO:
 * Every object-type has its own metadata consists of list of partially-free slabs, fully-used slabs
 * and a cache for a fully free slab. Objects are allocated from partial slab (or if none are there, then
 * a free slab). These are allocated from inbuilt object-types in the KObjectManager.
 *
 * SLAB:
 * A SLAB is just a structured page which contains objects of one type. All these objects are in
 * the constructed state, and are aligned properly. By using the SLAB, the allocator saves time in
 * constructing and destructing the objects. The SLAB metadata, is stored at the very end of the
 * page.
 *
 * The SLAB are created by mapping a free-kernel page to a pageframe. They are cached in
 * (CacheRegister) per-CPU lists. It allows better TLB usage. The allocation and deallocation
 * operations required only two TLB entries - the SLAB, and the Object-type info.
 */
#ifndef __MEMORY_KOBJECT_MANAGER_H__
#define __MEMORY_KOBJECT_MANAGER_H__

#include "Pager.h"
#include <Synch/Spinlock.h>
#include <Util/AVLTree.h>
#include <Util/CircularList.h>
#include <TYPE.h>

#define KM_SLEEP 1
#define KM_NOSLEEP 0

/*
 * Default-alignment is 4 for 32-bit systems/ 8 for 64-bit systems
 */
#define NO_ALIGN 4

#ifdef x86
	#define L1_CACHE_ALIGN 64
#endif

/**
 * OBSLAB - 
 *
 * Summary:
 * This type is used to store data about a slab and is located at the end of the page. It
 * is internal to the KObjectManager.
 *
 * Fields:
 * LiLinker - Used for participating in lists
 * ObStack - Stack of free, constructed objects
 * ObColor - Object-colour for this slab
 * ObCount - No. of objects in this slab (free ones)
 * TypeInfo - Address of the Object-type info
 *
 * @Version 1
 * @Since Circuit 2.03
 */
typedef
struct Slab {
	LIST_ELEMENT LiLinker;
	STACK ObStack;
	ULONG ObColor;
	ULONG ObCount;
} OBSLAB;

/**
 * OBINFO - 
 *
 * Summary:
 * This type contains all data for maintaining a independent (kernel) object. It stores its
 * name, size, alignment, colour, constructor, destructor and lots of other (internal) data.
 *
 * Fields:
 * LiLinker - Used for participating in list of active (kernel) objects
 * ObName - Name of the object
 * ObSize - Size of the object
 * ObColor - Last color of the object
 * ObAlign - Alignment-preferred for the object
 * BufferSize - Aligned size for the object
 * BufferPerSlab - No. of buffers fitting in one slab
 * BufferMargin - Margin left after fitting all buffers
 * ObConstruct - Constructor for the object (NULL, if none)
 * ObDestruct - Destructor for the object (NULL, if none)
 * ObAllocated - No. of object allocated
 * ObFree - No. of free objects
 * CallCount - No. of operations on the object-type
 * EmptySlab - Cache for a empty slab
 * PartialList - List of partial slabs
 * FullList - List of full slabs
 *
 * @Version 1
 * @Since Circuit 2.03
 */
typedef
struct ObjectInfo {
	LIST_ELEMENT LiLinker;
	CHAR *ObName;
	ULONG ObSize;
	ULONG ObColor;
	ULONG ObAlign;
	ULONG BufferSize;
	ULONG BufferPerSlab;
	ULONG BufferMargin;
	void (*ObConstruct) (Void *);
	void (*ObDestruct) (Void *);
	ULONG ObAllocated;
	ULONG ObFree;
	ULONG CallCount;
	struct Slab *EmptySlab;
	CLIST PartialList;
	CLIST FullList;
	SPIN_LOCK ObLock;
} OBINFO;

/**
 * Function: KiCreateType()
 *
 * Summary: This function creates a object allocator, for the given properties. Duplicate
 * types (with the same name) can exist and are not check for co-existence.
 *
 * Args:
 * tName - Object's name
 * tSize - Object's size
 * tAlign - Object's alignment
 * tConstruct - Constructor
 * tDestruct - Destructor
 *
 * Returns: The object allocator, allocated from the root allocator.
 *
 * @Version 1
 * @Since Circuit 2.03
 */
struct ObjectInfo *KiCreateType(CHAR *tName,
								ULONG tSize,
								ULONG tAlign,
								void (*tConstruct) (Void *),
								void (*tDestruct) (Void *)
);

VOID *KNew(struct ObjectInfo *typeInfo, ULONG kmSleep);
VOID KDelete(Void *object, struct ObjectInfo *objectInfo);
ULONG KiDestroyType(struct ObjectInfo *);

#define SETUP_OBJECT(typeName) KiCreateType("typeName", sizeof(typeName), sizeof(ULONG), NULL, NULL)

void obSetupAllocator(Void);
void SetupPrimitiveObjects(void);

#endif/* Memory/KObjectManager.h */
