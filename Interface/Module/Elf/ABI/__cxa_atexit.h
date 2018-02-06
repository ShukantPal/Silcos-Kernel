///
/// @file __cxa_atexit.h
///
/// (don't include this file, see Implementor.h)
/// -------------------------------------------------------------------
/// This program is free software: you can redistribute it and/or modify
/// it under the terms of the GNU General Public License as published by
/// the Free Software Foundation, either version 3 of the License, or
/// (at your option) any later version.
///
/// This program is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
/// GNU General Public License for more details.
///
/// You should have received a copy of the GNU General Public License
/// along with this program.  If not, see <http://www.gnu.org/licenses/>
///
/// Copyright (C) 2017 - Shukant Pal
///
#ifndef HDR_MODULE_ELF_ABI___CXA_ATEXIT_H_
#define HDR_MODULE_ELF_ABI___CXA_ATEXIT_H_

#include "icxxabi.h"
#include <Memory/KObjectManager.h>
#include <Utils/LinkedList.h>

extern "C"
{

//! object type defined in ModuleLoader to allocate call-back structs
extern ObjectInfo *tElf_ABI_ExitorFunc_;

//! linked-list of module at-exit call-back entries
LinkedList __atExitEntries;

//! abi-specified dso-handle which is to be used by dynamic linker
void *__dso_handle = 0;

///
/// Used for registering a call-back function which is to be called during
/// module unloading. This function should free all module-specific resources
/// and shutdown any module background-tasks.
///
/// This is an ABI-specific functionality and modules should be cautious about
/// using it.
///
/// @param exitorFunc - call-back pointer which is executed during unloading
/// @param object - the only parameter to the at-exit function
/// @param dsoHandle - the modules dso-handle used by dynamic-linker
/// @return - always 0, passed due to ABI conformance
/// @version 1.0
/// @since Silcos 2.05
/// @author Shukant Pal
///
int __cxa_atexit(void (*exitorFunc)(void *), void *object, void *dsoHandle)
{
	Elf::ABI::ExitorFunction *funcDetails =
			new(tElf_ABI_ExitorFunc_) Elf::ABI::ExitorFunction();

	funcDetails->exitorFunc = exitorFunc;
	funcDetails->object = object;
	funcDetails->dsoHandle = dsoHandle;

	AddElement(&funcDetails->linkageNode, &__atExitEntries);
	++(dsoHandle);

	return (0);
}

}// extern "C"

#endif/* Elf/ABI/__cxa_atexit.h*/
