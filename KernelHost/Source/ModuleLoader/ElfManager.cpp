/**
 * File: ElfManager.cpp
 *
 * Summary:
 * ElfManager and its related services are implemented here. Note that many
 * functions are borrowed from the ElfAnalyzer.
 *
 * Copyright (C) 2017 - Shukant Pal
 */
#define NS_PMFLGS

#include <Memory/KMemorySpace.h>
#include <Memory/KMemoryManager.h>
#include <Memory/KObjectManager.h>
#include <Module/Elf/ElfManager.hpp>
#include <Module/Elf/ElfLinker.hpp>
#include <Module/ModuleLoader.h>
#include <Module/MSI.h>
#include <KERNEL.h>

using namespace Module;
using namespace Module::Elf;

extern ObjectInfo *tDynamicLink;

extern "C" void elf_dbg();

unsigned long ElfManager::getLimitAddress(ElfManager *mgr)
{
	unsigned long maxAddrFound = 0, curAddrFound;
	unsigned long phdrIndex = 0;
	unsigned long phdrCount = mgr->phdrCount;
	ProgramHeader *programHeader = mgr->phdrTable;

	while(phdrIndex < phdrCount)
	{
		if(programHeader->entryType == PT_LOAD || programHeader->entryType == PT_DYNAMIC)
		{
			curAddrFound = programHeader->virtualAddress + programHeader->memorySize;
			if(maxAddrFound < curAddrFound)
			{
				maxAddrFound = curAddrFound;
			}
		}

		++(programHeader);
		++(phdrIndex);
	}

	return (maxAddrFound);
}

void ElfManager::loadBinary(unsigned long address)
{
	// This part loads the module's binary into kernel memory & places
	// all segments into the correct areas.
	unsigned long limitVAddr = ElfManager::getLimitAddress(this);
	if(limitVAddr)
	{
		pageCount = NextPowerOf2(limitVAddr) >> KPGOFFSET;

		if(address == 0)
		{
			baseAddress = KiPagesAllocate(HighestBitSet(pageCount), ZONE_KMODULE, FLG_NONE);
		}
		else
		{
			baseAddress = address;
		}

		loadAddress = KeFrameAllocate(HighestBitSet(pageCount), ZONE_KERNEL, FLG_NONE);
		EnsureAllMappings(baseAddress, loadAddress, pageCount * KPGSIZE, NULL, PRESENT | READ_WRITE);

		unsigned long segIndex = 0;
		unsigned long segCount = phdrCount;
		ProgramHeader *segHdr = phdrTable;
		while(segIndex < segCount)
		{
			if(segHdr->entryType == PT_LOAD || segHdr->entryType == PT_DYNAMIC)
			{
				memcpyf(
						(const Void *) ((UBYTE *) binaryHeader + segHdr->fileOffset),
						(Void *) (baseAddress + segHdr->virtualAddress),
						segHdr->fileSize
				);

				// Segments like BSS have extra space at the end which needs all
				// set to zeros
				if(segHdr->fileSize < segHdr->memorySize)
				{
					memsetf(
							(Void *) ((UBYTE *) baseAddress + segHdr->virtualAddress + segHdr->fileSize),
							0,
							segHdr->memorySize - segHdr->fileSize
					);
				}
			}

			++(segIndex);
			++(segHdr);
		}
	}
	else
	{
		// This should rarely happen, but the kernel is ready for malicious modules
		// and module-loading fails.
		pageCount = 0;
		baseAddress = 0;
	}
}

static char msgDynamicSegmentMissing[] = "ElfManager - No dynamic segment !";
static char msgDsmMissing[] = "ElfManager - Dynamic Symbols or Hash table missing";

void ElfManager::fillBlankDsm()
{
	dynamicSymbols.entryCount = 0;
	dynamicHash.bucketEntries = 0;
	dynamicHash.chainEntries = 0;
	DbgLine(msgDsmMissing);
}

ElfManager::ElfManager()
{
	baseAddress = 0;
	binaryHeader = 0;

	DynamicEntry *dsmEntry = KernelElf::getDynamicEntry(DT_SYMTAB);
	DynamicEntry *dsmNamesEntry = KernelElf::getDynamicEntry(DT_STRTAB);
	DynamicEntry *dsmSizeEntry = KernelElf::getDynamicEntry(DT_SYMENT);
	DynamicEntry *dsmHashEntry = KernelElf::getDynamicEntry(DT_HASH);

	if(dsmEntry != NULL && dsmNamesEntry != NULL &&
			dsmSizeEntry != NULL && dsmHashEntry != NULL)
	{
		// Fill dynamic symbol table field
		dynamicSymbols.entryTable = (Symbol *) ((UBYTE *) binaryHeader + dsmEntry->refPointer);
		dynamicSymbols.nameTable = (char *) ((UBYTE *) binaryHeader + dsmNamesEntry->refPointer);

		// Fill dynamic hash-table field
		unsigned long *dsmHashContents = (unsigned long *) ((UBYTE *) binaryHeader + dsmHashEntry->refPointer);
		dynamicHash.bucketEntries = *dsmHashContents;
		dynamicHash.chainEntries = dsmHashContents[1];
		dynamicHash.bucketTable = dsmHashContents + 2;
		dynamicHash.chainTable = dsmHashContents + 2 + dynamicHash.bucketEntries;

		dynamicSymbols.entryCount = dynamicHash.chainEntries;
	}
	else
		fillBlankDsm();

	// This section searches for the relocation tables of both types (RELA &
	// REL). It is also important for the module to export relocation entries.
	// But only one type is 'needed', RELA or REL
	DynamicEntry *dRelEntry = KernelElf::getDynamicEntry(DT_REL);
	DynamicEntry *dRelTableSizeEntry = KernelElf::getDynamicEntry(DT_RELSZ);
	DynamicEntry *dRelSizeEntry = KernelElf::getDynamicEntry(DT_RELENT);

	if(dRelEntry != NULL && dRelTableSizeEntry != NULL && dRelSizeEntry != NULL)
	{
		DbgLine("Boot Module - Linking (rel-type Elf) ");
		relTable.entryTable = (RelEntry *) (baseAddress + dRelEntry->refPointer);
		relTable.entrySize = dRelSizeEntry->refValue;
		relTable.entryCount = dRelTableSizeEntry->refValue / relTable.entrySize;
	}
	else
		relTable.entryCount = 0;

	DynamicEntry *dRelaEntry = KernelElf::getDynamicEntry(DT_RELA);
	DynamicEntry *dRelaTableSizeEntry = KernelElf::getDynamicEntry(DT_RELASZ);
	DynamicEntry *dRelaSizeEntry = KernelElf::getDynamicEntry(DT_RELAENT);

	if(dRelaEntry != NULL && dRelaTableSizeEntry != NULL && dRelaSizeEntry != NULL)
	{
		DbgLine("Boot Module - Linking (rela-type Elf) ");
		relaTable.entryTable = (RelaEntry *) (baseAddress + dRelaEntry->refPointer);
		relaTable.entrySize = dRelaSizeEntry->refValue;
		relaTable.entryCount = dRelaTableSizeEntry->refValue / relaTable.entrySize;
	}
	else
		relaTable.entryCount = 0;

	DynamicEntry *dPltRelEntry = KernelElf::getDynamicEntry(DT_PLTREL);
	DynamicEntry *dPltRelSzEntry = KernelElf::getDynamicEntry(DT_PLTRELSZ);
	DynamicEntry *dPltJmpEntry = KernelElf::getDynamicEntry(DT_JMPREL);

	if(dPltRelEntry != NULL && dPltRelSzEntry != NULL && dPltJmpEntry != NULL)
	{
		pltRelocTable.relocType = dPltRelEntry->refValue;
		pltRelocTable.tableLocation = baseAddress + dPltJmpEntry->refPointer;

		if(pltRelocTable.relocType == DT_REL)
		{
			pltRelocTable.entryCount = dPltRelSzEntry->refValue / sizeof(RelEntry);
		}
		else
		{
			pltRelocTable.entryCount = dPltRelSzEntry->refValue / sizeof(RelaEntry);
		}
	}
}

ElfManager::ElfManager(ElfHeader *binaryHeader)
{
	// Find PHDR-table & SHDR-table
	this->binaryHeader = binaryHeader;
	phdrTable = PROGRAM_HEADER(binaryHeader);
	phdrCount = binaryHeader->programHeaderEntryCount;
	phdrSize = binaryHeader->programHeaderEntrySize;
	shdrTable = SECTION_HEADER(binaryHeader);
	shdrCount = binaryHeader->sectionHeaderEntryCount;
	shdrSize = binaryHeader->sectionHeaderEntrySize;

	loadBinary(0);

	// A dynamic segment should be present in all kernel modules, because they
	// need to be linked with the kernel.
	ProgramHeader *dynamicSegment = getProgramHeader(PT_DYNAMIC);
	if(dynamicSegment != NULL)
	{
		dynamicTable = (DynamicEntry *) (baseAddress + dynamicSegment->virtualAddress);
		dynamicEntryCount = dynamicSegment->fileSize / sizeof(DynamicEntry);

		// This section searches for the dynamic symbol table & its corresponding
		// hash table.
		DynamicEntry *dsmEntry = getDynamicEntry(DT_SYMTAB);
		DynamicEntry *dsmCountEntry = getDynamicEntry(DT_SYMENT);
		DynamicEntry *dsmNamesEntry = getDynamicEntry(DT_STRTAB);
		DynamicEntry *dsmHashEntry = getDynamicEntry(DT_HASH);

		if(dsmEntry != NULL && dsmCountEntry != NULL
				&& dsmNamesEntry != NULL && dsmHashEntry != NULL)
		{
			// Fill dynamic symbol table field
			dynamicSymbols.entryTable = (Symbol *) ((UBYTE *) binaryHeader + dsmEntry->refPointer);
			dynamicSymbols.nameTable = (char *) ((UBYTE *) binaryHeader + dsmNamesEntry->refPointer);

			// Fill dynamic hash-table field
			unsigned long *dsmHashContents = (unsigned long *) ((UBYTE *) binaryHeader + dsmHashEntry->refPointer);
			dynamicHash.bucketEntries = *dsmHashContents;
			dynamicHash.chainEntries = dsmHashContents[1];
			dynamicHash.bucketTable = dsmHashContents + 2;
			dynamicHash.chainTable = dsmHashContents + 2 + dynamicHash.bucketEntries;

			dynamicSymbols.entryCount = dynamicHash.chainEntries;
		}
		else
			fillBlankDsm();

		// This section searches for the relocation tables of both types (RELA &
		// REL). It is also important for the module to export relocation entries.
		// But only one type is 'needed', RELA or REL
		DynamicEntry *dRelEntry = getDynamicEntry(DT_REL);
		DynamicEntry *dRelTableSizeEntry = getDynamicEntry(DT_RELSZ);
		DynamicEntry *dRelSizeEntry = getDynamicEntry(DT_RELENT);

		if(dRelEntry != NULL && dRelTableSizeEntry != NULL && dRelSizeEntry != NULL)
		{
			DbgLine("Boot Module - Linking (rel-type Elf) ");
			relTable.entryTable = (RelEntry *) (baseAddress + dRelEntry->refPointer);
			relTable.entrySize = dRelSizeEntry->refValue;
			relTable.entryCount = dRelTableSizeEntry->refValue / relTable.entrySize;
		}
		else
			relTable.entryCount = 0;

		DynamicEntry *dRelaEntry = getDynamicEntry(DT_RELA);
		DynamicEntry *dRelaTableSizeEntry = getDynamicEntry(DT_RELASZ);
		DynamicEntry *dRelaSizeEntry = getDynamicEntry(DT_RELAENT);

		if(dRelaEntry != NULL && dRelaTableSizeEntry != NULL && dRelaSizeEntry != NULL)
		{
			DbgLine("Boot Module - Linking (rela-type Elf) ");
			relaTable.entryTable = (RelaEntry *) (baseAddress + dRelaEntry->refPointer);
			relaTable.entrySize = dRelaSizeEntry->refValue;
			relaTable.entryCount = dRelaTableSizeEntry->refValue / relaTable.entrySize;
		}
		else
			relaTable.entryCount = 0;

		DynamicEntry *dPltRelEntry = getDynamicEntry(DT_PLTREL);
		DynamicEntry *dPltRelSzEntry = getDynamicEntry(DT_PLTRELSZ);
		DynamicEntry *dPltJmpEntry = getDynamicEntry(DT_JMPREL);

		if(dPltRelEntry != NULL && dPltRelSzEntry != NULL && dPltJmpEntry != NULL)
		{
			pltRelocTable.relocType = dPltRelEntry->refValue;
			pltRelocTable.tableLocation = baseAddress + dPltJmpEntry->refPointer;

			if(pltRelocTable.relocType == DT_REL)
			{
				pltRelocTable.entryCount = dPltRelSzEntry->refValue / sizeof(RelEntry);
			}
			else
			{
				pltRelocTable.entryCount = dPltRelSzEntry->refValue / sizeof(RelaEntry);
			}
		}
	}
	else
	{
		DbgLine(msgDynamicSegmentMissing);
		dynamicTable = NULL;
		dynamicEntryCount = 0;

		fillBlankDsm();

		relTable.entryCount = 0;
		relaTable.entryCount = 0;
	}

	DynamicEntry *init = this->getDynamicEntry(DT_INIT);
	if(init)
	{
		Dbg("init: ");
		DbgInt(init->refPointer);
		DbgLine(" ");
	}
}

ElfManager::~ElfManager(){}

Symbol* ElfManager::getStaticSymbol(const char *name)
{
	return ElfAnalyzer::querySymbol((char*) name, &dynamicSymbols, &dynamicHash);
}

/**
 * Function: ElfManager::getProgramHeader
 *
 * Summary:
 * This function searches a program header in the elf-module by its type. It
 * can return NULL also.
 *
 * Args:
 * enum PhdrType typeRequired - Required-type of the program-header
 *
 * Author: Shukant Pal
 */
ProgramHeader *ElfManager::getProgramHeader(PhdrType typeRequired)
{
	if(phdrTable)
	{
		unsigned long testIndex = 0;
		unsigned long testCount = phdrCount;
		ProgramHeader *testPhdr = phdrTable;

		while(testIndex < testCount)
		{
			if(testPhdr->entryType == typeRequired)
			{
				return (testPhdr);
			}
			
			++(testPhdr);
			++(testIndex);
		}
	}

	return (NULL);
}

/**
 * Function: ElfManager::getDynamicEntry
 *
 * Summary:
 * This function searches for a entry in the dynamic section/segment of the elf
 * module. It may return NULL on failure.
 *
 * Args:
 * enum DynamicTag tag - Required tag for the entry
 *
 * Author: Shukant Pal
 */
DynamicEntry *ElfManager::getDynamicEntry(DynamicTag tag)
{
	if(dynamicTable)
	{
		unsigned long entryIndex = 0;
		DynamicEntry *entry = dynamicTable;

		while(entryIndex < dynamicEntryCount)
		{
			if(entry->Tag == tag)
			{
				return (entry);
			}
			
			++(entryIndex);
			++(entry);
		}
	}

	return (NULL);
}

/**
 * Function: ElfManager::exportDynamicLink
 *
 * Summary:
 * ElfManager is a heavy-weight container for the elf-binary and can be used to
 * get all required information out of it. All of this is almost unnecessary
 * for the module linker, thus a (DynamicLink) structure is used, which is
 * exported from the module by this function.
 *
 * Returns:
 * Exported dynamic link structure, which can be used for linkage with other kernel
 * modules.
 *
 * Version: 1.1
 * Since: Circuit 2.03++
 * Author: Shukant Pal
 */
DynamicLink *ElfManager::exportDynamicLink()
{
	DynamicLink *linkInfo = new(tDynamicLink) DynamicLink();

	memcpy((Void*) &dynamicSymbols, (Void*) &linkInfo->dynamicSymbols, sizeof(SymbolTable));
	memcpy((Void*) &dynamicHash, (Void*) &linkInfo->symbolHash, sizeof(HashTable));
	linkInfo->moduleDependencies = NULL;

	return (linkInfo);
}