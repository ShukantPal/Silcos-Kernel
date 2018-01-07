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
#include <KERNEL.h>

DebugStream *Streams[10];
char StreamNo = -1;

const char *__space = " ";
const char *__leftparen = "(";
const char *__rightparen = ") ";
const char *__comma = ", ";

SPIN_LOCK dbgLock;

char digitMapping[36] = "0123456789ABCDEFGHIJK";
char digitZero[2] = "0";
static void ToString(SIZE n, char* buffer, unsigned char d)
{
	if(d < 2 || d > 36)
	{
		buffer = "";
		return;			
	}
	else
	{
		if(n == 0)
		{
			buffer[0] = '0';
			buffer[1] = '\0';
			return;
		}
		
		char reverseBuffer[1 + (sizeof(unsigned long) * 8)];
		long charIndex = 0;
		bool neg = FALSE;
		if(n < 0)
		{
			neg = TRUE;
			n *= -1;
		}

		while(n > 0)
		{
			reverseBuffer[charIndex++] = digitMapping[n % d];
			n /= d;
		}

		if(neg)
			reverseBuffer[charIndex++] = '-';

		reverseBuffer[charIndex--] = '\0';

		long bufferIndex = 0;
		while(charIndex >= 0)
			buffer[bufferIndex++] = reverseBuffer[charIndex--];
		buffer[bufferIndex] = '\0';
	}
}

static void ToFillString(SIZE n, char *buffer, unsigned char d)
{
	if(d < 2 || d > 36)
	{
		buffer[32] = '\0';
		return;			
	}
	else
	{
		if(n == 0)
		{
			memset(buffer, '0', 32);
			buffer[32] = '\0';
			return;
		}

		char reverseBuffer[65];
		long charIndex = 0;
		bool neg = FALSE;
		if(n < 0)
		{
			neg = TRUE;
			n *= -1;
		}

		while(n > 0)
		{
			reverseBuffer[charIndex++] = digitMapping[n % d];
			n /= d;
		}

		while(charIndex < 32)
			reverseBuffer[charIndex++] = '0';

		if(neg)
			reverseBuffer[charIndex++] = '-';
		--charIndex;

		long org_c = 0;
		while(charIndex >= 0)
		{
			buffer[org_c++] = reverseBuffer[charIndex--];
		}

		buffer[32] = '\0';
	}
}

extern "C" void Dbg(const char *msg)
{
	SpinLock(&dbgLock);
	unsigned char Stream = 0;
	for( ; Stream <= StreamNo; Stream++)
		Streams[Stream] -> Write(msg);
	SpinUnlock(&dbgLock);
}

extern "C" void DbgInt(SIZE n)
{
	if(n) {
		char buf[33];
		ToString(n, buf, 10);
		Dbg(buf);
	} else Dbg(digitZero);
}

extern "C" void DbgErro(SIZE n, unsigned long sy)
{
	//if(n) {
		char buf[65];
		ToFillString(n, buf, sy);
		Dbg(buf);
	//} else Dbg("0");
}

extern "C" void DbgDump(void *c_, unsigned long s)
{
	char c[s + 1];
	memcpy(c_, c, s);
	c[s] ='\0';
	Dbg(c);
}

extern "C" void DbgBinOut(void *c_, unsigned long s) // s must be multiple of 4 (to dump all)
{
	unsigned long s_ = s / 4;
	long i = s_ - 1;
	while(i >= 0)
	{
		DbgErro(((unsigned long*) c_)[i], 2);
		--i;
	}
}

extern "C" void DbgLine(const char  *msg)
{
	SpinLock(&dbgLock);
	char Stream = 0;
	for( ; Stream <= StreamNo; Stream++)
		Streams[Stream] -> WriteLine(msg);
	SpinUnlock(&dbgLock);
}

unsigned long AddStream(DebugStream *dbg)
{
	++StreamNo;
	if(StreamNo == 10)
	{
		--StreamNo;
		return -1;
	}
	else
	{
		Streams[(unsigned long) StreamNo] = dbg;
		return StreamNo;
	}
}

void RemoveStream(unsigned long dbgNo)
{
	unsigned long dbg = dbgNo;
	--StreamNo;
	for( ; (char) dbg < StreamNo; dbg++)
		Streams[dbg] = Streams[dbg + 1];
}
