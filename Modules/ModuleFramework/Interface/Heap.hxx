/**
 * File: Heap.hxx
 *
 * Summary:
 * Microkernel services don't include a dynamic-size heap for reducing binary
 * size. The module framework implements a heap-tree based dynamic memory-heap
 * with localization features.
 * 
 * Functions:
 *
 * Origin:
 *
 * Copyright (C) 2017 - Shukant Pal
 */
#ifndef __MODULES_MODULEFRAMEWORK_INTERFACE_HEAP_HXX__
#define __MODULES_MODULEFRAMEWORK_INTERFACE_HEAP_HXX__

#include <Util/Memory.h>
#include <KERNEL.h>

namespace Heap
{

/**
 * Struct: BlockContainer
 *
 * Summary:
 * Block-container represents a allocation-case and contains metadata for a memory
 * block.
 *
 * Origin:
 * It is used for getting proper information of a block, without the client recording
 * it by the heap-allocator.
 *
 * Author: Shukant Pal
 */
struct BlockContainer
{
	unsigned int magicNo;/* A magic-field which should contain HEAP_MAGIC */
	unsigned int blockOrder;/* Power-of-two for the block size */
	unsigned int referenceCount;/* Reference count for the memory-block */
};

/*
 * Allows checking for header-corruption by the allocator & invalidating wrong
 * freeing of memory.
 */
#define HEAP_MAGIC 0x2bc929de

/*
 * Get the block-container struct for any memory-location. It may be invalid &
 * this is checked by validating the magicNo field.
 */
#define BlockFor(memAddr) ((BlockContainer*) ((ULONG) memAddr - 12))

}

Void* kmalloc(unsigned int memSize, unsigned int initalUsers = 1);
bool kfree(Void *memGiven, bool forceDelete = false);
void* kralloc(void *kmal_mem, unsigned long sasur_ka_size);
void* krcalloc(void *kmal_mem, unsigned long sasur_ka_size);

/**
 * Function: kuse
 *
 * Summary:
 * Updates a reference-count for a kmalloc-memory so that the heap can know not
 * to delete the object-in-memory on the next kfree.
 *
 * Args:
 * void *memory - Pointer to the start of allocated region (not in middle)
 *
 * Changes:
 * Updates the reference count of the memory.
 *
 * Author: Shukant Pal
 */
static inline void kuse(const void *memory)
{
	Heap::BlockContainer *mdesc = (Heap::BlockContainer*) ((char*) memory - sizeof(Heap::BlockContainer));
	++(mdesc->referenceCount);
}

static inline void *kcalloc(unsigned int memSize)
{
	void *mem = kmalloc(memSize);
	memsetf(mem, 0, memSize);
	return (mem);
}

void __initHeap();

#endif/* Module/ModuleFramework/Interface/Heap.hxx */
