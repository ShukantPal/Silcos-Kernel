/*=++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * File: Console.c
 *
 * Summary: This file contains the code for printing screens onto the text-based console.
 *
 * @Version 1
 * @Since Circuit 2.03
 *+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=*/

#include <Types.h>
#include <Debugging.h>

static volatile U8 *writeBuffer;
static volatile U8 *bufferPos;
static USHORT writeIndex = 0;

static DEBUG_STREAM Console;

VOID ClearScreen() 
{
	USHORT currentPos = 0;
	while(currentPos < 80 * 25 * 2) {
		writeBuffer[currentPos++] = ' ';
		writeBuffer[currentPos++] = 0x07;
	}
}

U8 InitConsole(U8 *vid) {
	writeBuffer = vid;
	bufferPos = writeBuffer;

	USHORT currentPos = 0;
	while(currentPos < 80 * 25 * 2) {
		writeBuffer[currentPos++] = ' ';
		writeBuffer[currentPos++] = 0x07;
	}

	Console.Write= &Write;
	Console.WriteLine = &WriteLine;

	return AddStream(&Console);
}

VOID WriteTo(U8 *buf) {
	writeBuffer = buf;
}

VOID Write(CHAR *msg) {
	CHAR *ch = msg;
	while(*ch != '\0'){
		*bufferPos = *ch;
		++bufferPos;
		*bufferPos = 0x07;
		++bufferPos;
		++ch;	
	}
	writeIndex += 2 * ((ULONG) ch - (ULONG) msg - 1);
}

VOID WriteLine(CHAR *msg) {
	Write(msg);
	writeIndex += (80 * 2) - (writeIndex % (80 * 2));
	bufferPos = writeBuffer + writeIndex;
}
