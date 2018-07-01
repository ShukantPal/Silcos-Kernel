/**
 * @file KernelModule.h
 *
 * Provides a interface to access and cache data present in ELF-objects
 * related to their relocation.
 *
 * This is a special "header" file. It should be included in other C++
 * modules very carefully, as it uses the Module::Elf namespace.
 *
 *  Created on: 16-Jun-2018
 *      Author: sukantdev
 */
#ifndef INITIALIZER_GENERIC_KERNELMODULE_H_
#define INITIALIZER_GENERIC_KERNELMODULE_H_

#include <Module/Elf/ELF.h>

#ifndef _CBUILD
	using namespace Module::Elf;
#endif

struct KernelModule
{
	union {
		struct ElfHeader *fileHdr;//!< ELF-header pointer in the object-file
		unsigned long fileBase;//!< Base physical address of the object-file
	};

	unsigned long realBase;
	unsigned long virtualBase;
	unsigned long linkerBase;
	unsigned long orderIndex;

	struct {
		struct ProgramHeader *segments;
		unsigned long segEntryCount;
		unsigned long segEntrySize;
	};

	struct {
		struct DynamicEntry *linkerTable;
	};

	struct RelocationTable rel;
	struct RelocationTable rela;
	struct RelocationTable pltrel;

	struct SymbolTable dynSyms;
	struct HashTable dynHash;
};

struct KernelSymbol
{
	struct KernelSymbol *next;
	struct KernelSymbol *last;
	unsigned long ownerIndex;
	unsigned long virtualAddress;
	char *name;
};

#ifdef _CBUILD

struct ProgramHeader *SearchSegment(struct KernelModule *ldrObj,
		enum PhdrType typeRequired);

struct DynamicEntry *SearchLinkerTag(struct KernelModule *ldrObj,
		enum DynamicTag tagRequired);

void ResolveRel(struct RelEntry *rel, struct KernelModule *ldrObj);
void ResolveRela(struct RelaEntry *rela, struct KernelModule *ldrObj);
void ResolveAllRel(struct RelEntry *rel, struct KernelModule *ldrObj,
		unsigned long count);
void ResolveAllRela(struct RelaEntry *rela, struct KernelModule *ldrObj,
		unsigned long count);

static inline unsigned long HashString(char *str)
{
	unsigned long hashKey = 0, testHolder;

	while(*str)
	{
		hashKey = (hashKey << 4) + *str++;
		testHolder = hashKey & 0xF0000000;
		if(testHolder)
			hashKey ^= testHolder >> 24;
		hashKey &= ~testHolder;
	}

	return (hashKey);
}
#endif

#endif/* Generic/KernelModule.h */
