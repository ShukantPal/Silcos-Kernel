/**
 * Copyright (C) 2017 - Shukant Pal
 */

#include <Debugging.h>
#include <Types.h>
#include <Util/CtPrim.h>

Void ASSERT(bool c, char *String)
{
	if(!c)
	{
		DbgLine(String);
		DbgLine("Circuit Fused ! Platform Shutting Down Due To Undetectable Condition");
		asm volatile("cli");
		while(TRUE) asm volatile("hlt");
	}
}

Void memset(Void *bPointer, UBYTE btValue, SIZE bSize)
{
	UBYTE *btBuffer = (UBYTE *) bPointer;/* Transfer bytes */
	UBYTE *btPointer = (UBYTE *) bPointer + bSize;/* Transfer serially */
	while((unsigned long) btPointer > (unsigned long) btBuffer)
	{
		--(btPointer);
		 *btPointer = btValue;
	}
}

Void memsetf(Void *bPointer, SIZE lgsValue, SIZE bSize)
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

Void memcpy(const Void *Obj1, Void *Obj2, SIZE ObjSize)
{
	char *O1 = (char *) Obj1;
	char *O2 = (char *) Obj2;
	
	while(ObjSize)
	{
		--ObjSize;
		O2[ObjSize] = O1[ObjSize];
	}
}

Void memcpyf(const Void *bPointerI, Void *bPointerII, SIZE bufferSize)
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

bool memcmp(const Void *bPointerI, const Void *bPointerII, SIZE bSize)
{
	unsigned char *bCounterI  = (unsigned char*) bPointerI;
	unsigned char *bCounterII = (unsigned char*) bPointerII;
	long bOffset = (long) bSize;
	
	while(bOffset > 0) 
	{
		--(bOffset);
		if(*bCounterI != *bCounterII)
			return (false);
		else 
		{
			++(bCounterI); ++(bCounterII);
		}
	}
	return (true);
}

bool strcmp(const char *s1, const char *s2)
{
	while(TRUE) 
	{
		if(*s1 != *s2) 
		{
			return (false);
		}
		else if(*s1 == '\0')
			break;
		++s1;
		++s2;
	}
	return (true);
}

bool strcmpn(const char *str0, const char *str1, const long n)
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
