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

unsigned char ModuleName[16] = "KTERM";
unsigned long ModuleVersion = 1001000;
unsigned long ModuleType = 0;

unsigned long NO_ENTRY = 0xDBDAFEFC;

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
extern "C" void KModuleMain(){
	DbgLine("Loaded package com.silcos.obmgr");

	while(TRUE) asm volatile("nop"); // no thread exit
}
