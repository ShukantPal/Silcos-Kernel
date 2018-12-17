///
/// @file KObjectManager.h
/// -------------------------------------------------------------------
/// This program is free software: you can redistribute it and/or modify
/// it under the terms of the GNU General Public License as published by
/// the Free Software Foundation, either version 3 of the License, or
/// (at your option) any later version.
///
/// This program is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
/// GNU General Public License for more details.
///
/// You should have received a copy of the GNU General Public License
/// along with this program.  If not, see <http://www.gnu.org/licenses/>
///
/// Copyright (C) 2017 - Shukant Pal
///
#ifndef __MEMORY_KOBJECT_MANAGER_H__
#define __MEMORY_KOBJECT_MANAGER_H__

#include "KFrameManager.h"
#include "Pager.h"
#include <Synch/Spinlock.h>
#include <TYPE.h>
#include <Utils/AVLTree.hpp>
#include <Utils/LinkedList.h>

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

///
/// Holds the meta-info for any object-type. Kernel software uses the
/// KiCreateType() & KiDestroyType() functions to create & delete these
/// types of objects. On holding an ObjectInfo type, kernel software can
/// allocate & deallocate specific types of objects.
///
/// @version 1.0
/// @since Circuit 2.03
/// @author Shukant Pal
///
struct ObjectInfo
{
	LinkedListNode liLinker;
	const char *name;
	unsigned long rawSize;
	unsigned long colorScheme;
	unsigned long align;
	unsigned long bufferSize;
	unsigned long bufferPerSlab;
	unsigned long bufferMargin;
	void (*ctor) (Void *);
	void (*dtor) (Void *);
	unsigned long allocatedObjects;
	unsigned long freeCount;
	unsigned long callCount;
	Slab *emptySlab;
	CircularList partialList;
	CircularList fullList;
	Spinlock lock;

	ObjectInfo() // @suppress("Class members should be properly initialized")
	{

	}
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
