#define AI_VMM
#include <Module/ELF.h>
#include <Module/ModuleRecord.h>
#include <KERNEL.h>

BOOL MdCheckELFCompat(ELF32_EHDR *eHeader){
	if(eHeader->Identifier[EI_MAG0] != ELFMAG0) return (FALSE);
return (TRUE);
	if(eHeader->Identifier[EI_MAG1] != ELFMAG1) return (FALSE);
	if(eHeader->Identifier[EI_MAG2] != ELFMAG2) return (FALSE);
	if(eHeader->Identifier[EI_MAG3] != ELFMAG3) return (FALSE);

	if(eHeader->Machine != EM_RUNNER)
		return (FALSE);/* Check platform-correctness */
		
//	if(eHeader->Type != ET_DYN)
//		return (FALSE);/* KModules are always a relocatable */

	return (TRUE);
}

static inline
CHAR *EFindSectionNames(ELF_EHDR *eHeader){
	ELF_SHDR *eSectionHeader = (ELF_SHDR *) ((ADDRESS) eHeader + eHeader->SectionHeaderOffset);
	eSectionHeader += eHeader->SectionStringIndex;
	
	CHAR *eSectionNames = (CHAR *) ((ADDRESS) eHeader + eSectionHeader->Offset);
	return (eSectionNames);
}

static inline ELF_SHDR *EFindSectionByIndex(ULONG eRequiredSectionIndex, ELF_EHDR *eHeader){
	return ((ELF_SHDR *) ((ADDRESS) eHeader + eHeader->SectionHeaderOffset)) + eRequiredSectionIndex;
}

static ELF_SHDR *EFindSectionByName(CHAR *eSectionName, ELF_EHDR *eHeader){
	CHAR *eSectionNames = EFindSectionNames(eHeader);

	ULONG eSectionIndex = 0;
	ULONG eSectionCount = eHeader->SectionHeaderEntryCount;
	ELF_SHDR *eSectionHeader = (ELF_SHDR *) ((ADDRESS) eHeader + eHeader->SectionHeaderOffset);
	while(eSectionIndex < eSectionCount){
		if(strcmp(eSectionName, eSectionNames + eSectionHeader->Name))
			return (eSectionHeader);
		++(eSectionIndex);
		++(eSectionHeader);
	}
	
	return (NULL);
}

static ELF_SHDR *EFindSectionHeaderByType(ELF_SHT eRequiredSectionType, ELF_SHDR *eSectionHeader, ELF_EHDR *eHeader){
	ULONG eSectionHeaderIndex;
	ULONG eSectionHeaderCount = eHeader->SectionHeaderEntryCount;

	if(eSectionHeader == NULL){
		eSectionHeader = (ELF_SHDR *) ((ADDRESS) eHeader + eHeader->SectionHeaderOffset);
		eSectionHeaderIndex = 0;
	} else {
		eSectionHeaderIndex = ((ADDRESS) eSectionHeader - (ADDRESS) eHeader - eHeader->SectionHeaderOffset) / sizeof(ELF_SHDR);
		++(eSectionHeader);
	}
	
	while(eSectionHeaderIndex < eSectionHeaderCount){
		if(eSectionHeader->Type == eRequiredSectionType)
			return (eSectionHeader);
		++(eSectionHeaderIndex);
		++(eSectionHeader);
	}

	return (NULL);
}

// -- ELF Program Header===========================================================================================

static ELF_PHDR *EFindProgramHeaderByType(ELF_PT eRequiredProgramHeaderType, ELF_PHDR *eProgramHeader, ELF_EHDR *eHeader){
	ULONG eProgramHeaderIndex;
	ULONG eProgramHeaderCount = eHeader->ProgramHeaderEntryCount;
	
	if(eProgramHeader == NULL){
		eProgramHeader = (ELF_PHDR *) ((ADDRESS) eHeader + eHeader->ProgramHeaderOffset);
		eProgramHeaderIndex = 0;
	} else {
		eProgramHeaderIndex = ((ADDRESS) eProgramHeader - (ADDRESS) eHeader - eHeader->ProgramHeaderOffset) / sizeof(ELF_PHDR);
	}
	
	while(eProgramHeaderIndex < eProgramHeaderCount){
		if(eProgramHeader->Type == eRequiredProgramHeaderType)
			return (eProgramHeader);
			
		++(eProgramHeaderIndex);
		++(eProgramHeader);
	}

	return (NULL);
}

//=================================================== ELF Symbols ======================================================

static ELF_SYM *EFindSymbolByName(CHAR *eRequiredSymbolName, KMOD_ECACHE *eCache){
	ULONG eSymbolIndex = 0;
	ULONG eSymbolCount = eCache->eSymbolTableLength;
	ELF_SYM *eSymbol = eCache->eSymbolTable;
	CHAR *eSymbolNames = eCache->eSymbolNames;

	while(eSymbolIndex < eSymbolCount){
		if(strcmp(eRequiredSymbolName, eSymbolNames + eSymbol->Name))
			return (eSymbol);

		++(eSymbolIndex);
		++(eSymbol);
	}

	return (NULL);
}

static inline ELF_SYM *EFindSymbolByIndex(ULONG eRequiredSymbolIndex, KMOD_ECACHE *eCache){
	return (eCache->eSymbolTable + eRequiredSymbolIndex);
}

static ELF_SHDR *EFillSymbolTable(ELF_EHDR *eHeader){	
	CHAR *eSymbolNames = (CHAR *) ((ADDRESS) eHeader + EFindSectionByName(".strtab", eHeader)->Offset);
	if(eSymbolNames == NULL) return (NULL);

	ELF_SHDR *eDynamicSymbolTable = EFindSectionHeaderByType(SHT_SYMTAB, NULL, eHeader);
	ULONG eSymbolIndex = 0;
	ULONG eSymbolCount = eDynamicSymbolTable->Size / sizeof(ELF_SYM);
	ELF_SYM *eSymbol = (ELF_SYM *) ((ADDRESS) eHeader + eDynamicSymbolTable->Offset);
	while(eSymbolIndex < eSymbolCount){
	if(eSymbol->Name != 0){
		Dbg(eSymbolNames + eSymbol->Name); Dbg(", Size:"); DbgInt(eSymbol->Size); Dbg(", isNeeded:"); DbgInt(ELF32_ST_BIND(eSymbol->Info) == STB_GLOBAL); Dbg(", Val:"); DbgInt(eSymbol->Value); DbgLine(";");
	}	++(eSymbol);
		++(eSymbolIndex);
	}

	return (NULL);
}

//================================================= DYNAMIC Section ====================================================

static ELF_DYN *EFindDyn(DTAG requiredDynTag, KMOD_ECACHE *eCache){
	ULONG eDynIndex = 0;
	ULONG eDynCount = eCache->eDynTableLength;
	ELF_DYN *eDyn = eCache->_DYNAMIC;
	
	while(eDynIndex < eDynCount){
	DbgInt(eDyn->Tag); Dbg(" ");
		if(eDyn->Tag == requiredDynTag)
			return (eDyn);
		++(eDynIndex);
		++(eDyn);
	}

	return (NULL);
}

//=============================================== ELF Relocation =======================================================

static VOID EPerformRel(ELF_REL *eRel, KMOD_ECACHE *eCache){
	ELF_EHDR *eHeader = eCache->eHeader;
	ELF_SYM *eRelSymbol = EFindSymbolByIndex(ELF32_R_SYM(eRel->Info), eCache);
	ULONG *eRelLocation; extern void elf_dbg();
	switch(ELF32_R_TYPE(eRel->Info)){
		case R_386_JMP_SLOT:
			Dbg("REL_FOUND _JMP");
			eRelLocation = ((ADDRESS) eHeader + eRel->Offset);
			if(ELF32_ST_TYPE(eRelSymbol->Info) == STT_SECTION){
				ELF_SHDR *eRelSection = EFindSectionByIndex(eRelSymbol->SectionIndex, eCache->eHeader);
				eRelLocation += eRelSection->Offset/4;
				DbgInt(eRel->Offset + eRelSection->Offset); Dbg(" "); Dbg(eCache->eSectionNames + eRelSection->Name);
			}
			*eRelLocation = &elf_dbg;
			Dbg("__write_over");
			eRelSymbol->Value = &elf_dbg;
			break;
		default:
			Dbg("NO_REL FOUND");
			break;
	}
}

static VOID EPerformRela(ELF_RELA *eRela, KMOD_ECACHE *eCache){
	Dbg("__rela");
}

static VOID EFillRelocations(KMOD_ECACHE *eCache){
	ELF_EHDR *eHeader = eCache->eHeader;
	ELF_SHDR *eRelocationSection = EFindSectionHeaderByType(SHT_REL, NULL, eHeader);
	extern void elf_dbg();
	ULONG eRelIndex;
	ULONG eRelCount;
	ELF_REL *eRel;
	Dbg("_D");
	while(eRelocationSection != NULL){
		eRelIndex = 0;
		eRelCount = eRelocationSection->Size / sizeof(ELF_REL);
		DbgInt(eRelCount);
		eRel = (ELF_REL *) ((ADDRESS) eHeader + eRelocationSection->Offset);
		while(eRelIndex < eRelCount){
		Dbg("__D");
			EPerformRel(eRel, eCache);
			++(eRelIndex);
			++(eRel);
		}
		eRelocationSection = EFindSectionHeaderByType(SHT_REL, eRelocationSection, eHeader);
	}
}

static VOID EParseRela(KMOD_ECACHE *eCache){
	ELF_SHDR *eRelaSection = EFindSectionHeaderByType(SHT_RELA, NULL, eCache->eHeader);
	extern void elf_dbg();
	ULONG eRelaIndex;
	ULONG eRelaCount;
	ELF_RELA *eRela;
	while(eRelaSection != NULL){
		eRelaIndex = 0;
		eRelaCount = eRelaSection->Size / sizeof(ELF_RELA);
		eRela = (ELF_RELA *) ((ADDRESS) eCache->eHeader + eRelaSection->Offset);
		while(eRelaIndex < eRelaCount){
			Dbg("_rela");
			EPerformRela(eRela, eCache);
			++(eRelaIndex);
			++(eRela);
		}

		eRelaSection = EFindSectionHeaderByType(SHT_RELA, eRelaSection, eCache->eHeader);
	}
}

//=================================================== ELF CACHE ========================================================

VOID ELoadCache(KMOD_ECACHE *kmCache, ELF_EHDR *eHeader){
	kmCache->eSectionNames = EFindSectionNames(eHeader);

	ELF_SHDR *eSymtabSection = EFindSectionHeaderByType(SHT_SYMTAB, NULL, eHeader);
	kmCache->eSymbolNames = (CHAR *) ((ADDRESS) eHeader + EFindSectionByName(".strtab", eHeader)->Offset);
	kmCache->eSymbolTable = (ELF_SYM *) ((ADDRESS) eHeader + eSymtabSection->Offset);
	kmCache->eSymbolTableLength = eSymtabSection->Size / sizeof(ELF_SYM);

	ELF_SHDR *eDynsymSection = EFindSectionHeaderByType(SHT_DYNSYM, NULL, eHeader);
	kmCache->eDynamicSymbolNames = (CHAR *) ((ADDRESS) eHeader + EFindSectionByName(".dynstr", eHeader));
	kmCache->eDynamicSymbolTable = (ELF_SYM *) ((ADDRESS) eHeader + eDynsymSection->Offset);
	kmCache->eDynamicSymbolTableLength = eDynsymSection->Size / sizeof(ELF_SYM);

	ELF_SHDR *eDynamicSection = EFindSectionHeaderByType(SHT_DYNAMIC, NULL, eHeader);
	kmCache->_DYNAMIC = (ELF32_DYN *) ((ADDRESS) eHeader + eDynamicSection->Offset);
	kmCache->eDynTableLength = eDynamicSection->Size / sizeof(DTAG);
	
	kmCache->eRelaTable = NULL;
	kmCache->eRelaCount = 0;
	kmCache->eRelTable = NULL;
	kmCache->eRelCount = 0;

	kmCache->eHeader = eHeader;
}

static
void __show_all_sections(ELF_EHDR *eHeader, KMOD_ECACHE *eCache){
	ULONG index = 0;
	ULONG count = eHeader->SectionHeaderEntryCount;
	ELF_SHDR *section = (ULONG) eHeader + eHeader->SectionHeaderOffset;
	while(index < count){
		Dbg("section: "); Dbg(eCache->eSectionNames + section->Name); DbgLine(" ; ");
		++index;++section;
	}
}

static
void __show_all_reltypes(KMOD_ECACHE *e_cache){
	ULONG index = 0;
	ULONG count = e_cache->eRelCount;
	ELF_REL *e_rel = e_cache->eRelTable;
	while(index < count){
		Dbg("_rel: "); DbgInt(ELF32_R_TYPE(e_rel->Info)); DbgLine(" ;");
		++index; ++e_rel;
	}
}

static
void __show_all_relatypes(KMOD_ECACHE *e_cache){
	ULONG index = 0;
	ULONG count = e_cache->eRelaCount;
	ELF_REL *e_rela = e_cache->eRelaTable;
	while(index < count){
		Dbg("_rel: "); DbgInt(ELF32_R_TYPE(e_rela->Info)); DbgLine(" ;");
		++index; ++e_rela;
	}
}

VOID MdLoadELF(ELF_EHDR *eHeader, KMOD_RECORD *kmRecord){
	ELoadCache(&kmRecord->ECache, eHeader);
	Dbg("kmod-load -verbose");
	EFillRelocations(&kmRecord->ECache);
	EParseRela(&kmRecord->ECache);
	
	ELF_SYM *e_elf_dbg = EFindSymbolByName("elf_dbg", &kmRecord->ECache);
	if(
	e_elf_dbg){
		Dbg("_found");
		extern void elf_dbg();

		VOID (*ent)() = (VOID (*) ()) ((ULONG)eHeader + 4096);
		ent();
	}
	
	//__show_all_reltypes(&kmRecord->ECache);
	//EFillSymbolTable(eHeader);// Link the sym-tab

	DbgLine(" __elf_discoverd");
}
