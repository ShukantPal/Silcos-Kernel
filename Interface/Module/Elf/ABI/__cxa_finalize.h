/**
 * File: __cxa_finalize.h
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
#ifndef INTERFACE_MODULE_ELF_ABI___CXA_FINALIZE_H_
#define INTERFACE_MODULE_ELF_ABI___CXA_FINALIZE_H_

#include "icxxabi.h"
#include "__cxa_atexit.h"

/**
 * Finalizes freeing operation during the unloading of shared kernel modules &
 * is internal to the module. It calls all the registered unloading handlers
 * as specified by ABI.
 *
 * Args:
 * void *callFunc - the only function that should be called (NULL, if all
 * 			handlers should be called in each record).
 *
 * @version 1.0
 * @author Shukant Pal
 */
extern "C" void __cxa_finalize(void *callFunc)
{
	Elf::ABI::ExitorFunction *record =
			(Elf::ABI::ExitorFunction*) __atExitEntries.head;

	Elf::ABI::ExitorFunction *called;

	if(!callFunc){
		while(record){
			if(record->exitorFunc != NULL)
				record->exitorFunc(record->object);

			called = record;
			record = (Elf::ABI::ExitorFunction*)
					record->linkageNode.next;
			RemoveElement(&called->linkageNode, &__atExitEntries);
			kobj_free((kobj*) called, tElf_ABI_ExitorFunc_);
		}
	} else {
		while(record){
			if(record->exitorFunc == callFunc)
				record->exitorFunc(record->object);

			called = record;
			record = (Elf::ABI::ExitorFunction*)
					record->linkageNode.next;
			RemoveElement(&called->linkageNode, &__atExitEntries);
			kobj_free((kobj*) called, tElf_ABI_ExitorFunc_);
		}
	}
}


#endif/* Elf/ABI/__cxa_finalize.h*/
