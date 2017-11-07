/**
 * File: ElfLinker.hpp
 *
 * Copyright (C) 2017 - sukantdev
 */
#ifndef INTERFACE_MODULE_ELFLINKER_HPP_
#define INTERFACE_MODULE_ELFLINKER_HPP_

#include "ELF.h"
#include "ElfManager.hpp"

namespace Module
{

namespace Elf
{

/**
 * Class: ElfLinker
 *
 * Summary:
 * This class contains the vital services to combine link elf-binaries with
 * other modules. It resolves relocation entries & may (in the future)
 * implement GOT & PLT stuff.
 *
 * Version: 1.0
 * Since: Circuit 2.03++
 * Author: Shukant Pal
 */
class ElfLinker
{
public:
	static void __fill_edbg(RelEntry*, ElfManager&);

	static void resolveRelocation(RelEntry *relocEntry, ElfManager &handlerService);
	static void resolveRelocation(RelaEntry *relocEntry, ElfManager &handlerService);
	static void resolveRelocations(RelTable &relocTable, ElfManager &handlerService);
	static void resolveRelocations(RelaTable &relaTable, ElfManager &handlerService);

	static inline void resolveRelocations(RelocationTable &relocTable, ElfManager &handlerService)
	{
		if(relocTable.relocType == DT_REL)
			ElfLinker::resolveRelocations((RelTable&) relocTable, handlerService);
	}

	static inline void resolveLinkage(ElfManager &modService)
	{
		ElfLinker::resolveRelocations(modService.relTable, modService);
		ElfLinker::resolveRelocations(modService.pltRelocTable, modService);
	}
};

}// namespace Elf

}// namespace Module

#endif /* INTERFACE_MODULE_ELFLINKER_HPP_ */
