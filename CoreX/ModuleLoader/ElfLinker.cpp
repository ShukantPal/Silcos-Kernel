/**
 * File: ElfLinker.cpp
 *
 * Summary:
 * This file contains the implementation for the dynamic linkage & relocation
 * services for modules. It is used by the module-loader to modules as shared
 * libraries in the kernel-space.
 * 
 * Functions:
 *
 * Origin:
 *
 * Copyright (C) 2017 - Shukant Pal
 */
#include <Module/Elf/ElfLinker.hpp>
#include <Module/ModuleRecord.h>
#include <KERNEL.h>

using namespace Module;
using namespace Module::Elf;

/**
 * Function: ElfLinker::resolveRelocation
 *
 * Summary:
 * This function resolves a 'rel' type relocation.
 *
 * Args:
 * RelEntry *relocEntry - 'Rel' entry descriptor
 * ElfManager &handlerService - Module's elf-manager
 *
 * Author: Shukant Pal
 */
void ElfLinker::resolveRelocation(
		RelEntry *relocEntry,
		ElfManager &handlerService
){
	ULONG *relocationArena = (ULONG*) (handlerService.baseAddress + relocEntry->Offset);

	ULONG symbolIdx = ELF32_R_SYM(relocEntry->Info);
	Symbol *symbolFound = handlerService.dynamicSymbols.entryTable + symbolIdx;

	CHAR *signature = handlerService.dynamicSymbols.nameTable + symbolFound->Name;
	Symbol *declarer = RecordManager::querySymbol(signature);

	if(declarer != NULL){
		*relocationArena = declarer->Value;
	}
}

/**
 * Function: ElfLinker::resolveRelocations
 *
 * Summary:
 * This function resolves all the relocations in a 'rel' relocation table.
 *
 * Args:
 * RelTable &relocTable - 'Rel' relocation table
 * ElfManager &handlerService - Elf-manager for the relevant module
 *
 * Author: Shukant Pal
 */
void ElfLinker::resolveRelocations(
		RelTable &relocTable,
		ElfManager &handlerService
){
	ULONG relIndex = 0;
	ULONG relCount = relocTable.entryCount;
	RelEntry *relDesc = relocTable.entryTable;

	while(relIndex < relCount){
		ElfLinker::resolveRelocation(relDesc, handlerService);

		++(relIndex);
		++(relDesc);
	}
}
