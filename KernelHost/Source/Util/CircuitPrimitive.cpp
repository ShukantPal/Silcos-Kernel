/**
 * Copyright (C) 2017 - Shukant Pal
 */

#include <Debugging.h>
#include <Types.h>
#include "../../../Interface/Utils/CtPrim.h"

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
///
/// Copies all the characters in the string-buffer `str0` to the buffer
/// located at `str1` till the null-terminating character arrives.
///
/// @param str0[in] - source string-buffer, only read
/// @param str1[out] - destination buffer, written only
/// @version 1.0
/// @author Shukant Pal
///
void strcpy(const char *str0, char *str1)
{
	if(*str0)
	{
		while(*str0)
		{
			*str1 = *str0;
			++(str1);
			++(str0);
		}

		*str1 = '\0';// null-terminator
	}
}
