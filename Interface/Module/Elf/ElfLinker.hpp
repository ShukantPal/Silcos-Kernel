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

#include "ELF.h"
#include "ElfManager.hpp"

namespace Module
{
namespace Elf
{

///
/// Provides utility functions to resolve relocations and link various elf
/// objects.
///
/// @version 1.0
/// @since Silcos 2.05
/// @author Shukant Pal
///
class ElfLinker
{
public:
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

#endif/* Module/Elf/ElfLinker.hpp */
