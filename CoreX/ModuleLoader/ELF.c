#define NS_PMFLGS

#include <Memory/KMemoryManager.h>
#include <Memory/KMemorySpace.h>
#include <Memory/KObjectManager.h>
#include <Module/ELF.h>
#include <Module/ModuleRecord.h>
#include <KERNEL.h>

struct ObjectInfo *tELF_PHDR;

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
	struct ElfSectionHeader *eSectionHeader = (ELF_SHDR *) ((ADDRESS) eHeader + eHeader->SectionHeaderOffset);
	eSectionHeader += eHeader->SectionStringIndex;
	
	CHAR *eSectionNames = (CHAR *) ((ADDRESS) eHeader + eSectionHeader->Offset);
	return (eSectionNames);
}

static inline
struct ElfSectionHeader *
EFindSectionByIndex(ULONG eRequiredSectionIndex, ELF_EHDR *eHeader){
	return ((ELF_SHDR *) ((ADDRESS) eHeader + eHeader->SectionHeaderOffset)) + eRequiredSectionIndex;
}

ELF_SHDR *
EFindSectionByName(CHAR *eSectionName, ELF_EHDR *eHeader){
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

struct ElfSectionHeader *
EFindSectionHeaderByType(enum ElfSectionType eRequiredSectionType, ELF_SHDR *eSectionHeader, ELF_EHDR *eHeader){
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

struct ElfProgramHeader *
EFindProgramHeaderByType(
			enum ElfProgramHdrType eRequiredProgramHeaderType,
			struct ElfProgramHeader *eProgramHeader,
			struct ElfHeader *eHeader
){
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

struct ElfSectionHeader *
EFillSymbolTable(ELF_EHDR *eHeader){
	CHAR *eSymbolNames = (CHAR *) ((ADDRESS) eHeader + EFindSectionByName(".strtab", eHeader)->Offset);
	if(eSymbolNames == NULL) return (NULL);

	struct ElfSectionHeader *eDynamicSymbolTable = EFindSectionHeaderByType(SHT_SYMTAB, NULL, eHeader);
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

ULONG EHashSymbolName(CHAR *eSymbolName){
	ULONG hashValue = 0, testHolder;
	while(*eSymbolName){
		hashValue = (hashValue << 4) + *eSymbolName++;
		testHolder = hashValue & 0xF0000000;
		if(testHolder)
			hashValue ^= testHolder >> 24;
		hashValue &= ~testHolder;
	}

	return (hashValue);
}

/**
 * Function: ESearchForSymbol
 *
 * Summary:
 * This function searches for a symbol with the given name. It takes a symbol
 * table and its corresponding hash table and uses the bucket-chain algorithm
 * for finding the symbol.
 */
struct ElfSymbol *
ESearchForSymbol(CHAR *eRequiredSymbolNme, struct ElfSymbolTable *eSymbolTbl, struct ElfHashTable *eHashTbl){
	ULONG eSymbolHsh = EHashSymbolName(eRequiredSymbolNme);
	ULONG eBucketEnt = eSymbolHsh % eHashTbl->BucketEntries;

	ULONG eChainEnt = eHashTbl->BucketTable[eBucketEnt];
	ULONG *eChainPtr;
	struct ElfSymbol *eRelevantSymbol;
	do {
		eChainPtr = eHashTbl->ChainTable + eChainEnt;
	//	eRelevantSymbol = EFindSymbolByIndex(eChainEnt, eSymbolTbl);
		if(strcmp(eRequiredSymbolNme, eSymbolTbl->Names + eRelevantSymbol->Name))
			return (eRelevantSymbol);

		eChainEnt = *eChainPtr;
	} while(eChainEnt != STN_UNDEF);

	return (NULL);
}

/**
 * Function: ESearchForSymbolGlobally
 *
 * Summary:
 * This function searches for the symbol in all modules except the current one (thisModule)
 * of course.
 *
 */
struct ElfSymbol *
ESearchForSymbolGlobally(CHAR *eRequiredSymbolNm, struct ModuleRecord *thisModule){
	struct ModuleRecord *testModule = (struct ModuleRecord *) LoadedModules.Head;
	struct ElfCache *testCache;
	struct ElfSymbol *relevantSymbol;

	while(testModule != NULL){
		if(testModule != thisModule){
			testCache = &testModule->ECache;
			relevantSymbol = ESearchForSymbol(eRequiredSymbolNm, &testCache->dsmTable, &testCache->dsmHash);
			if(relevantSymbol != NULL)
				return (relevantSymbol);
		}

		testModule = testModule->NextModule;
	}

	return (NULL);
}

//================================================= DYNAMIC Section ====================================================

struct ElfDynamicEntry *
EFindDynamicEntry(enum DynamicTag requiredDTag, struct ElfCache *eCache){
	struct ElfDynamicTable *dynamicTbl = &eCache->ProgramHdrCache.DynamicTable;
	
	struct ElfDynamicEntry *dTag = dynamicTbl->EntryTable;
	ULONG dIndex = 0;
	ULONG dCount = dynamicTbl->EntryCount;
	while(dIndex < dCount){
		if(dTag->Tag == requiredDTag)
			return (dTag);

		++(dIndex);
		++(dTag);
	}

	return (NULL);
}

//=================================================== ELF CACHE ========================================================

void
ELoadProgramCache(struct ElfProgramCache *pgmCache, struct ElfHeader *moduleHeader)
{
	ULONG phdrIndex = 0;
	ULONG phdrCount = moduleHeader->ProgramHeaderEntryCount;
	struct ElfProgramHeader *phdr = PROGRAM_HEADER(moduleHeader);
	DbgInt(moduleHeader->ProgramHeaderOffset);

	ULONG pgmMemorySize = 0;
	while(phdrIndex < phdrCount)
	{
		switch(phdr->Type)
		{
		case PT_LOAD:
			Dbg("Phdr "); DbgInt(phdr->VAddr); Dbg(" size: "); DbgInt(phdr->MemorySize); Dbg(" align:"); DbgInt(phdr->Align); DbgLine(";");
			pgmMemorySize += phdr->MemorySize;
			break;
		case PT_DYNAMIC:
			pgmCache->Dynamic = phdr;
			pgmCache->DynamicTable.EntryTable = phdr->VAddr;
			pgmCache->DynamicTable.EntryCount = phdr->FileSize / sizeof(struct ElfDynamicEntry);
		}

		++(phdrIndex);
		++(phdr);
	}
	DbgLine("e_load_pgm_cache done");
	pgmCache->PageCount = NextPowerOf2(pgmMemorySize >> KPGOFFSET);
}

/**
 * Function: ELoadModule
 *
 * Summary:
 * This function loads the module segments into ZONE_KMODULE kernel memory and
 * copies the file segments to the module memory. This allows virtual addresses
 * to be used in the file.
 *
 * @Since Circuit 2.03
 * @Author Shukant Pal
 */
void
ELoadModule(struct ModuleRecord *elfModule)
{
	struct ElfHeader *moduleHeader = elfModule->ECache.eHeader;
	struct ElfProgramCache *modulePhdrCache = &elfModule->ECache.ProgramHdrCache;
	Void *moduleMemory = (Void *) KiPagesAllocate(HighestBitSet(modulePhdrCache->PageCount), ZONE_KMODULE, FLG_NONE);
	Void *moduleSegment;

	ULONG phdrIndex = 0;
	ULONG phdrCount = moduleHeader->ProgramHeaderEntryCount;
	struct ElfProgramHeader *phdr = PROGRAM_HEADER(moduleHeader);
	while(phdrIndex < phdrCount)
	{
		if(phdr->Type == PT_LOAD)
		{
			moduleSegment = (Void *) ((ADDRESS) moduleMemory + phdr->VAddr);
			EnsureUsability((ADDRESS) moduleSegment, NULL, FLG_NONE, KERNEL_DATA);
			memcpy((Void*) ((UBYTE *) moduleHeader + phdr->Offset), moduleSegment, 0);
		}
	}

	elfModule->BaseAddr = (ADDRESS) moduleMemory;
}

void
ELoadCache(struct ElfHeader *eHeader, struct ModuleRecord *mRecord){
	struct ElfCache *mCache = &mRecord->ECache;
	mCache->eHeader = eHeader;
//	kmCache->eSectionNames = EFindSectionNames(eHeader);
	ELoadProgramCache(&mCache->ProgramHdrCache, mCache->eHeader);
	ELoadModule(mRecord);

/*
	struct ElfSectionHeader *eSymtabSection = EFindSectionHeaderByType(SHT_SYMTAB, NULL, eHeader);
	kmCache->eSymbolNames = (CHAR *) ((ADDRESS) eHeader + EFindSectionByName(".strtab", eHeader)->Offset);
	kmCache->eSymbolTable = (ELF_SYM *) ((ADDRESS) eHeader + eSymtabSection->Offset);
	kmCache->eSymbolTableLength = eSymtabSection->Size / sizeof(ELF_SYM);

	struct ElfSectionHeader *dsmShdr = EFindSectionHeaderByType(SHT_DYNSYM, NULL, eHeader);
	kmCache->eDynamicSymbolNames = (CHAR *) ((ADDRESS) eHeader + EFindSectionByName(".dynstr", eHeader)->Offset);
	kmCache->eDynamicSymbolTable = (ELF_SYM *) ((ADDRESS) eHeader + dsmShdr->Offset);
	kmCache->eDynamicSymbolTableLength = dsmShdr->Size / sizeof(ELF_SYM);

	struct ElfSectionHeader *dynShdr = EFindSectionHeaderByType(SHT_DYNAMIC, NULL, eHeader);
	kmCache->_DYNAMIC = (ELF32_DYN *) ((ADDRESS) eHeader + dynShdr->Offset);
	kmCache->eDynTableLength = dynShdr->Size / sizeof(DTAG);
	
	struct ElfSectionHeader *dsmHashShdr = EFindSectionHeaderByType(SHT_HASH, NULL, eHeader);
	struct ElfHashTable *dsmHashTbl = &kmCache->DynamicSymbolHashTbl;
	ULONG *dsmHashContents = (ULONG *) ((ADDRESS) eHeader + dsmHashShdr->Offset);
	dsmHashTbl->HashSectionHdr = dsmHashShdr;
	dsmHashTbl->BucketEntries = *dsmHashContents;
	dsmHashTbl->ChainEntries = *(dsmHashContents + 1);
	dsmHashTbl->BucketTable = dsmHashContents + 2;
	dsmHashTbl->ChainTable = dsmHashContents + 2 + dsmHashTbl->BucketEntries;
*/

	struct ElfDynamicEntry *dsmEntry = EFindDynamicEntry(DT_SYMTAB, mCache);
	struct ElfDynamicEntry *dsmCountEntry = EFindDynamicEntry(DT_SYMENT, mCache);
	struct ElfDynamicEntry *dsmNamesEntry = EFindDynamicEntry(DT_STRTAB, mCache);
	struct ElfDynamicEntry *dsmHashEntry = EFindDynamicEntry(DT_HASH, mCache);
	if(dsmEntry == NULL || dsmCountEntry == NULL
			|| dsmNamesEntry == NULL || dsmHashEntry == NULL)
	{// Load Dynamic Symbol Table
		struct ElfSymbolTable *dynamicSymbols = &mCache->dsmTable;
		dynamicSymbols->Names = (CHAR *) mRecord->BaseAddr + dsmNamesEntry->Pointer;
		dynamicSymbols->EntryTable = (struct ElfSymbol *) (mRecord->BaseAddr + dsmEntry->Pointer);
		dynamicSymbols->EntryCount = dsmCountEntry->Value;

		struct ElfHashTable *dSymbolHash = &mCache->dsmHash;
		ULONG *dSymbolHashContents = (ULONG *) (mRecord->BaseAddr + dsmHashEntry->Pointer);
		dSymbolHash->BucketEntries = dSymbolHashContents[0];
		dSymbolHash->ChainEntries = dSymbolHashContents[1];
		dSymbolHash->BucketTable = dSymbolHashContents + 2;
		dSymbolHash->ChainTable = dSymbolHashContents + 2 + dSymbolHash->BucketEntries;
	} else
		Dbg("_err dsmEntry null");

	return;
}

void
MdLoadELF(struct ElfHeader *eHeader, struct ModuleRecord *kmRecord)
{
	ELoadCache(eHeader, kmRecord);
	DbgLine("Module::ELF - @TransferControl");

	ELoadModule(kmRecord);
	
	//__show_all_reltypes(&kmRecord->ECache);
	//EFillSymbolTable(eHeader);// Link the sym-tab

	DbgLine("__success endfordbg");
}

void
ESetupLoader(){
	tELF_PHDR = SETUP_OBJECT(ELF_PHDR);
}
