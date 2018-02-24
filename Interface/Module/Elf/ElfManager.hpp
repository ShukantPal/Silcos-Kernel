///
/// @file ElfManager.hpp
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
#ifndef _INTERFACE_MODULE_ELFMANAGER_HPP_
#define _INTERFACE_MODULE_ELFMANAGER_HPP_

#include "ELF.h"
#include "ElfAnalyzer.hpp"
#include <Memory/Pager.h>

namespace Module
{

struct DynamicLink;
class ModuleLoader;

namespace Elf
{

/**
 * Class: ElfManager
 *
 * Summary:
 * This class is used for loading elf-binaries into kernel space. It will load
 * program segments into memory during initialization.
 *
 * After initialization, this object is of little use, other than exporting its
 * dynamic linking information (see getDynamicLink()). Unless specified by the
 * module by the moduleInfo struct, this object will be destroyed by the dynamic
 * loader. It can be used for getting internal information by the module, and
 * is not for any linking purpose. Infact, ElfAnalyzer provides the back-end
 * services for it and ElfLinker support allows linkage with the kernel.
 *
 * On destruction, the file for the module is also freed.
 *
 * NOTE:
 * All operations are done w.r.t program segment (except a few)
 *
 * Important Functions:
 * getSymbol - Used for getting a dynamic symbol by its name or type
 * getStaticSymbol - Used for getting a symbol from the symtab section
 * getDynamicEntry - Used for getting entry into the dynamic table by tag
 *
 * Version: 1.1
 * Since: Circuit 2.03,++
 * Author: Shukant Pal
 */
class ElfManager
{
public:
	ElfManager();
	ElfManager(ElfHeader *binaryHeader);
	~ElfManager();

	inline unsigned long base(){ return (baseAddress); }
	inline SymbolTable *getSymbolTable(){ return (&dynamicSymbols); }
	inline Symbol *getSymbol(char *symName)
	{
		return (ElfAnalyzer::querySymbol(symName, &dynamicSymbols,
							&dynamicHash));
	}
	inline SymbolTable *getStaticSymbolTable(){ return (staticSymbols); }

	Symbol *getStaticSymbol(const char *symName);
	Symbol *getStaticSymbol(unsigned long symIdx);
	inline ProgramHeader *getPhdrTable(){ return (phdrTable); }
	ProgramHeader *getProgramHeader(enum PhdrType type);
	inline ProgramHeader *getProgramHeader(unsigned long phdrIdx){ return (phdrTable + phdrIdx); }
	inline SectionHeader *getShdrTable(){ return (shdrTable); }
	SectionHeader *getSectionHeader(enum SectionType type);
	SectionHeader *getSectionHeader(unsigned long shdrIdx);
	DynamicEntry *getDynamicEntry(enum DynamicTag tag);
	static unsigned long getSymbolHash(char &symName);
	static unsigned long getGNUSymbolHash(char &symName);
	DynamicLink *exportDynamicLink();
private:
	// Elf-header present in the file. This exists in the file-only, and
	// may/not be loaded into program segments. Thus, ElfManger should be
	// destroyed before unloading the raw-binary file.
	ElfHeader *binaryHeader;

	// Program-headers MUST be present. If someone malicious tries to
	// load a misleading module, it will be NULL. All operations will
	// fail, if NULL, but not fault.
	ProgramHeader *phdrTable;
	unsigned long phdrCount;
	unsigned long phdrSize;

	// If the section-table is loaded into the binary alongside then,
	// this will exist otherwise 0
	SectionHeader *shdrTable;
	unsigned long shdrCount;
	unsigned long shdrSize;

	// Dynamic segment pointers
	DynamicEntry *dynamicTable;
	unsigned long dynamicEntryCount;

	// Dynamic & static symbol tables
	SymbolTable dynamicSymbols;
	HashTable dynamicHash;
	SymbolTable *staticSymbols;

	// Relocation tables
	RelTable relTable;
	RelaTable relaTable;
	RelocationTable pltRelocTable;

	// Values related to the program loaded into memory during construction
	// of ElfManager. If no program segments are found, then it will be 0.
	unsigned long pageCount;
	unsigned long baseAddress;
	PADDRESS loadAddress;

	void fillBlankDsm();
	inline void fillBlankRel(){ relTable.entryCount = 0; }
	inline void fillBlankRela(){ relaTable.entryCount = 0; }
	void loadBinary(unsigned long address);
	static unsigned long getLimitAddress(class ElfManager *mgr);

	friend class ElfAnalyzer;
	friend class ElfLinker;
	friend class Module::ModuleLoader;
};

}// namespace Elf
}// namespace Module

#endif/* Module/ElfManager.hpp */
