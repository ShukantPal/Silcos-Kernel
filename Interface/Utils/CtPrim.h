/**
 * Copyright (C) - Shukant Pal
 */

#ifndef UTIL_CT_PRIM_H
#define UTIL_CT_PRIM_H

#include <Types.h>

/* mem-management functions (primitive & independent) */
void memset(void *Pointer, unsigned char Value, SIZE Size);

static inline void memsetf(Void *bPointer, SIZE lgsValue, SIZE bSize)
{
	unsigned long *lgBuffer = (unsigned long*) bPointer;
	bSize &= ~(3);
	unsigned long *lgPointer = (unsigned long *) ((UBYTE *) bPointer + bSize);

	while((unsigned long) lgPointer > (unsigned long) lgBuffer)
	{
		--(lgPointer);
		*lgPointer = lgsValue;
	}
}
static inline void memcpy(const Void *Obj1, Void *Obj2, SIZE ObjSize)
{
	char *O1 = (char *) Obj1;
	char *O2 = (char *) Obj2;

	while(ObjSize)
	{
		--ObjSize;
		O2[ObjSize] = O1[ObjSize];
	}
}

static inline void memcpyf(const Void *bPointerI, Void *bPointerII, SIZE bufferSize)
{
	if(bufferSize)
	{
		unsigned long *cPointerSource = (unsigned long *) ((UBYTE *) bPointerI + bufferSize);
		unsigned long *cPointerDest = (unsigned long *) ((UBYTE *) bPointerII + bufferSize);
		bufferSize /= sizeof(SIZE);

		while(bufferSize)
		{
			--(bufferSize);
			--(cPointerSource);
			--(cPointerDest);
			*cPointerDest = *cPointerSource;
		}
	}
}


bool memcmp(const void *from, const void *to, SIZE cpSize);

static inline bool strcmp(const char *s1, const char *s2)
{
	while(TRUE)
	{
		if(*s1 != *s2)
		{
			return (false);
		}
		else if(*s1 == '\0')
			return (true);
		++s1;
		++s2;
	}
}

static inline bool strcmpn(const char *str0, const char *str1, const long n)
{
	long idx = 0;
	while(idx < n)
	{
		if(*str0 != *str1)
		{
			return (false);
		}
		else if(*str1 == '\0')
		{
			break;
		}

		++(str0);
		++(str1);
		++(idx);
	}

	return (true);
}


void strcpy(const char *str0, char *str1);

static inline unsigned long strlen(const char *str0)
{
	unsigned long curlen = 0;
	while(*str0)
	{
		++(curlen); ++(str0);
	}
	return (curlen);
}

#endif/* Util/CtPrim.h */
