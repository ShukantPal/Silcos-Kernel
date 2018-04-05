///
/// @file ModuleRecord.h
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
#ifndef __MODULE_RECORD_H__
#define __MODULE_RECORD_H__

#include <Memory/KObjectManager.h>
#include "Elf/ELF.h"
#include "Elf/ElfManager.hpp"
#include <Synch/Spinlock.h>
#include <Synch/ReadWriteSerializer.h>
#include <Utils/LinkedList.h>

namespace Module
{

typedef
enum ModuleType
{
	KMT_KDF_DRIVER	= 1,// Kernel-mode Driver
	KMT_EXC_MODULE	= 2,// Executive Module
	KMT_KMCF_EXTEN	= 3,// KMCF Extension (Kmodule Communicator Framework)
	KMT_UNKNOWN	= 4// Unknown (restricted form)
} KM_TYPE;

enum ABI
{
	INVALID = 0,
	ELF = 1
};

struct DynamicLink
{
	LinkedList *moduleDependencies;
	Module::Elf::SymbolTable dynamicSymbols;
	Module::Elf::HashTable symbolHash;
};

}

Module::ModuleRecord *MdCreateModule(char *moduleName,
					unsigned long moduleVersion,
					unsigned long moduleType);
extern ObjectInfo *tKMOD_RECORD;
extern LinkedList LoadedModules;

#endif
