/**
 * File: Main.c
 *
 * Summary:
 * This file contains the code to initialize the KTerm core interface.
 *
 * Copyright (C) 2017 - Shukant Pal
 */
#define FBUILD_C

#include <Debugging.h>
#include <TYPE.h>


UCHAR ModuleName[16] = "KTERM";
ULONG ModuleVersion = 1001000;
ULONG ModuleType = 0;

/**
 * Function: KModuleMain
 *
 * Summary:
 * This function initializes the KTERM tree & setups the builtin pre-static
 * subsystems of the 'core'.
 *
 * Version: 1.0
 * Author: Shukant Pal
 */
VOID KModuleMain(){
	DbgLine("KTERM I/O successfully loaded! Taking control of terminal I/O");
	DbgLine("KTERM - Setting up subsystem hierarchy");

	while(TRUE) asm volatile("nop"); // no thread exit
}
