/*=+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * File: MSI.h
 *
 * Summary:
 * MSI is the acronym for 'Multiboot Section Interface'. It is used by the core
 * kernel to link its own symbols with the kmodules. This separate interface is
 * provided, because the multiboot-compliant boot-loader may/may not load the ELF
 * header. But, the multiboot specification provides the multiboot-section tags
 * for the ELF-kernel binaries.
 *  
 * MSI is just an interface to use the multiboot-section tags and load the core
 * kernel symbol table.
 *
 * Copyright (C) 2017 - Shukant Pal
 *+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
#ifndef __MODULE_MSI_H__
#define __MODULE_MSI_H__

#include "ELF.h"
#include <Multiboot2.h>

typedef ELF_SHDR MSI_SHDR;

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
typedef struct {
	U32 SectionHeaderCount;
	U32 SectionHeaderEntrySize;
	U32 SectionNameIndex;
	CHAR *SectionNames;
	MSI_SHDR *SectionHeaderTable;
	struct {
		CHAR *DynamicSymbolNames;
		ELF_SYM *DynamicSymbolTable;
		ULONG DynamicSymbolCount;
	};
	struct {
		CHAR *SymbolNames;
		ELF_SYM *SymbolTable;
		ULONG SymbolCount;
	};
} KCOR_MSICACHE;

extern KCOR_MSICACHE msiKernelSections;

MSI_SHDR *MsiFindSectionHeaderByName(
	CHAR *eRequiredSectionName
);

static inline
MSI_SHDR *MsiFindSectionHeaderByIndex(ULONG msiRequiredSectionIndex){
	return (msiKernelSections.SectionHeaderTable + msiRequiredSectionIndex);
}

MSI_SHDR *MsiFindSectionHeaderByType(
	ULONG eRequiredSectionType
);

ELF_SYM *MsiFindSymbolByName(
	CHAR *
);

VOID MsiSetupKernelForLinking(
		VOID
);
  
#endif/* Module/MSI.h */
