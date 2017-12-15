/**
 * File: MSI.h
 *
 * Summary:
 *
 *
 * Copyright (C) 2017 - Shukant Pal
 */
#ifndef __INTERFACE_MODULE_MSI_H__
#define __INTERFACE_MODULE_MSI_H__

#include "Elf/ELF.h"
#include <Multiboot2.h>

using namespace Module::Elf;

typedef struct SectionHeader MSI_SHDR;

/**
 * Type: KCOR_MSICACHE
 *
 * Summary:
 * This type caches the 'limited' kernel elf-binary information present in the
 * multiboot information table. It is used for allowing the core to be present
 * as a module-dependency in the KMCF.
 *
 * @See "ModuleLoader.h"
 * @Version 1.1
 * @Since Circuit 2.03
 * @Author Shukant Pal
 */
typedef
struct MultibootElfCache {
	U32 SectionHeaderCount;
	U32 SectionHeaderEntrySize;
	U32 SectionNameIndex;
	char *SectionNames;
	MSI_SHDR *SectionHeaderTable;
	struct {
		char *DynamicSymbolNames;
		struct Symbol *DynamicSymbolTable;
		unsigned long DynamicSymbolCount;
	};
	struct SymbolTable eSymbolTbl;
	struct HashTable eSymbolHashTbl;
} KCOR_MSICACHE;

extern KCOR_MSICACHE msiKernelSections;

/**
 * Class: KernelElf
 *
 * Summary:
 * This class manages the boot-time module loading & also abstracts the microkernel
 * elf-binary, which is special due to the absence of its elf-header.
 *
 * Functions:
 * getDynamicEntry - Searches for a microkernel dynamic entry
 * registerDynamicLink - Registers the microkernel dynamic-link info
 * loadBootModules - Loads all the boot-time modules (called only once)
 *
 * Origin:
 * This is the modified version of the 'Multiboot-Section Interface' which abstracted
 * multiboot-sections, which has been deprecated due to use of symbols to find the
 * dynamic table.
 *
 * Version: 1.1
 * Since: Circuit 2.03++
 * Author: Shukant Pal
 */
class KernelElf
{
public:
	static DynamicEntry *getDynamicEntry(DynamicTag tag);
	static ModuleRecord *registerDynamicLink();
	static void loadBootModules();
};

void MsiSetupKernelForLinking(
		struct ElfCache *coreCache
);
  
#endif/* Module/MSI.h */
