/**
 * @file Console.c
 *
 * Presents a neat implementation of console-output that can be used in the
 * initializer, for debugging purposes.
 *
 * The console is used for printing text output without any special arguments
 * (only character strings). The console buffer should be located in accessible
 * physical memory.
 *
 * This file is actually a subset of the Console.cpp source in KernelHost. It
 * can be updated whenever the main-stream console source is modified.
 *
 * @since Silcos 3.05
 * @version 1.0
 * @author Shukant Pal
 */
#define _CBUILD
#include <Generic/EarlyConsole.h>
#include <Utils/CtPrim.h>

char digitMapping[36] = "0123456789ABCDEFGHIJK";
char digitZero[2] = "0";

static void toString(unsigned long num, char* buffer, unsigned char base)
{
	if(base < 2 || base > 36)
	{
		buffer = "";
		return;
	}

	if(num == 0)
	{
		buffer[0] = '0';
		buffer[1] = '\0';
		return;
	}

	char reverseBuffer[1 + (sizeof(unsigned long) * 8)];
	long charIndex = 0;
	bool neg = FALSE;
	if(num < 0)
	{
		neg = TRUE;
		num *= -1;
	}

	while(num > 0)
	{
		reverseBuffer[charIndex++] = digitMapping[num % base];
		num /= base;
	}

	if(neg)
		reverseBuffer[charIndex++] = '-';

	reverseBuffer[charIndex--] = '\0';

	long bufferIndex = 0;
	while(charIndex >= 0)
		buffer[bufferIndex++] = reverseBuffer[charIndex--];

	buffer[bufferIndex] = '\0';
}

static volatile U8 *writeBuffer;
static volatile U8 *bufferPos;
static unsigned short writeIndex = 0;

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

void ClearScreen()
{
	unsigned short currentPos = 0;
	while(currentPos < 80 * 25 * 2)
	{
		writeBuffer[currentPos++] = ' ';
		writeBuffer[currentPos++] = 0x07;
	}

	bufferPos = 0;
	writeIndex = 0;
}

void InitConsole(unsigned char *vid)
{
	writeBuffer = vid;
	bufferPos = writeBuffer;

	unsigned short currentPos = 0;
	while(currentPos < 80 * 25 * 2)
	{
		writeBuffer[currentPos++] = ' ';
		writeBuffer[currentPos++] = 0x07;
	}
}

void Write(const char *msg)
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

void WriteInteger(unsigned long num)
{
	char textBuffer[33];
	toString(num, textBuffer, 10);
	Write(textBuffer);
}

void WriteLine(const char *msg)
{
	Write(msg);
	finishLine();
	switchLine();
	markLine();
}


