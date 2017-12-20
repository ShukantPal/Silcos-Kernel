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
	unsigned long Status;
	char *CurrentBuffer;
	Void (*Write) (const char *);
	Void (*WriteLine) (const char *);
};

extern "C"
{
void Dbg(const char *dbgString);
void DbgInt(SIZE);
void DbgErro(SIZE, unsigned long Digits);
void DbgDump(void *Pointer, unsigned long DumpByteSize);
void DbgBinOut(Void *Pointer, unsigned long DumpByteSize);
void DbgLine(const char *String);

/* Debug-stream g/s interface */
unsigned long AddStream(struct DebugStream *);
void RemoveStream(unsigned long Index);

U8 InitConsole(U8* videoRAM);
void Write(const char *consoleASCIIString);
void WriteLine(const char *consoleASCIIString);
void ClearScreen();
}

#endif /* Debugging.h */
