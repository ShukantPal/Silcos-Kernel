/**
 * File: Console.c
 *
 * Summary:
 * Allows printing to VGA text-based screens whose video-RAM is located at a
 * fixed virtual address. This should be initialized by HAL to allow on-screen
 * debugging. It is assumed that the screen is a grid of 80x25 characters.
 *
 * Changes:
 * # Fixed line-by-line printing & line-overflows which lead to mixed text
 * # Allow reprinting from top of the screen, when screen becomes full
 *
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
#include <Types.h>
#include <Debugging.h>
#include <Util/Memory.h>

static volatile U8 *writeBuffer;
static volatile U8 *bufferPos;
static unsigned short writeIndex = 0;
static DebugStream Console;

static inline void ClearLine(unsigned long lineIndex)
{
	memset((void*) bufferPos, 0, 2 * (80 - writeIndex % 80));
}

static inline unsigned long SwitchLine()
{
	if(writeIndex % 80){
		unsigned long delta = 80 - writeIndex % 80;
		bufferPos += 2 * delta;
		writeIndex += delta;

		if(writeIndex != 80 * 25){
			ClearLine(writeIndex / 80);
			(bufferPos[0]) = '>';
			(bufferPos[1]) = 0x07;
			(bufferPos[2]) = '>';
			(bufferPos[3]) = 0x07;
		}
		return (delta);
	} else
		return (0);
}

static inline void RestoreConsole()
{
	if(writeIndex == 80 * 25){
		writeIndex = 0;
		bufferPos = writeBuffer;
		ClearLine(0);
	}
}

static const char *PrintInline(const char *asciiString)
{
	unsigned long inlineIndex = (unsigned long) (writeIndex % 80);
	unsigned long oldIndex = inlineIndex;
	while(inlineIndex < 80 && *asciiString){
		switch(*asciiString)
		{
		case '\n':
		case '\r':
			writeIndex += inlineIndex - oldIndex;
			SwitchLine();
			inlineIndex = 0;
			oldIndex = 0;
			break;
		case '\t':
			for(unsigned long tabIdx = 0; tabIdx <= 8 && inlineIndex < 80; tabIdx++){
				*(bufferPos++) = ' ';
				*(bufferPos++) = 0x7;
				++(inlineIndex);
			}
			break;
		case '\b':
			bufferPos -= 2;
			--(writeIndex);
			break;
		default:
			*(bufferPos++) = *asciiString;
			*(bufferPos++) = 0x07;
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
	unsigned short currentPos = 0;
	while(currentPos < 80 * 25 * 2) {
		writeBuffer[currentPos++] = ' ';
		writeBuffer[currentPos++] = 0x07;
	}
}

extern "C" U8 InitConsole(U8 *vid) {
	writeBuffer = vid;
	bufferPos = writeBuffer;

	unsigned short currentPos = 0;
	while(currentPos < 80 * 25 * 2) {
		writeBuffer[currentPos++] = ' ';
		writeBuffer[currentPos++] = 0x07;
	}

	Console.Write= &Write;
	Console.WriteLine = &WriteLine;

	return AddStream(&Console);
}

extern "C" void WriteTo(U8 *buf) {
	writeBuffer = buf;
}

extern "C" void Write(const char *msg) {/*
	const char *ch = msg;
	while(*ch != '\0'){
		*bufferPos = *ch;
		++bufferPos;
		*bufferPos = 0x07;
		++bufferPos;
		++ch;	
	}
	writeIndex += 2 * ((unsigned long) ch - (unsigned long) msg - 1);*/

	RestoreConsole();

	const char *ch = msg;
	ch = PrintInline(ch);

	// This part will occur only if msg overflows this line.
	while(*ch){
		SwitchLine();
		ch = PrintInline(ch);
	}
}

extern "C" void WriteLine(const char *msg) {
	Write(msg);
	SwitchLine();
}
