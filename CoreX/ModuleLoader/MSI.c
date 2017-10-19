/*=++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * File: MSI.c
 *
 * Summary:
 * This file contains the source for the 'Multiboot Section Interface' and lets
 * the kernel use its own symbol table and section headers.
 *
 * Copyright (C) 2017 - Shukant Pal
 *+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=*/

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

	return (msiSectionHeader);
}

ELF_SYM *MsiFindSymbolByName(CHAR *eRequiredSymbolName){

}

static
void __msi_show_all_shdrs(){
	ULONG msi_shdr_idx = 0;
	ULONG msi_shdr_cnt = msiKernelSections.SectionHeaderCount;
	ELF_SHDR *msi_shdr = msiKernelSections.SectionHeaderTable;
	while(msi_shdr_idx < msi_shdr_cnt){
		Dbg("msi_shdr: "); Dbg(msiKernelSections.SectionNames + msi_shdr->Name); Dbg(";");
		++msi_shdr_idx;++msi_shdr;
	}
}

static
void __msi_show_all_dynsyms(){
	ULONG msi_sym_index = 0;
	ULONG msi_sym_count = msiKernelSections.DynamicSymbolCount;
	ELF_SYM *msi_sym = msiKernelSections.DynamicSymbolTable;
	while(msi_sym_index < msi_sym_count){
		Dbg("msi_dynsym: "); Dbg(msi_sym->Name + msiKernelSections.DynamicSymbolNames); DbgLine(";");
		++msi_sym; ++msi_sym_index;
	}
}

VOID MsiSetupKernelForLinking(){
	MULTIBOOT_TAG_ELF_SECTIONS *mbKernelSections = SearchMultibootTag(MULTIBOOT_TAG_TYPE_ELF_SECTIONS);
	if(mbKernelSections == NULL)
		DbgLine("__err Module::MSI::MsiSetupKernelForLinking #ERR - no mb-tag present for elf sections");
	else {
		msiKernelSections.SectionHeaderCount = mbKernelSections->Number;
		msiKernelSections.SectionHeaderEntrySize = mbKernelSections->EntrySize;
		msiKernelSections.SectionNameIndex = mbKernelSections->ShstrIndex;
		msiKernelSections.SectionHeaderTable = (MSI_SHDR *) &mbKernelSections->Sections;
		msiKernelSections.SectionNames = (CHAR *) MsiFindSectionHeaderByIndex(msiKernelSections.SectionNameIndex)->Address + 0xC0000000;

		DbgInt(msiKernelSections.SectionNameIndex);

		MSI_SHDR *msiDynsymSectionHdr = MsiFindSectionHeaderByType(SHT_SYMTAB);
		MSI_SHDR *msiDynsymNameSectionHdr = MsiFindSectionHeaderByName(".strtab");
		if(msiDynsymSectionHdr == NULL || msiDynsymNameSectionHdr == NULL){
			DbgLine("Module::MSI::MsiSetupKernelForLinking - Dynamic Symbols Not Found!");
		} else {
			msiKernelSections.DynamicSymbolNames = (CHAR *) ((ULONG)0xC0000000 + msiDynsymNameSectionHdr->Address);
			msiKernelSections.DynamicSymbolCount = msiDynsymSectionHdr->Size / sizeof(ELF_SYM);
			msiKernelSections.DynamicSymbolTable = (ELF_SYM *) ((ULONG) 0xC0000000 + msiDynsymSectionHdr->Address);

			Dbg("-msi-show-all-dynsyms");
			__msi_show_all_dynsyms();
		}

		DbgLine("Module::MSI::MsiSetupKernelForLinking - MSI Kernel Sections were found & loaded.");
	}
}


