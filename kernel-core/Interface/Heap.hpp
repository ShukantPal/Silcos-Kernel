///
/// @file Heap.hpp
/// @module KernelHost
///
/// The heap provides an allocator-interface for blocks of varying sizes
/// without having to "create"/"destroy" any object-type. This is particularly
/// useful for objects that are less often allocated/freed and for expandable
/// objects which don't have fixed sizes.
///
/// Along with allocation/deallocation utilities, the heap (in the future will)
/// provides debugging support, by utilizing the extra memory given to the
/// client.
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

#ifndef KERNHOST_HEAP_HPP__
#define KERNHOST_HEAP_HPP__

#include "Utils/Memory.h"

namespace Heap
{

///
/// Holds the data about an allocated block in heap-memory. It is used
/// for reference-counting, holding boundary-information, etc.
///
/// @version 1.0
/// @since Silcos 2.05
/// @author Shukant Pal
///
struct BlockContainer
{
	unsigned int magicNo;/* A magic-field which should contain HEAP_MAGIC */
	unsigned int blockOrder;/* Power-of-two for the block size */
	unsigned int refCount;/* Reference count for the memory-block */
};

//! BlockContainer::magicNo should contain this constant. If it doesn't, this
//! means that memory-access has overflowed and unfortunately has corrupted the
//! heap.
#define HEAP_MAGIC 0x2bc929de

//! Alias for getting the BlockContainer struct for allocated heap-memory
#define BlockFor(memAddr)((BlockContainer*) \
		((unsigned long) memAddr - sizeof(BlockContainer)))

}

void* kmalloc(unsigned int memSize, unsigned int initalUsers = 1);
bool kfree(Void *memGiven, bool forceDelete = false);
void* kralloc(void *kmal_mem, unsigned long sasur_ka_size);
void* krcalloc(void *kmal_mem, unsigned long sasur_ka_size);

///
/// Increments the reference-count for a block of heap-memory. Useful while
/// passing around objects & strings, particularly.
///
/// @param memory - kmalloc'ed memory
///
static inline void kuse(const void *memory)
{
	Heap::BlockContainer *mdesc = (Heap::BlockContainer*) ((char*) memory - sizeof(Heap::BlockContainer));
	++(mdesc->refCount);
}

///
/// Allocates memory and sets it fully with zeros.
///
/// @param memSize - amount, in bytes, of memory to be allocated
///
static inline void *kcalloc(unsigned int memSize)
{
	void *mem = kmalloc(memSize);
	memsetf(mem, 0, memSize);
	return (mem);
}

decl_c void __initHeap();

#endif/* Module/ModuleFramework/Interface/Heap.hpp */
