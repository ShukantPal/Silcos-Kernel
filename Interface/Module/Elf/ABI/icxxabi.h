/**
 * File: icxxabi.h
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

#ifndef INTERFACE_MODULE_ELF_ABI_ICXXABI_H_
#define INTERFACE_MODULE_ELF_ABI_ICXXABI_H_

#include <Util/LinkedList.h>

namespace Elf
{
namespace ABI
{


/**
 * Struct: ExitorFunction
 *
 * Summary:
 * Records a function that should be called during unloading of a shared kernel
 * library. The function takes a object as a parameter and the record also
 * holds a dso-handle.
 *
 * Functions:
 * execute - call the handler function
 *
 * Version: 1.2
 * Author: Shukant Pal
 */
struct ExitorFunction
{
	LinkedListNode linkageNode;
	void (*exitorFunc)(void *objectArg);// Termination handler
	void *object;// Argument taken by the handler
	void *dsoHandle;// DSO-handle provided during construction
};

} // ABI
} // Elf

decl_c {

int __cxa_atexit(void (*f)(void*), void *objptr, void *dso);
void __cxa_finalize(void *f);

}

#endif /* INTERFACE_MODULE_ELF_ABI_ICXXABI_H_ */
