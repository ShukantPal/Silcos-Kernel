/**
 * Copyright (C) 2017 - Shukant Pal
 *
 * Used in default grain-allocator.
 */

#ifndef UTIL_CONST_ARRAY_H
#define UTIL_CONST_ARRAY_H

#include <Types.h>

/* 64-bit integer in a struct */
typedef struct {
	UINT32_T First;
	UINT32_T Second;
} DoubleInt;

/* Constant or non-heap managed arrays */
typedef struct {
	DoubleInt *ArrayData;
	UINT32_T Size;
} ConstArray;

/* Not used anymore. Can be removed */
typedef struct {
	void ** ArrayField;
	UINT32_T Size;
	UINT32_T MaxSize;
} Array;

/* Add a double int */
void Add(DoubleInt, ConstArray*);

/* Remove double int at a index */
void Remove(UINT32_T, ConstArray*);

#endif /* Util/ConstArray.h */
