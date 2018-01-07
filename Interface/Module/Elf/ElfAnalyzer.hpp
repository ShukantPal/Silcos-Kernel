/**
 * File: ElfAnalyzer.hpp
 *
 * Copyright (C) 2017 - Shukant Pal
 */

#ifndef INTERFACE_MODULE_ELFANALYZER_HPP_
#define INTERFACE_MODULE_ELFANALYZER_HPP_

#include "ELF.h"
#include <KERNEL.h>

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
	static bool validateBinary(Void *fileBinary) kxhide;
	static class ElfManager *getSecureManager() kxhide;

	static unsigned long getSymbolHash(char *symbolName) kxhide;

	// Not implemented yet!
	static unsigned long getGNUSymbolHash(char *symbolName) kxhide;
	static struct Symbol* querySymbol(char *name, struct SymbolTable *symbolTable, struct HashTable *hashTable) kxhide;
	static struct Symbol* querySymbol(char *name, struct SymbolTable *symbolTable, struct GNUHashTable *gnuHash) kxhide;
};

}

}


#endif /* INTERFACE_MODULE_ELFANALYZER_HPP_ */
