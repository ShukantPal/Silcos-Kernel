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

///
/// @struct ModuleRecord
///
/// This struct is used for modeling a runtime-record of a kernel module. It
/// contains ABI information and dynamic-linking information related to the
/// module.
///
/// It is used by the RecordManager for cross-ABI searches for symbols exported
/// by software-components. Client must initialize it & register it into the
/// RecordManager.
///
/// @version 1.0
/// @since Circuit 2.03
/// @author Shukant Pal
///
typedef
struct ModuleRecord
{
	union
	{
		struct
		{
			ModuleRecord *NextModule;
			ModuleRecord *PreviousModule;
		};//! used for linking these records in linked lists
		LinkedListNode LiLinker;
	};
	char buildName[16];//! name given to the module during build-time
	unsigned long buildVersion;//! version of the module build
	KM_TYPE serviceType;//! type of module & its services
	DynamicLink *linkerInfo;//! exported link-info (@see ElfManager)
	void (*init)();//! initialization function for this module
	ADDRESS entryAddr;//! main-function address
	void (*fini)();//! finalization function for this module
	ADDRESS BaseAddr;//! base virtual address where this module is mapped
} KMOD_RECORD;

///
/// @class RecordManager
///
/// This class provides a ABI-independent organization of modules & their
/// associated symbol lookup tables. The RecordManager helps the dynamic
/// linkers to query symbols & module-information globally.
///
/// It also helps the linker to process multiple inter-dependent modules,
/// each of which the dynamic-link info is exported and inserted into the
/// respective records. Then the relocations are done on each of them.
///
///
/// @version 1.1
/// @since Circuit 2.03++
/// @author Shukant Pal
///
class RecordManager
{
public:
	static inline void registerRecord(ModuleRecord *modRecord)
	{
		AddElement((LinkedListNode *) modRecord, &globalRecordList);
	}

	static ModuleRecord *create(char *modName, unsigned long modVersion,
					unsigned long type);
	static const ModuleRecord *search(char *name);
	static Module::Elf::Symbol *querySymbol(char *symbolName,
					unsigned long& baseAddress);
private:
	static LinkedList globalRecordList kxhide;
};

}

Module::ModuleRecord *MdCreateModule(char *moduleName,
					unsigned long moduleVersion,
					unsigned long moduleType);
extern ObjectInfo *tKMOD_RECORD;
extern LinkedList LoadedModules;

#endif
