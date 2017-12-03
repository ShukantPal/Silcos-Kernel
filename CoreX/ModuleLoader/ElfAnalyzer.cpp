/**
 * File: ElfAnalyzer.cpp
 *
 * Copyright (C) 2017 - sukantdev
 */

#include <Module/Elf/ElfAnalyzer.hpp>
#include <Util/Memory.h>

using namespace Module;
using namespace Module::Elf;

BOOL ElfAnalyzer::validateBinary(
		Void *binaryFile
){
	struct ElfHeader *analyzedHeader = (struct ElfHeader *) binaryFile;
	if(analyzedHeader->fileIdentifier[EI_MAG0] != ELFMAG0) return (FALSE);
	if(analyzedHeader->fileIdentifier[EI_MAG1] != ELFMAG1) return (FALSE);
	if(analyzedHeader->fileIdentifier[EI_MAG2] != ELFMAG2) return (FALSE);
	if(analyzedHeader->fileIdentifier[EI_MAG3] != ELFMAG3) return (FALSE);

	return (TRUE);

	if(analyzedHeader->platformRequired != EM_RUNNER)
		return (FALSE);/* Check platform-correctness */

	//	if(analyzedHeader->Type != ET_DYN)
	//		return (FALSE);/* KModules are always a relocatable */

	return (TRUE);
}

ULONG ElfAnalyzer::getSymbolHash(
		CHAR *symbolName
){
	ULONG hashKey = 0, testHolder;
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
struct Symbol *ElfAnalyzer::querySymbol(
		CHAR *requiredSymbolName,
		struct SymbolTable *symbolRecord,
		struct HashTable *hashTable
){
	ULONG hashKey = ElfAnalyzer::getSymbolHash(requiredSymbolName);
	ULONG chainIndex = hashTable->bucketTable[hashKey % hashTable->bucketEntries];
	ULONG *chainEntry;
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
