/**
 * File: Heap.cxx
 *
 * Summary:
 * Dynamic-size heaps are implemented ontop of the slab-allocator, and blocks of
 * 16-byte multiple sizes are allocated from slabs. Maximum size of memory allocated
 * is 512-bytes.
 * 
 * Functions:
 * kmalloc - Implements heap-allocation & uses object-poisoning for padded blocks
 * kfree - Frees heap-allocated memory & and checks for bit-wis corruption at
 *         padded locations
 *
 * Origin:
 * Creating a seperate slab allocator for every allocation-type is not good, thus,
 * a non-fixed-size allocator is required.
 *
 * Copyright (C) 2017 - Shukant Pal
 */
#include <Memory/KObjectManager.h>
#include <Heap.hxx>
#include <KERNEL.h>

using namespace Heap;

/*
 * Only following heap sizes are allowed for the current build -
 * 32, 64, 128, 256, and 512 bytes
 *
 * NOTE- these include the chunk header
 */
ObjectInfo* heapEngines[5];

/**
 * Function: kmalloc
 *
 * Summary:
 * Allocates a memory-block with a 16-byte alignment guaranteed for
 * a size, and padding may be done! Maximum size for allocation is
 * 512 bytes.
 *
 * Args:
 * unsigned int memSize - Size of required memory
 * unsigned int initialUsers (default=1) - Initial reference count for the block
 *
 * NOTE:
 * kmalloc is considered a language feature & is a lowercase-keyword.
 *
 * Author: Shukant Pal
 */
void* kmalloc(unsigned int memSize, unsigned int initialUsers)
{
	memSize += sizeof(BlockContainer);

	if(memSize < 32)
		memSize = 32;
	else if(memSize < 512)
		memSize = NextPowerOf2(memSize);
	else
		return (NULL);

	unsigned int totalOrder = HighestBitSet(memSize);

	ObjectInfo *requiredAllocator = heapEngines[totalOrder - 5];
	BlockContainer *memGiven = (BlockContainer*) KNew(requiredAllocator, KM_SLEEP);

	memGiven->magicNo = HEAP_MAGIC;
	memGiven->referenceCount = initialUsers;
	memGiven->blockOrder= totalOrder;

	return ((void*)memGiven + sizeof(BlockContainer));
}

/**
 * Function: kfree
 *
 * Summary:
 * Validates the pointer given & frees the memory is reference count becomes 0
 * or forceDelete is set to true.
 *
 * Args:
 * void *memGiven - Memory given to the client by kmalloc
 * unsigned int forceDelete (default=false) - Whether to delete memory, even if refcount>0
 *
 * Author: Shukant Pal
 */
bool kfree(void* memGiven, bool forceDelete)
{
	BlockContainer *memBlock = BlockFor(memGiven);

	if(memBlock->magicNo != HEAP_MAGIC)
		return (false);
	else {
		--(memBlock->referenceCount);

		if(memBlock->referenceCount == 0 || forceDelete){
			ObjectInfo *requiredAllocator = heapEngines[memBlock->blockOrder - 5];
			KDelete((void*) memBlock, requiredAllocator);
		}

		return (true);
	}
}

void* kralloc(void *heap_mem, unsigned long new_size)
{
	BlockContainer *heap_block = BlockFor(heap_mem);
	unsigned long org_size = 1 << heap_block->blockOrder;

	if(org_size >= new_size)
		return (heap_mem);
	else {
		return kmalloc(new_size);
	}
}

void *krcalloc(void *heap_mem, unsigned long new_size)
{
	BlockContainer *heap_block = BlockFor(heap_mem);
	unsigned long org_size = (1 << heap_block->blockOrder);

	if(org_size >= new_size){
		if(org_size> new_size)
			memsetf(heap_mem + org_size, 0, new_size - org_size);
		return (heap_mem);
	} else
		return (kcalloc(new_size));
}

char engineName[] = "heap_sv";

/**
 * Function: __initHeap
 * Attributes: init, hidden
 *
 * Summary:
 * Initializes heap allocators
 *
 * Author: Shukant Pal
 */
void __initHeap()
{
	unsigned int engineIndex = 0;
	while(engineIndex < 5){
		heapEngines[engineIndex] = KiCreateType(engineName, 32 << engineIndex, NO_ALIGN, NULL, NULL);

		++(engineIndex);
	}
}
