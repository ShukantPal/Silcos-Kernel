///
/// @file Console.cpp
///
/// Allows printing to VGA text-based screens whose video-RAM is located at a
/// fixed virtual address. This should be initialized by HAL to allow on-screen
/// debugging. It is assumed that the screen is a grid of 80x25 characters.
///
/// Changes:
/// a. Fixed line-by-line printing & line-overflows which lead to mixed text
/// b. Allow reprinting from top of the screen, when screen becomes full
///
/// -------------------------------------------------------------------
/// This program is free software: you can redistribute it and/or modify
/// it under the terms of the GNU General Public License as published by
/// the Free Software Foundation, either version 3 of the License, or
/// (at your option) any later version.
///
/// This program is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
/// GNU General Public License for more details.
///
/// You should have received a copy of the GNU General Public License
/// along with this program.  If not, see <http://www.gnu.org/licenses/>
///
/// Copyright (C) 2017 - Shukant Pal
///
#include <Types.h>
#include <Debugging.h>
#include <Synch/Spinlock.h>
#include <Utils/Memory.h>

static volatile U8 *writeBuffer;
static volatile U8 *bufferPos;
static unsigned short writeIndex = 0;
static DebugStream Console;
extern Spinlock dbgLock;

static inline void clearLine(unsigned long lineIndex)
{
	memsetf((void*) bufferPos, (0x17 << 24) | (0x17 << 8),
			2 * (80 - writeIndex % 80));
}

static inline void flushLine(unsigned long fromIndex)
{
	memsetf((void*)(writeBuffer + 2 * fromIndex),
			(0x0F << 24) | (0x0F << 8), 160);
}

static inline void finishLine()
{
	memsetf((void*) bufferPos, (0x17 << 24) | (0x17 << 8),
			2 * (80 - writeIndex % 80));
}

///
/// Changes the write-index & buffer-position to the point where the next line
/// starts on the console.
///
/// @returns change in the write-index (delta)
/// @author Shukant Pal
///
static inline unsigned long switchLine()
{
	if(writeIndex % 80)
	{
		unsigned long delta = 80 - writeIndex % 80;
		bufferPos += 2 * delta;
		writeIndex += delta;

		if(writeIndex != 80 * 25)
		{
			clearLine(writeIndex / 80);
			(bufferPos[0]) = '>';
			(bufferPos[1]) = 0x7;
			(bufferPos[2]) = '>';
			(bufferPos[3]) = 0x7;
		}
	
		return (delta);
	}
	else
		return (0);
}

static inline void markLine()
{
	unsigned long markerPos = (writeIndex % 80) ?
					writeIndex + 80 - writeIndex % 80 : writeIndex;
	flushLine(markerPos);

	volatile U8 *marker = writeBuffer + 2 * markerPos;
	marker[0] = '>';
	marker[2] = '>';
	marker[4] = '>';
}

static inline void restoreConsole()
{
	if(writeIndex >= 80 * 25)
	{
		writeIndex = 0;
		bufferPos = writeBuffer;
		clearLine(0);
	}
}

///
/// Continues printing the ascii-string onto the console screen until a line has
/// been fully written. It can switch to the next line (and reset its counter)
/// if  a '\n' (line-escape) sequence is encountered.
///
/// @param asciiString - the part of the string, from which the print
/// 			should start. the pointer to the part of the string,
/// 			till which characters have been printed already.
///
/// Changes:
/// # Add support for tabs, backspace and more!
///
/// @version 1.0
/// @since Silcos 2.05
/// @author Shukant Pal
///
static const char *printInline(const char *asciiString)
{
	if(!(writeIndex % 80))
		finishLine();

	unsigned long inlineIndex = (unsigned long)(writeIndex % 80);
	unsigned long oldIndex = inlineIndex;
	while(inlineIndex < 80 && *asciiString)
	{
		switch(*asciiString)
		{
		case '\n':
		case '\r':
			writeIndex += inlineIndex - oldIndex;
			finishLine();
			switchLine();
			inlineIndex = 0;
			oldIndex = 0;
			break;
		case '\t':
			for(unsigned long tabIdx = 0; tabIdx <= 8 && inlineIndex < 80; tabIdx++){
				*(bufferPos++) = ' ';
				*(bufferPos++) = 0x17;
				++(inlineIndex);
			}
			break;
		case '\b':
			bufferPos -= 2;
			--(writeIndex);
			break;
		default:
			*(bufferPos++) = *asciiString;
			*(bufferPos++) = 0x17;
			++(inlineIndex);
			break;
		}
		++(asciiString);
	}

	writeIndex += inlineIndex - oldIndex;
	return (asciiString);
}


extern "C" void ClearScreen()
{
	SpinLock(&dbgLock);

	unsigned short currentPos = 0;
	while(currentPos < 80 * 25 * 2)
	{
		writeBuffer[currentPos++] = ' ';
		writeBuffer[currentPos++] = 0x07;
	}

	bufferPos = 0;
	writeIndex = 0;

	SpinUnlock(&dbgLock);
}

extern "C" U8 InitConsole(U8 *vid)
{
	writeBuffer = vid;
	bufferPos = writeBuffer;

	unsigned short currentPos = 0;
	while(currentPos < 80 * 25 * 2)
	{
		writeBuffer[currentPos++] = ' ';
		writeBuffer[currentPos++] = 0x07;
	}

	Console.Write= &Write;
	Console.WriteLine = &WriteLine;

	return (AddStream(&Console));
}

extern "C" void WriteTo(U8 *buf)
{
	writeBuffer = buf;
}

extern "C" void Write(const char *msg)
{
	restoreConsole();

	const char *ch = msg;
	ch = printInline(ch);

	// This part will occur only if msg overflows this line.
	while(*ch)
	{
		switchLine();
		ch = printInline(ch);
	}

	markLine();
}

extern "C" void WriteLine(const char *msg)
{
	Write(msg);
	finishLine();
	switchLine();
	markLine();
}
