/**
 * @file ElfAnalyzer.cpp
 * @module KernelHost
 *
 * Implements the utility function to access and manipulate various
 * features of an elf-object.
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
#include <Module/Elf/ElfAnalyzer.hpp>
#include <Utils/Memory.h>
#include <KERNEL.h>

using namespace Module;
using namespace Module::Elf;

/**
 * Validates the binary-file present in kernel-memory as an elf-object,
 * which must be a "shared" library.
 *
 * @param binaryFile - file of the module loaded from secondary
 * 			storage
 * @return - whether or not the module is a elf-object
 */
bool ElfAnalyzer::validateBinary(void *binaryFile)
{
	ElfHeader *analyzedHeader = (ElfHeader *) binaryFile;

	if(analyzedHeader->fileIdentifier[EI_MAG0] != ELFMAG0) return (false);
	if(analyzedHeader->fileIdentifier[EI_MAG1] != ELFMAG1) return (false);
	if(analyzedHeader->fileIdentifier[EI_MAG2] != ELFMAG2) return (false);
	if(analyzedHeader->fileIdentifier[EI_MAG3] != ELFMAG3) return (false);

	return (true);

	if(analyzedHeader->platformRequired != EM_RUNNER)
		return (false);/* Check platform-correctness */

	if(analyzedHeader->fileType != ET_DYN)
		return (false);/* KModules are always a shared library */

	return (true);
}

/**
 * Searches for an linker/dynamic entry of the given tag, serially in
 * the table given.
 *
 * @param tag - the type of required entry
 * @param ent - table of entries
 * @return
 */
DynamicEntry *ElfAnalyzer::getLinkerEntry(DynamicTag tag, DynamicEntry *ent)
{
	while(ent->tag != DT_NULL) {
		if(ent->tag == tag) {
			return (ent);
		}
		++(ent);
	}

	return (null);
}

/**
 * Returns the hash value of the string given using the ELF-defined
 * algorithm.
 *
 * @param symbolName - name of the symbol/string to be hashed
 */
unsigned long ElfAnalyzer::getSymbolHash(char *symbolName)
{
	unsigned long hashKey = 0, testHolder;
	
	while(*symbolName)
	{
		hashKey = (hashKey << 4) + *symbolName++;
		testHolder = hashKey & 0xF0000000;
		if(testHolder)
			hashKey ^= testHolder >> 24;
		hashKey &= ~testHolder;
	}

	return (hashKey);
}

/**
 * Searches for the symbol with the given string as its name. It subsequently
 * takes help of the "standard" ELF hash-table present in the elf-object and
 * a symbol-table which holds the symbol-entry.
 *
 * @param requiredSymbolName - name of the symbol being searched for
 * @param symbolRecord - table of symbols in which it resides
 * @param hashTable - "standard" ELF hash-table of symbols
 * @return - the symbol entry having the given name, present in the
 * 		symbol table given
 * @version 1.0
 * @since Circuit 2.03
 * @author Shukant Pal
 */
Symbol *ElfAnalyzer::querySymbol(char *requiredSymbolName,
		SymbolTable *symbolRecord, HashTable *hashTable)
{
	unsigned long hashKey = ElfAnalyzer::getSymbolHash(requiredSymbolName);
	unsigned long chainIndex = hashTable->bucketTable
			[hashKey % hashTable->bucketEntries];
	unsigned long *chainEntry;
	Symbol *relevantSymbol;

	do {
		chainEntry = hashTable->chainTable + chainIndex;
		relevantSymbol = symbolRecord->entryTable + chainIndex;

		if(strcmp(requiredSymbolName, symbolRecord->nameTable +
				relevantSymbol->name))
			return (relevantSymbol);
		else
			chainIndex = *chainEntry;
	} while(chainIndex != STN_UNDEF);

	return (NULL);
}
