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

#include "KFrameManager.h"
#include "Pager.h"
#include <Synch/Spinlock.h>
#include <Util/AVLTree.h>
#include <Util/CircularList.h>
#include <TYPE.h>

#define KM_SLEEP FLG_ATOMIC
#define KM_NOSLEEP FLG_NONE

/*
 * Default-alignment is 4 for 32-bit systems/ 8 for 64-bit systems
 */
#define NO_ALIGN 4

#ifdef x86
	#define L1_CACHE_ALIGN 64
#endif

struct Slab
{
	LinkedListNode liLinker;
	STACK bufferStack;
	unsigned long colouringOffset;
	unsigned long freeCount;
};

struct ObjectInfo
{
	LinkedListNode liLinker;// Used for listing this, for pressurizing vmm
	const char *name;// Name of the object, used for debugging
	unsigned long rawSize;// Raw size of the object
	unsigned long colorScheme;// Current coloring scheme for the incoming slabs
	unsigned long align;// Alignment constraints for the object
	unsigned long bufferSize;// Size of the aligned-object buffer
	unsigned long bufferPerSlab;// No. of buffers that can fit into one slab
	unsigned long bufferMargin;// Leftover-space in a slab, after buffers
	void (*ctor) (Void *);// Object-constructor (not for C++, yet)
	void (*dtor) (Void *);// Object-destructor (not for C++, yet)
	unsigned long allocatedObjects;// No. of object that are in usage
	unsigned long freeCount;// No. of objects that are free in current pool
	unsigned long callCount;// No. of operation done on this type (not used)
	Slab *emptySlab;// Cache for a empty slab
	CircularList partialList;// List of partial slabs
	CircularList fullList;// List of complete slabs
	Spinlock lock;// Serialization lock
};

extern "C" ObjectInfo *KiCreateType(const char *tName, unsigned long tSize,
			unsigned long tAlign, void (*tConstruct) (Void *),
				void (*tDestruct) (Void *));
extern "C" void *KNew(ObjectInfo *typeInfo, unsigned long kmSleep);
extern "C" void KDelete(void *object, ObjectInfo *objectInfo);
extern "C" unsigned long KiDestroyType(ObjectInfo *);

void obSetupAllocator(Void) kxhide;
void SetupPrimitiveObjects(void) kxhide;

#endif/* Memory/KObjectManager.h */
