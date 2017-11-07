/**
 * File: ElfManager.hpp
 *
 * Copyright (C) 2017 - sukantdev
 */

#ifndef _INTERFACE_MODULE_ELFMANAGER_HPP_
#define _INTERFACE_MODULE_ELFMANAGER_HPP_

#include "ELF.h"
#include "ElfAnalyzer.hpp"
#include <Memory/Pager.h>

namespace Module
{

struct DynamicLink;
class ModuleLoader;

namespace Elf {

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
	ElfManager(struct ElfHeader *binaryHeader);
	~ElfManager();

	inline struct SymbolTable *getSymbolTable(){
		return (&dynamicSymbols);
	}

	inline struct Symbol *getSymbol(CHAR *symName){
		return ElfAnalyzer::querySymbol(symName, &dynamicSymbols, &dynamicHash);
	}

	inline struct SymbolTable *getStaticSymbolTable(){
		return (staticSymbols);
	}

	struct Symbol *getStaticSymbol(CHAR *symName);
	struct Symbol *getStaticSymbol(ULONG symIdx);

	inline struct ProgramHeader *getPhdrTable(){ return (phdrTable); }
	struct ProgramHeader *getProgramHeader(enum PhdrType type);
	inline struct ProgramHeader *getProgramHeader(ULONG phdrIdx){ return (phdrTable + phdrIdx); }
	inline struct SectionHeader *getShdrTable(){ return (shdrTable); }
	struct SectionHeader *getSectionHeader(enum SectionType type);
	struct SectionHeader *getSectionHeader(ULONG shdrIdx);
	struct DynamicEntry *getDynamicEntry(enum DynamicTag tag);

	static ULONG getSymbolHash(CHAR &symName);
	static ULONG getGNUSymbolHash(CHAR &symName);

	struct DynamicLink *exportDynamicLink();
private:
	// Elf-header present in the file. This exists in the file-only, and
	// may/not be loaded into program segments. Thus, ElfManger should be
	// destroyed before unloading the raw-binary file.
	struct ElfHeader *binaryHeader;

	// Program-headers MUST be present. If someone malicious tries to
	// load a misleading module, it will be NULL. All operations will
	// fail, if NULL, but not fault.
	struct ProgramHeader *phdrTable;
	ULONG phdrCount;
	ULONG phdrSize;

	// If the section-table is loaded into the binary alongside then,
	// this will exist otherwise 0
	struct SectionHeader *shdrTable;
	ULONG shdrCount;
	ULONG shdrSize;

	// Dynamic segment pointers
	struct DynamicEntry *dynamicTable;
	ULONG dynamicEntryCount;

	// Dynamic & static symbol tables
	struct SymbolTable dynamicSymbols;
	struct HashTable dynamicHash;
	struct SymbolTable *staticSymbols;

	// Relocation tables
	struct RelTable relTable;
	struct RelaTable relaTable;
	struct RelocationTable pltRelocTable;

	// Values related to the program loaded into memory during construction
	// of ElfManager. If no program segments are found, then it will be 0.
	unsigned long pageCount;
	unsigned long baseAddress;
	PADDRESS loadAddress;

	void fillBlankDsm();

	inline void fillBlankRel(){ relTable.entryCount = 0; }
	inline void fillBlankRela(){ relaTable.entryCount = 0; }

	void loadBinary(ULONG address);

	static ULONG getLimitAddress(class ElfManager *mgr);

	friend class ElfAnalyzer;
	friend class ElfLinker;
	friend class Module::ModuleLoader;
};

} // Elf

} // Module

#endif/* Module/ElfManager.hpp */
