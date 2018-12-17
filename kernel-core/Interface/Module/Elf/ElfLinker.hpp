///
/// @file ElfLinker.hpp
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

#ifndef KERNHOST__MODULE__ELFLINKER_HPP__
#define KERNHOST__MODULE__ELFLINKER_HPP__

#include "ElfManager.hpp"

namespace Module
{

class ModuleContainer;

///
/// Provides utility functions to resolve relocations and link various elf
/// objects.
///
/// @version 1.0
/// @since Silcos 2.05
/// @author Shukant Pal
///
class ElfLinker final
{
public:
	static void resolveRelocation(Elf::RelEntry *relocEntry,
			Elf::ElfManager &handlerService,
			ModuleContainer *objToRelocate);
	static void resolveRelocation(Elf::RelaEntry *relocEntry,
			Elf::ElfManager &handlerService,
			ModuleContainer *objToRelocate);
	static void resolveRelocations(Elf::RelTable &relocTable,
			Elf::ElfManager &handlerService,
			ModuleContainer *objToRelocate);
	static void resolveRelocations(Elf::RelaTable &relaTable,
			Elf::ElfManager &handlerService,
			ModuleContainer *objToRelocate);

	static inline void resolveRelocations(Elf::RelocationTable &relocTable,
			Elf::ElfManager &handlerService,
			ModuleContainer *objToRelocate)
	{
		if(relocTable.relocType == Elf::DT_REL)
			ElfLinker::resolveRelocations((Elf::RelTable&) relocTable,
					handlerService, objToRelocate);
		else
		{
			DbgLine(" Big error - rela not supported");
			while(TRUE);
		}
	}

	static inline void resolveLinkage(Elf::ElfManager &modService,
			ModuleContainer *objToRelocate)
	{
		ElfLinker::resolveRelocations(modService.rel(), modService,
				objToRelocate);
		ElfLinker::resolveRelocations(*modService.getPltRelocations(),
				modService, objToRelocate);
	}
private:
	ElfLinker();
};

}// namespace Module

#endif/* Module/Elf/ElfLinker.hpp */
