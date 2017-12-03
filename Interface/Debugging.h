/**
 * File: Debugging.h
 *
 * Summary:
 * Allows kernel-mode debugging with a fixed no. of streams. When the kernel
 * matures a custom-debugger is registered which allows dynamic-debugging
 * streams.
 *
 * Copyright (C) 2017 - Shukant Pal
 */
#ifndef DEBUGGER_H__
#define DEBUGGER_H__

#include "Types.h"

struct DebugStream
{
	ULONG Status;
	CHAR *CurrentBuffer;
	Void (*Write) (const char *);
	Void (*WriteLine) (const char *);
};

decl_c
{
void Dbg(const char *dbgString);
void DbgInt(SIZE_T);
void DbgErro(SIZE_T, ULONG Digits);
void DbgDump(void *Pointer, ULONG DumpByteSize);
void DbgBinOut(Void *Pointer, ULONG DumpByteSize);
void DbgLine(const char *String);

	/* Debug-stream g/s interface */
ULONG AddStream(struct DebugStream *);
void RemoveStream(ULONG Index);

U8 InitConsole(U8* videoRAM);
void Write(const char *consoleASCIIString);
void WriteLine(const char *consoleASCIIString);
}

#endif /* Debugging.h */
