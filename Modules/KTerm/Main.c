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

UCHAR ModuleName[16] = "KTERM";
ULONG ModuleVersion = 1001000;
ULONG ModuleType = 0;

/**
 * Function: KModuleMain()
 *
 * Summary:
 * This function initializes the KTERM tree & setups the builtin pre-static
 * subsystems of the 'core'.
 */
VOID KModuleMain(){
	volatile UCHAR *video_buffer = (volatile UCHAR *) 0xC00B8000;
	*video_buffer = 'M';
	video_buffer[2] = 'O';
	video_buffer[4] = 'D';

	elf_dbg();
	
	*video_buffer = 'F';
}
