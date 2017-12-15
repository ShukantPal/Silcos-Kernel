/**
 * Copyright (C) 2017 - Shukant Pal
 */

#ifndef UTIL_X_ARRAY_H
#define UTIL_X_ARRAY_H

#include <Types.h>

/**
 * XArray (a.k.a struct GrowableArray) - 
 *
 * Heap-allocated array to pointers for global
 * objects. Typically used to enumerate a small number
 * of things, like PoC.
 *
 */
typedef
struct GrowableArray 
{
	void **ArrayData;
	unsigned short Size;
	unsigned short MSize;
	unsigned short GSize; // Alloc Size
} XArray;

unsigned long XAddData(
	void *X,
	XArray *Array
);

void XRemoveData(
	unsigned long Index,
	XArray *Array
);

#endif /* Util/XArray.h */
