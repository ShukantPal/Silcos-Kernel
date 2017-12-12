/**
 * File: __cxa_atexit.h
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
#ifndef INTERFACE_MODULE_ELF_ABI___CXA_ATEXIT_H_
#define INTERFACE_MODULE_ELF_ABI___CXA_ATEXIT_H_

#include <Memory/KObjectManager.h>
#include "icxxabi.h"
#include <Util/LinkedList.h>

decl_c {

extern ObjectInfo *tElf_ABI_ExitorFunc_;
LinkedList __atExitEntries;
void *__dsohandle = 0;

/**
 * Function: __cxa_atexit
 *
 * Summary:
 * Registers a function which should be run during unloading of the shared
 * kernel module, allowing freeing of critical resources and execution of
 * destructors. It is specified by the System-V ABI for ELF-files.
 *
 * Args:
 * void (*exitorFunc)(void *) - the function which handles termination
 * void *object - the single argument to be given to the function
 * void *dsoHandle -
 */
int __cxa_atexit(void (*exitorFunc)(void *), void *object, void *dsoHandle)
{
	Elf::ABI::ExitorFunction *funcDetails =
			new(tElf_ABI_ExitorFunc_) Elf::ABI::ExitorFunction();

	funcDetails->exitorFunc = exitorFunc;
	funcDetails->object = object;
	funcDetails->dsoHandle = dsoHandle;

	AddElement(&funcDetails->linkageNode, &__atExitEntries);

	return (0);
}

}

#endif/* Elf/ABI/__cxa_atexit.h*/
