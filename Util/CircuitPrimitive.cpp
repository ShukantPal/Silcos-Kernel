/**
 * Copyright (C) 2017 - Shukant Pal
 */

#include <Debugging.h>
#include <Types.h>
#include <Util/CtPrim.h>

Void ASSERT(BOOL c, CHAR *String)
{
	if(!c)
	{
		DbgLine(String);
		DbgLine("Circuit Fused ! Platform Shutting Down Due To Undetectable Condition");
		asm volatile("cli");
		while(TRUE) asm volatile("hlt");
	}
}

Void memset(Void *bPointer, UBYTE btValue, SIZE bSize){
	UBYTE *btBuffer = (UBYTE *) bPointer;/* Transfer bytes */
	UBYTE *btPointer = (UBYTE *) bPointer + bSize;/* Transfer serially */
	while((ULONG) btPointer > (ULONG) btBuffer) {
		--(btPointer);
		 *btPointer = btValue;
	}
}

Void memsetf(Void *bPointer, SIZE_T lgsValue, SIZE_T bSize){
	ULONG *lgBuffer = bPointer;
	bSize &= ~(3);
	ULONG *lgPointer = (ULONG *) ((UBYTE *) bPointer + bSize);
	while((ULONG) lgPointer > (ULONG) lgBuffer){
		--(lgPointer);
		*lgPointer = lgsValue;
	}
}

Void memcpy(const Void *Obj1, Void *Obj2, SIZE_T ObjSize){
	CHAR *O1 = (CHAR *) Obj1;
	CHAR *O2 = (CHAR *) Obj2;
	while(ObjSize){
		--ObjSize;
		O2[ObjSize] = O1[ObjSize];
	}
}

Void memcpyf(const Void *bPointerI, Void *bPointerII, SIZE bufferSize){
	if(bufferSize){
		ULONG *cPointerSource = (ULONG *) ((UBYTE *) bPointerI + bufferSize);
		ULONG *cPointerDest = (ULONG *) ((UBYTE *) bPointerII + bufferSize);
		bufferSize /= sizeof(SIZE);
		while(bufferSize){
			--(bufferSize);
			--(cPointerSource);
			--(cPointerDest);
			*cPointerDest = *cPointerSource;
		}
	}
}

BOOL memcmp(const Void *bPointerI, const Void *bPointerII, SIZE bSize){
	UBYTE *bCounterI  = bPointerI;
	UBYTE *bCounterII = bPointerII;
	LONG bOffset = (LONG) bSize;
	while(bOffset > 0) {
		--(bOffset);
		if(*bCounterI != *bCounterII)
			return (FALSE);
		else {
			++(bCounterI); ++(bCounterII);
		}
	}
	return (TRUE);
}

BOOL strcmp(const CHAR *s1, const CHAR *s2)
{
	BOOL equ = TRUE;
	while(TRUE) {
		if(*s1 != *s2) {
			equ = FALSE;
			break;
		} else if(*s1 == '\0')
			break;
		++s1;
		++s2;
	}
	return (equ);
}
