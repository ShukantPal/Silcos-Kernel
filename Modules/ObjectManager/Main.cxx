/**
 * File: Main.c
 *
 * Summary:
 * This file contains the code to initialize the KTerm core interface.
 *
 * Copyright (C) 2017 - Shukant Pal
 */

#include <Object.hxx>
#include <Debugging.h>
#include <Heap.hxx>
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
decl_c void KModuleMain(){
	DbgLine("Loaded package com.silcos.obmgr");

	//char *kmal = (char*) kmalloc(12);

	while(TRUE) asm volatile("nop"); // no thread exit
}
