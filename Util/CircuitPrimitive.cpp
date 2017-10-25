/**
 * Copyright (C) 2017 - Shukant Pal
 */

#include <Debugging.h>
#include <Types.h>
#include <Util/CtPrim.h>

VOID ASSERT(BOOL c, CHAR *String)
{
	if(!c)
	{
		DbgLine(String);
		DbgLine("Circuit Fused ! Platform Shutting Down Due To Undetectable Condition");
		asm volatile("cli");
		while(TRUE) asm volatile("hlt");
	}
}

VOID memset(VOID *bPointer, UBYTE btValue, SIZE bSize){
	UBYTE *btBuffer = (UBYTE *) bPointer;/* Transfer bytes */
	UBYTE *btPointer = (UBYTE *) bPointer + bSize;/* Transfer serially */
	while((ULONG) btPointer > (ULONG) btBuffer) {
		--(btPointer);
		 *btPointer = btValue;
	}
}

VOID memsetf(VOID *bPointer, SIZE_T lgsValue, SIZE_T bSize){
	ULONG *lgBuffer = bPointer;
	bSize &= ~(3);
	ULONG *lgPointer = (ULONG *) ((UBYTE *) bPointer + bSize);
	while((ULONG) lgPointer > (ULONG) lgBuffer){
		--(lgPointer);
		*lgPointer = lgsValue;
	}
}

VOID memcpy(const VOID *Obj1, VOID *Obj2, SIZE_T ObjSize){
	CHAR *O1 = (CHAR *) Obj1;
	CHAR *O2 = (CHAR *) Obj2;
	while(ObjSize){
		--ObjSize;
		O2[ObjSize] = O1[ObjSize];
	}
}

VOID memcpyf(const VOID *Obj1, VOID *Obj2, SIZE ObjSize){
	ULONG *O1 = (ULONG *) Obj1;
	ULONG *O2 = (ULONG *) Obj2;
	ObjSize /= sizeof(SIZE);
	while(ObjSize) {
		--ObjSize;
		O2[ObjSize] = O1[ObjSize];
	}
	O2[0] = O1[0];
}

BOOL memcmp(const VOID *bPointerI, const VOID *bPointerII, SIZE bSize){
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
