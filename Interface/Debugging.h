/*=++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * File: Debugging.h
 *
 * Summary: This file contains the interface for debugging with streams.
 *
 * Copyright (C) 2017 - Shukant Pal
 *+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=*/
#ifndef DEBUGGER_H
#define DEBUGGER_H

#include "Types.h"

/* A software-debuggable port. */
typedef
struct {
	ULONG Status;
	CHAR *CurrentBuffer;
	VOID (*Write) (CHAR*);
	VOID (*WriteLine) (CHAR*);
} DEBUG_STREAM;

/* Backware-compat macros */
#define Debug Dbg
#define DebugInt DbgInt
#define DebugLine DbgLine

/* Debugging-utiltity branches */
VOID Dbg(
	CHAR *dbgString
);

VOID DbgInt(
	SIZE_T
);

VOID DbgErro(
	SIZE_T,
	ULONG Digits
);

VOID DbgDump(
	VOID *Pointer,
	ULONG DumpByteSize
);

VOID DbgBinOut(
	VOID *Pointer,
	ULONG DumpByteSize // Must be multiple of 4
);

VOID DbgLine(
	CHAR *String
);

/* Debug-stream g/s interface */
ULONG AddStream(
	DEBUG_STREAM*
);

VOID RemoveStream(
	ULONG Index
);

/* Native text-console prime debugging facility */
UCHAR InitConsole(
	UCHAR *VideoBuffer
);

VOID Write(
	CHAR  *ConsoleASCIIString
);

VOID WriteLine(
	CHAR *ConsoleASCIIString
);

#endif /* Debugging.h */
