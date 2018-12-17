/**
 * @file Debugging.h
 *
 * Provides C-style debugging (early) for the kernel.
 * -------------------------------------------------------------------
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
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

	DebugStream() // @suppress("Class members should be properly initialized")
	{
		/* See the wiki on EarlyInitRace, to know why not
		   members are init'ed */
	}
};

#ifndef _CBUILD
extern "C"
{
#endif

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

#ifndef _CBUILD
}
#endif

#endif /* Debugging.h */
