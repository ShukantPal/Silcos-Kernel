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
#ifndef HDR_MODULE_ELF_ABI_ICXXABI_H__
#define HDR_MODULE_ELF_ABI_ICXXABI_H__

#include <Utils/LinkedList.h>

namespace Elf
{
namespace ABI
{
///
/// @struct ExitorFunction
///
/// ABI-specific data holding a callback-function to call when a module is
/// unloaded.
///
/// @version 1.0
/// @author Shukant Pal
///
struct ExitorFunction
{
	LinkedListNode linkageNode;
	void (*exitorFunc)(void *objectArg);// Termination handler
	void *object;// Argument taken by the handler
	void *dsoHandle;// DSO-handle provided during construction
};

}// namespace ABI
}// namespace Elf

extern "C"
{
int __cxa_atexit(void (*f)(void*), void *objptr, void *dso);
void __cxa_finalize(void *f);
}

#endif/* Module/Elf/ABI/icxxabi.h */
