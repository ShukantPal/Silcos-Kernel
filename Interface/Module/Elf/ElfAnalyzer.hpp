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
	static bool validateBinary(Void *fileBinary);
	static class ElfManager *getSecureManager();

	static unsigned long getSymbolHash(char *symbolName);

	// Not implemented yet!
	static unsigned long getGNUSymbolHash(char *symbolName);
	static struct Symbol* querySymbol(char *name, struct SymbolTable *symbolTable, struct HashTable *hashTable);
	static struct Symbol* querySymbol(char *name, struct SymbolTable *symbolTable, struct GNUHashTable *gnuHash);
};

}

}


#endif /* INTERFACE_MODULE_ELFANALYZER_HPP_ */
