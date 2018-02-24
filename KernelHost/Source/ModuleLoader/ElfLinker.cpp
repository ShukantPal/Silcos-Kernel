///
/// @file ElfLinker.cpp
///
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
#include <Module/ModuleRecord.h>
#include <Module/Elf/ElfManager.hpp>
#include <Module/Elf/ElfLinker.hpp>

using namespace Module;
using namespace Module::Elf;

extern "C" void LinkerUndefined(void*, void*)
{
	DbgLine("unresolved symbol called");
}

extern "C" void __register_frame_info(void*,struct object*){}
extern "C" void __register_frame_info_bases(void *a1, struct object *a2, void *a3, void *a4);
extern "C" void *__deregister_frame_info(void*){ return (null); }

///
/// Resolves the relocation-entry for the given elf-object. It takes the
/// type of relocation and performs it as stated by the ELF Format
/// Specification. Some types of relocations have not been implemented, so
/// make sure your module doesn't use them or implement them.
///
/// @param relocEntry - relocation-entry to be resolved now
/// @param handlerService - elf-object handler
/// @version 1.0
/// @since Silcos 2.05
/// @author Shukant Pal
///
void ElfLinker::resolveRelocation(RelEntry *relocEntry,
					ElfManager &handlerService)
{
	unsigned long *field = (unsigned long*)(handlerService.baseAddress +
							relocEntry->Offset);
	unsigned long sindex = ELF32_R_SYM(relocEntry->Info);
	Symbol *symbolReferred = handlerService.dynamicSymbols.entryTable + sindex;
	char *signature = handlerService.dynamicSymbols.nameTable + symbolReferred->Name;
	unsigned long declBase;
	Symbol *declarer = RecordManager::querySymbol(signature, declBase);

	if(*signature == '\0' && ELF32_R_TYPE(relocEntry->Info) == R_386_RELATIVE)
	{
		*field += handlerService.baseAddress;
		return;
	}

	if(declarer != NULL)
	{
		switch(ELF32_R_TYPE(relocEntry->Info))
		{
		case R_386_JMP_SLOT:
		case R_386_GLOB_DAT:
			*field = declarer->Value + declBase;
			break;
		case R_386_32:
			*field += declarer->Value + declBase;
			break;
		case R_386_PC32:
			*field += declarer->Value + declBase - (unsigned long) field;
			break;
		default:
			DbgLine("Error 40A: TODO:: Build code (elf-linkage-reloc-switch");
			while(TRUE){ asm volatile("nop"); }
			break;
		}
	}
	else
	{
		Dbg("\nUnresolved Symbol: ");
		if(ELF32_ST_BIND(symbolReferred->Info) == STB_WEAK)
			Dbg(" (weak) ");
		DbgLine(signature);
		if(ELF32_ST_BIND(symbolReferred->Info) == STB_WEAK)
			return;
		while(TRUE);
	}
}

///
/// Resolves all the relocations in the RelTable. This are of type "rel" and
/// have implicit addends.
///
/// @param relocTable - rel-table containing the required entries
/// @param handlerService - elf-object being referred to
/// @version 1.0
/// @since Silcos 2.05
/// @author Shukant Pal
///
void ElfLinker::resolveRelocations(RelTable &relocTable, ElfManager &handlerService)
{
	unsigned long relIndex = 0;
	unsigned long relCount = relocTable.entryCount;
	RelEntry *relDesc = relocTable.entryTable;

	while(relIndex < relCount)
	{
		ElfLinker::resolveRelocation(relDesc, handlerService);

		++(relIndex);
		++(relDesc);
	}
}
