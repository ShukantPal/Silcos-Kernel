/*=++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ 
 * File: Debugger.c
 *
 * Summary: This file contains the code for debugging using a generic-interface by using debugging
 * streams. Other data forms that can be printed are also converted into string forms in the process.
 *
 * Copyright (C) 2017 - Shukant Pal 
 *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=*/

#include <Types.h>
#include <Debugging.h>
#include <Util/CtPrim.h>
#include <Synch/Spinlock.h>

DEBUG_STREAM *Streams[10];
CHAR StreamNo = -1;

SPIN_LOCK dbgLock;

CHAR digitMapping[36] = "0123456789ABCDEFGHIJK";
CHAR digitZero[2] = "0";
static
VOID ToString(SIZE n, CHAR* buffer, UCHAR d){
	if(d < 2 || d > 36) {
		buffer = "";
		return;			
	} else {
		if(n == 0) {
			buffer[0] = '0';
			buffer[1] = '\0';
			return;
		}
		
		CHAR reverseBuffer[1 + (sizeof(ULONG) * 8)];
		LONG charIndex = 0;
		BOOL neg = FALSE;
		if(n < 0) {
			neg = TRUE;
			n *= -1;
		}

		while(n > 0) {
			reverseBuffer[charIndex++] = digitMapping[n % d];
			n /= d;
		}

		if(neg)
			reverseBuffer[charIndex++] = '-';

		reverseBuffer[charIndex--] = '\0';

		LONG bufferIndex = 0;
		while(charIndex >= 0)
			buffer[bufferIndex++] = reverseBuffer[charIndex--];
		buffer[bufferIndex] = '\0';
	}
}

static VOID ToFillString(SIZE n, CHAR *buffer, UCHAR d)
{
	if(d < 2 || d > 36) {
		buffer[32] = '\0';
		return;			
	} else {
		if(n == 0) {
			memset(buffer, '0', 32);
			buffer[32] = '\0';
			return;
		}

		CHAR reverseBuffer[65];
		LONG charIndex = 0;
		BOOL neg = FALSE;
		if(n < 0) {
			neg = TRUE;
			n *= -1;
		}

		while(n > 0) {
			reverseBuffer[charIndex++] = digitMapping[n % d];
			n /= d;
		}

		while(charIndex < 32)
			reverseBuffer[charIndex++] = '0';

		if(neg)
			reverseBuffer[charIndex++] = '-';
		--charIndex;

		LONG org_c = 0;
		while(charIndex >= 0) {
			buffer[org_c++] = reverseBuffer[charIndex--];
		} 
//buffer[org_c] = '\0';
		buffer[32] = '\0';
	}
}

VOID Dbg(CHAR *msg) 
{
	SpinLock(&dbgLock);
	UCHAR Stream = 0;
	for( ; Stream <= StreamNo; Stream++)
		Streams[Stream] -> Write(msg);
	SpinUnlock(&dbgLock);
}

VOID DbgInt(SIZE n) 
{
	if(n) {
		CHAR buf[33];
		ToString(n, buf, 10);
		Dbg(buf);
	} else Dbg(digitZero);
}

VOID DbgErro(SIZE n, ULONG sy)
{
	//if(n) {
		CHAR buf[65];
		ToFillString(n, buf, sy);
		Dbg(buf);
	//} else Dbg("0");
}

VOID DbgDump(VOID *c_, ULONG s)
{
	CHAR c[s + 1];
	memcpy(c_, c, s);
	c[s] ='\0';
	Dbg(c);
}

VOID DbgBinOut(VOID *c_, ULONG s) // s must be multiple of 4 (to dump all)
{
	ULONG s_ = s / 4;
	LONG i = s_ - 1;
	while(i >= 0)
	{
		DbgErro(((ULONG*) c_)[i], 2);
		--i;
	}
}

VOID DbgLine(CHAR  *msg){
	SpinLock(&dbgLock);
	ULONG Stream = 0;
	for( ; Stream <= StreamNo; Stream++)
		Streams[Stream] -> WriteLine(msg);
	SpinUnlock(&dbgLock);
}

ULONG AddStream(DEBUG_STREAM *dbg){
	++StreamNo;
	if(StreamNo == 10) {
		--StreamNo;
		return -1;
	} else {
		Streams[(ULONG) StreamNo] = dbg;
		return StreamNo;
	}
}

VOID RemoveStream(ULONG dbgNo){
	ULONG dbg = dbgNo;
	--StreamNo;
	for( ; (CHAR) dbg < StreamNo; dbg++)
		Streams[dbg] = Streams[dbg + 1];
}
