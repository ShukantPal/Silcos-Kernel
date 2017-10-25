/*=++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * File: MSI.c
 *
 * Summary:
 * This file contains the source for the 'Multiboot Section Interface' and lets
 * the kernel use its own symbol table and section headers.
 *
 * Copyright (C) 2017 - Shukant Pal
 *+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=*/

#include <Memory/KMemorySpace.h>
#include <Module/MSI.h>
#include <Util/Memory.h>
#include <Debugging.h>

KCOR_MSICACHE msiKernelSections;

MSI_SHDR *MsiFindSectionHeaderByName(CHAR *eRequiredSectionName){
	ULONG msiSectionHeaderIndex = 0;
	ULONG msiSectionHeaderCount = msiKernelSections.SectionHeaderCount;
	MSI_SHDR *msiSectionHeader = msiKernelSections.SectionHeaderTable;
	while(msiSectionHeaderIndex < msiSectionHeaderCount){
		if(strcmp(eRequiredSectionName, msiKernelSections.SectionNames + msiSectionHeader->Name))
			return (msiSectionHeader);

		++(msiSectionHeaderIndex);
		++(msiSectionHeader);
	}

	return (NULL);
}

/**
 * Function: MsiFindSectionHeaderByType
 *
 * Summary:
 * Looks for the section header by doing
 */
MSI_SHDR *MsiFindSectionHeaderByType(ULONG eRequiredSectionType){
	ULONG msiSectionHeaderIndex = 0;
	ULONG msiSectionHeaderCount = msiKernelSections.SectionHeaderCount;
	MSI_SHDR *msiSectionHeader = msiKernelSections.SectionHeaderTable;
	while(msiSectionHeaderIndex < msiSectionHeaderCount){
		if(msiSectionHeader->Type == eRequiredSectionType)
			return (msiSectionHeader);

		++(msiSectionHeaderIndex);
		++(msiSectionHeader);
	}

	return (NULL);
}

VOID MsiSetupKernelForLinking(struct ElfCache *coreCache){
	MULTIBOOT_TAG_ELF_SECTIONS *mbKernelSections = SearchMultibootTag(MULTIBOOT_TAG_TYPE_ELF_SECTIONS);
	if(mbKernelSections == NULL){
		DbgLine("MSI::MsiSetupKernelForLinking - FATAL: Section headers not found");
	} else {
		msiKernelSections.SectionHeaderCount = mbKernelSections->Number;
		msiKernelSections.SectionHeaderEntrySize = mbKernelSections->EntrySize;
		msiKernelSections.SectionNameIndex = mbKernelSections->ShstrIndex;
		msiKernelSections.SectionHeaderTable = (MSI_SHDR *) &mbKernelSections->Sections;
		msiKernelSections.SectionNames
		 = (CHAR *) MsiFindSectionHeaderByIndex(msiKernelSections.SectionNameIndex)->Address + 0xC0000000;

		struct ElfSectionHeader *msiDynsymShdr = MsiFindSectionHeaderByType(SHT_DYNSYM);
		struct ElfSectionHeader *msiDynstrShdr = MsiFindSectionHeaderByName(".dynstr");
		if(msiDynsymShdr == NULL || msiDynstrShdr == NULL){
			DbgLine("MSI::MsiSetupKernelForLinking - FATAL: Kernel symbols didn't load properly.");
		} else {
			struct ElfSymbolTable *msiCoreDynamicSymbolTbl = &coreCache->dsmTable;
			msiCoreDynamicSymbolTbl->Names = (CHAR *) msiDynstrShdr->Address;
			msiCoreDynamicSymbolTbl->EntryTable = (ELF_SYM *) msiDynsymShdr->Address;
			msiCoreDynamicSymbolTbl->EntryCount = msiDynstrShdr->Size / sizeof(ELF_SYM);
		}

		struct ElfSectionHeader *msiHashShdr = MsiFindSectionHeaderByType(SHT_HASH);
		if(msiHashShdr == NULL){
			DbgLine("MSI::MsiSetupKernelForLinking - ERROR: Kernel Symbol-Hashtable not found!");
		} else {
			struct ElfHashTable *msiHashTbl = &coreCache->dsmHash;
			ULONG *msiHashTblContents = (ULONG *) msiHashShdr->Address;
			msiHashTbl->HashSectionHdr = msiHashShdr;
			msiHashTbl->BucketEntries = *msiHashTblContents;
			msiHashTbl->ChainEntries = *(msiHashTblContents + 1);
			msiHashTbl->BucketTable = msiHashTblContents + 2;
			msiHashTbl->ChainTable = msiHashTblContents + 2 + msiHashTbl->BucketEntries;
		}

		DbgLine("MSI::MsiSetupKernelForLinking - Success");
	}
}


