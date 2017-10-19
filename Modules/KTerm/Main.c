/*M=+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * File: Main.c
 *
 * Summary: This file contains the code to initialize the KTerm core interface.
 *
 * 
 *+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=M*/
 
#include <Debugging.h>
#include <TYPE.h>

extern void elf_dbg();

VOID KModuleMain(){
	volatile UCHAR *video_buffer = (volatile UCHAR *) 0xC00B8000;
	*video_buffer = 'M';
	video_buffer[2] = 'O';
	video_buffer[4] = 'D';

	elf_dbg();
	
	*video_buffer = 'F';
}
