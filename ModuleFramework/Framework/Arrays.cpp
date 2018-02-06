/**
 * File: Arrays.cxx
 * Copyright (C) 2017 - Shukant Pal
 */
#include "../../Interface/Utils/Arrays.hpp"

#include "../../Interface/Heap.hpp"
#include "../../Interface/Utils/Memory.h"

/*
 * Copies the original memory into the destination buffer with the given size,
 * so they contain the same contents. No array-bound checking is done so the
 * caller must ensure memory references within the range of the array
 * specified are valid.
 *
 * @param *org - Source memory
 * @param *dst - Destination memory-buffer
 * @param copySize - Amount of memory to copy in bytes
 * @author Shukant Pal
 */
void Arrays::copy(const void *org, void *dst, unsigned int copySize)
{
	const byte *orgPtr = (const byte*) org;
	byte *dstPtr = (byte*) dst;

	while(copySize--)
		*(dstPtr++) = *(orgPtr++);
}

/*
 * Copies specified memory into a kmalloc memory with the given new length, so
 * that the two arrays have the same contents. No array-bound checking is done
 * as the caller must ensure memory references within the range of the array
 * specified is valid.
 *
 * @param original - Memory from which the copy is to done
 * @param newLength - Length of the new array
 * @return - pointer to the newly copied array
 * @author Shukant Pal
 */
void* Arrays::copyOf(const void *original, unsigned int newLength)
{
	void *newCopy = kmalloc(newLength);
	copy(original, newCopy, newLength);
	return (newCopy);
}
