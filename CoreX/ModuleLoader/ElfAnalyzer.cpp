/**
 * File: ElfAnalyzer.cpp
 *
 * Copyright (C) 2017 - sukantdev
 */

#include <Module/Elf/ElfAnalyzer.hpp>
#include <Util/Memory.h>

using namespace Module;
using namespace Module::Elf;

bool ElfAnalyzer::validateBinary(void *binaryFile){
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

unsigned long ElfAnalyzer::getSymbolHash(char *symbolName)
{
	unsigned long hashKey = 0, testHolder;
	while(*symbolName){
		hashKey = (hashKey << 4) + *symbolName++;
		testHolder = hashKey & 0xF0000000;
		if(testHolder)
			hashKey ^= testHolder >> 24;
		hashKey &= ~testHolder;
	}

	return (hashKey);
}

#include <KERNEL.h>
Symbol *ElfAnalyzer::querySymbol(char *requiredSymbolName, SymbolTable *symbolRecord, HashTable *hashTable)
{
	unsigned long hashKey = ElfAnalyzer::getSymbolHash(requiredSymbolName);
	unsigned long chainIndex = hashTable->bucketTable[hashKey % hashTable->bucketEntries];
	unsigned long *chainEntry;
	Symbol *relevantSymbol;
	do {
		chainEntry = hashTable->chainTable + chainIndex;
		relevantSymbol = symbolRecord->entryTable + chainIndex;

		if(strcmp(requiredSymbolName, symbolRecord->nameTable + relevantSymbol->Name))
			return (relevantSymbol);
		else
			chainIndex = *chainEntry;
	} while(chainIndex != STN_UNDEF);

	return (NULL);
}
