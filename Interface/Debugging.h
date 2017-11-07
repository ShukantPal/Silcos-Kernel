/**
 * File: Debugging.h
 *
 * Summary: This file contains the interface for debugging with streams.
 *
 * Copyright (C) 2017 - Shukant Pal
 */
#ifndef __DEBUGGER_H__
#define __DEBUGGER_H__

#include "Types.h"

/* A software-debuggable port. */
typedef
struct DebugStream
{
	ULONG Status;
	CHAR *CurrentBuffer;
	Void (*Write) (CHAR *);
	Void (*WriteLine) (CHAR *);
} DEBUG_STREAM;

/* Backware-compat macros */
#define Debug Dbg
#define DebugInt DbgInt
#define DebugLine DbgLine

/* Debugging-utiltity branches */
#ifndef FBUILD_C
decl_c {
#endif
	void Dbg(CHAR *dbgString);
	void DbgInt(SIZE_T);
	void DbgErro(SIZE_T, ULONG Digits);
	void DbgDump(VOID *Pointer, ULONG DumpByteSize);
	void DbgBinOut(Void *Pointer, ULONG DumpByteSize);
	void DbgLine(CHAR *String);

	/* Debug-stream g/s interface */
	ULONG AddStream(struct DebugStream *);
	void RemoveStream(ULONG Index);
#ifndef FBUILD_C
}
#endif
	/* Native text-console prime debugging facility */
	UCHAR InitConsole(
		UCHAR *VideoBuffer
	);

	void Write(
		CHAR  *ConsoleASCIIString
	);

	void WriteLine(
		CHAR *ConsoleASCIIString
	);

#endif /* Debugging.h */
