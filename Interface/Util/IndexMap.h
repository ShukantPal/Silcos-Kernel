/**
 * Copyright (C) 2017 - Shukant Pal
 */

#ifndef UTIL_INDEX_MAP_H
#define UTIL_INDEX_MAP_H

#include "XArray.h"
#include <Types.h>

U32 GtIndex(
	XArray *IndexMap
);

static inline
VOID StIndex(U32 BitIndex, XArray *X)
{
	*((U32*) &X -> ArrayData[BitIndex / 32]) &= ~(1 << (BitIndex % 32));
}

#endif /* IndexMap.h */
