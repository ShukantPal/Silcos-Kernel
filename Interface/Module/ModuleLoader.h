/*=++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * File: ModuleLoader.h
 *
 * Summary:
 * ModuleLoader provides the interface to dynamically load and deload module binaries
 * which are present in memory already. It is a linker for the kernel & module and
 * can load a variety of file formats into the kernel.
 *
 * Copyright (C) 2017 - Shukant Pal
 *+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=*/
#ifndef __MODULE_MODULE_LOADER_H__
#define __MODULE_MODULE_LOADER_H__

#include <Memory/Pager.h> // For PADDRESS type

BOOL MdLoadFile(
	PADDRESS moduleAddress,
	ULONG moduleSize,
	CHAR *cmdLine
);

VOID MdSetupLoader(
	VOID
);

#endif/* Module/ModuleLoader.h */
