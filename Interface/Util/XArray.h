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
	VOID **ArrayData;
	USHORT Size;
	USHORT MSize;
	USHORT GSize; // Alloc Size
} XArray;

ULONG XAddData(
	VOID *X,
	XArray *Array
);

VOID XRemoveData(
	ULONG Index,
	XArray *Array
);

#endif /* Util/XArray.h */
