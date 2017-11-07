/**
 * File: ElfAnalyzer.hpp
 *
 * Copyright (C) 2017 - Shukant Pal
 */

#ifndef INTERFACE_MODULE_ELFANALYZER_HPP_
#define INTERFACE_MODULE_ELFANALYZER_HPP_

#include "ELF.h"

namespace Module
{

namespace Elf
{

/**
 * Class: ElfAnalyzer
 *
 * Summary:
 * This class provides a API for accessing ELF structures. It is used by the
 * ElfLinker & ElfManager classes for getting basic services.
 *
 * Author: Shukant Pal
 */
class ElfAnalyzer
{
public:
	static BOOL validateBinary(Void *fileBinary);
	static class ElfManager *getSecureManager();

	static ULONG getSymbolHash(CHAR *symbolName);

	// Not implemented yet!
	static ULONG getGNUSymbolHash(CHAR *symbolName);
	static struct Symbol* querySymbol(CHAR *name, struct SymbolTable *symbolTable, struct HashTable *hashTable);
	static struct Symbol* querySymbol(CHAR *name, struct SymbolTable *symbolTable, struct GNUHashTable *gnuHash);
};

}

}


#endif /* INTERFACE_MODULE_ELFANALYZER_HPP_ */
