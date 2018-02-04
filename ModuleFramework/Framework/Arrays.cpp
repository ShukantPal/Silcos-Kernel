/**
 * File: Arrays.cxx
 * Copyright (C) 2017 - Shukant Pal
 */
#include "../../Interface/Util/Arrays.hpp"

#include <Heap.hxx>
#include <Util/Memory.h>

/**
 * Function: Arrays::copy
 *
 * Summary:
 * Copies the original memory into the destination buffer with the given size,
 * so they contain the same contents. No array-bound checking is done so the
 * caller must ensure memory references within the range of the array
 * specified are valid.
 *
 * Args:
 * const void *org - Source memory
 * void *dst - Destination memory-buffer
 * unsigned int copySize - Amount of memory to copy in bytes
 *
 * Author: Shukant Pal
 */
void Arrays::copy(const void *org, void *dst, unsigned int copySize)
{
	const byte *orgPtr = (const byte*) org;
	byte *dstPtr = (byte*) dst;
	while(copySize-- > 0)
	{
		*dstPtr = *orgPtr;
	}
}

/**
 * Function: Arrays:copyOf
 *
 * Summary:
 * Copies specified memory into a kmalloc memory with the given new length, so
 * that the two arrays have the same contents. No array-bound checking is done
 * as the caller must ensure memory references within the range of the array
 * specified is valid.
 *
 * Args:
 * void* original - Memory from which the copy is to done
 * unsigned int newLength - Length of the new array
 *
 * Returns:
 * Pointer to the newly copied array
 *
 * Author: Shukant Pal
 */
void* Arrays::copyOf(const void *original, unsigned int newLength)
{
	void *newCopy = kmalloc(newLength);
	copy(original, newCopy, newLength);
	return (newCopy);
}
