/* @file Heap.cpp
 *
 * Implements all functions required to access system memory for small to
 * large sized objects. To save memory, you should allocate objects from
 * here for scarcely used types of objects.
 * -------------------------------------------------------------------
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 *
 * Copyright (C) 2017 - Shukant Pal
 */
#include "../Interface/Heap.hpp"

#include <Memory/KObjectManager.h>
#include <Memory/Pager.h>
#include <Memory/KMemoryManager.h>
#include <Memory/MemoryTransfer.h>
#include <KERNEL.h>

using namespace Heap;

#ifdef ARCH_32
	#define minHeapSz 32
	#define minHeapOff 5
	#define maxHeapSz 1024
	#define maxHeapOff 10
#else
	#define minHeapSz 64
	#define minHeapOff 6
	#define maxHeapSz 2048
	#define maxHeapOff 11
#endif

const char *heapEngineI = "heapEngineI";//< 8 DWORDs
const char *heapEngineII = "heapEngineII";//< 16 DWORDs
const char *heapEngineIII = "heapEngineIII";//< 32 DWORDs
const char *heapEngineIV = "heapEngineIV";//< 64 DWORDs
const char *heapEngineV = "heapEngineV";//< 128 DWORDs
const char *heapEngineVI = "heapEngineVI";//< 256 DWORDs

const char *dbHeapEngineNms[6] = {
		heapEngineI, heapEngineII, heapEngineIII,
		heapEngineIV, heapEngineV, heapEngineVI
};//< array of heap-engine names

ObjectInfo* heapEngines[6];// slab-allocators for each allocation class

/*
 * Allocates the specified amount of memory if available directly from kernel
 * memory, using underlying slab allocator support. The actual amount of memory
 * returned may not be exact, be it is guaranteed to be equal to or greater
 * than the requested bytes.
 *
 * Since Silcos 3.02, support has been given for allocations above 256
 * DWORDs by returning whole blocks of pages directly from the vmm, mapping
 * them to physical kernel-memory.
 *
 * To keep track of usage and prevent dangling pointers, kmalloc() provides a
 * method to hold the no. of users of heap memory. The memory cannot be freed
 * (unless forced) until the no. of users drops to zero.
 *
 * Note that kmalloc is more of a keyword than a function. Use it whenever
 * you allocate objects in small numbers.
 *
 * @param[in] memSize - no. of bytes required in kernel-memory
 * @param[in] initialUsers - no. of initial users that refer to this memory
 * @author Shukant Pal
 * @see KNew, KiCreateType <KObjectManager.h>
 */
void* kmalloc(unsigned int memSize, unsigned int initialUsers)
{
	memSize += sizeof(BlockContainer);

	if(memSize < minHeapSz)
	{
		memSize = 32;
	}
	else if(memSize < maxHeapSz)
	{
		memSize = NextPowerOf2(memSize);
	}
	else
	{
		/// Here, we are allocating directly from the vmm, as the
		/// requested amount is greater than maxHeapSz.
		if(memSize < KPGSIZE)
			memSize = KPGSIZE;

		unsigned long pagesReq = NextPowerOf2(memSize) >> KPGOFFSET;
		pagesReq = HighestBitSet(pagesReq);

		unsigned long memAddr = KiPagesAllocate(pagesReq,
							ZONE_KOBJECT, ATOMIC);

		EnsureUsability(memAddr, null, ATOMIC, KernelData);

		BlockContainer *mBlock = (BlockContainer*) memAddr;
		mBlock->magicNo = HEAP_MAGIC;
		mBlock->refCount = initialUsers;
		mBlock->blockOrder = pagesReq + KPGOFFSET;
		return ((void*)(mBlock + 1));
	}

	unsigned int allocOrder = HighestBitSet(memSize);
	BlockContainer *memGiven = (BlockContainer*)
				KNew(heapEngines[allocOrder - 5], KM_SLEEP);
	memGiven->magicNo = HEAP_MAGIC;
	memGiven->refCount = initialUsers;
	memGiven->blockOrder= allocOrder;
	return ((void*)(memGiven + 1));
}

/*
 * Takes back kmalloc'ed memory if found valid, unless the no. of users left
 * is greater than one and forceDelete is not required. Corrupted memory cannot
 * be taken back here.
 *
 * If the memory was taken from vmm, it unmaps it and returns it to the
 * vmm. This occurs when the memory size was greater than 256 DWORDs.
 *
 * @param[in] memGiven - pointer to kmalloc'ed memory
 * @param[in] forceDelete - client may force kfree() to take back memory even
 * 				if the no. of users is still greater than one
 * @return - whether the memory was freed succesfully
 * @author Shukant Pal
 */
bool kfree(void* memGiven, bool forceDelete)
{
	BlockContainer *memBlock = BlockFor(memGiven);

	if(memBlock->magicNo != HEAP_MAGIC)
	{
		return (false);
	}
	else if(memBlock->blockOrder < KPGOFFSET)
	{
		--(memBlock->refCount);
		if(memBlock->refCount == 0 || forceDelete)
			KDelete((void*) memBlock,
					heapEngines[memBlock->blockOrder - 5]);
		return (true);
	}
	else
	{
		unsigned long vmBlockOrder = memBlock->blockOrder - KPGOFFSET;
		MMFRAME *paddr = GetFrames((unsigned long) memGiven,
							vmBlockOrder, null);
		KeFrameFree((PADDRESS) paddr);
		EnsureFaulty((unsigned long) memGiven, null);
		KiPagesFree((unsigned long) memGiven);
		return (true);
	}
}

/*
 * Re-allocates memory to new size. If the current buffer couldn't be extended
 * a new location in memory is returned. User should copy his data to that
 * buffer if given -
 *
 *			if(old_buffer != new_buffer)
 *				user_mycopy_func(...);
 *
 * @param[in] - pointer to original kmalloc'ed memory
 * @param[new_size] - new size required for data
 * @author Shukant Pal
 */
void* kralloc(void *heap_mem, unsigned long new_size)
{
	BlockContainer *heap_block = BlockFor(heap_mem);
	unsigned long org_size = 1 << heap_block->blockOrder;

	if(org_size >= new_size)
	{
		return (heap_mem);
	}
	else
	{
		return (kmalloc(new_size));
	}
}

/*
 * Used when "refreshing" memory while expanding its range. Returns memory
 * with the new size and filled with "zeros". It is a compresed combination of
 * kralloc & kcalloc.
 *
 * @param[in] heap_mem - kmalloc'ed memory to be zeroed & expanded
 * @param[in] new_size - new size required for data
 * @author Shukant Pal
 * @see kcalloc, kralloc
 */
void *krcalloc(void *heap_mem, unsigned long new_size)
{
	BlockContainer *heap_block = BlockFor(heap_mem);
	unsigned long org_size = (1 << heap_block->blockOrder);

	if(org_size >= new_size)
	{
		if(org_size> new_size)
			memsetf((void*)((char*) heap_mem + org_size), 0,
					new_size - org_size);
		return (heap_mem);
	}
	else
		return (kcalloc(new_size));
}

/*
 * Invoked by __init() during module initialization. Creates the types required
 * for each allocation class under 256 DWORDs.
 *
 * Due to the nature of __init()ing each module as a library, other kernel
 * modules should not invoke heap-access functions until they are assured that
 * __init() has completed (in ModuleFramework). If __init() functions in other
 * modules require the heap, then work must be done so that each module can
 * invoke __init() (in ModuleFramework) while it runs only the first time by a
 * "bool" switch.
 *
 * @author Shukant Pal
 */
decl_c void __initHeap()
{
	unsigned int engIdx = 0;
	while(engIdx < 6)
	{
		heapEngines[engIdx] = KiCreateType(dbHeapEngineNms[engIdx],
				32 << engIdx, NO_ALIGN, NULL, NULL);
		++(engIdx);
	}
}
