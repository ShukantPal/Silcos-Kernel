/**
 * @file ElfManager.cpp
 * ------------------------------------------------------------------
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
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
#include <KERNEL.h>

using namespace Module;
using namespace Module::Elf;

extern ObjectInfo *tDynamicLink;

/**
 * Calculates the maximum relative-address that is used by the code,
 * data, and other stuff in the elf-object in the form of segments. In
 * other words, the relative-address where the end of all segments
 * resides or the size of the loaded binary.
 *
 * @param mgr - elf-object manager
 * @version 1.0
 * @since Circuit 2.03
 * @author Shukant Pal
 */
unsigned long ElfManager::getLimitAddress(ElfManager *mgr)
{
	unsigned long maxAddrFound = 0, curAddrFound;
	unsigned long phdrIndex = 0;
	unsigned long phdrCount = mgr->phdrCount;
	ProgramHeader *programHeader = mgr->phdrTable;

	while(phdrIndex < phdrCount)
	{
		if(programHeader->entryType == PT_LOAD)
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

	if(limitVAddr) {
		if(limitVAddr < KPGSIZE) {
			pageCount = 1;
		} else {
			pageCount = NextPowerOf2(limitVAddr) >> KPGOFFSET;
		}

		if(address == 0) {
			baseAddress = KiPagesAllocate(HighestBitSet(pageCount),
					ZONE_KMODULE, FLG_NONE);
		} else {
			baseAddress = address;
		}

		loadAddress = KeFrameAllocate(HighestBitSet(pageCount),
				ZONE_KERNEL, FLG_NONE);

		Pager::mapAll(baseAddress, loadAddress, pageCount * KPGSIZE, FLG_ATOMIC,
				PRESENT | READ_WRITE);

		baseAddress += (unsigned long) binaryHeader % KPGSIZE;
		unsigned long segIndex = 0;
		unsigned long segCount = phdrCount;
		ProgramHeader *segHdr = phdrTable;
		while(segIndex < segCount)
		{
			if(segHdr->entryType == PT_LOAD)
			{
				memcpyf(
						(const Void *)((unsigned long) binaryHeader + segHdr->fileOffset),
						(Void *)(baseAddress + segHdr->virtualAddress),
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
	while(TRUE);
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
		dynamicTable = (DynamicEntry *)(baseAddress + dynamicSegment->virtualAddress);
		dynamicEntryCount = dynamicSegment->memorySize / sizeof(DynamicEntry);

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
			dynamicSymbols.entryTable = (Symbol *) ((UBYTE *) binaryHeader + dsmEntry->ptr);
			dynamicSymbols.nameTable = (char *) ((UBYTE *) binaryHeader + dsmNamesEntry->ptr);

			// Fill dynamic hash-table field
			unsigned long *dsmHashContents = (unsigned long *)((UBYTE *) binaryHeader +
					dsmHashEntry->ptr);
			dynamicHash.bucketEntries = *dsmHashContents;
			dynamicHash.chainEntries = dsmHashContents[1];
			dynamicHash.bucketTable = dsmHashContents + 2;
			dynamicHash.chainTable = dsmHashContents + 2 + dynamicHash.bucketEntries;

			dynamicSymbols.entryCount = dynamicHash.chainEntries;
		}
		else
		{
			Dbg(" this one");
			fillBlankDsm();
		}

		// This section searches for the relocation tables of both types (RELA &
		// REL). It is also important for the module to export relocation entries.
		// But only one type is 'needed', RELA or REL
		DynamicEntry *dRelEntry = getDynamicEntry(DT_REL);
		DynamicEntry *dRelTableSizeEntry = getDynamicEntry(DT_RELSZ);
		DynamicEntry *dRelSizeEntry = getDynamicEntry(DT_RELENT);

		if(dRelEntry != NULL && dRelTableSizeEntry != NULL && dRelSizeEntry != NULL)
		{
			DbgLine("Boot Module - Linking (rel-type Elf) ");
			relTable.entryTable = (RelEntry *)(baseAddress + dRelEntry->ptr);
			relTable.entrySize = dRelSizeEntry->val;
			relTable.entryCount = dRelTableSizeEntry->val / relTable.entrySize;
		}
		else
			relTable.entryCount = 0;

		DynamicEntry *dRelaEntry = getDynamicEntry(DT_RELA);
		DynamicEntry *dRelaTableSizeEntry = getDynamicEntry(DT_RELASZ);
		DynamicEntry *dRelaSizeEntry = getDynamicEntry(DT_RELAENT);

		if(dRelaEntry != NULL && dRelaTableSizeEntry != NULL && dRelaSizeEntry != NULL)
		{
			DbgLine("Boot Module - Linking (rela-type Elf) ");
			relaTable.entryTable = (RelaEntry *) (baseAddress + dRelaEntry->ptr);
			while(TRUE);
			relaTable.entrySize = dRelaSizeEntry->val;
			relaTable.entryCount = dRelaTableSizeEntry->val / relaTable.entrySize;
		}
		else
			relaTable.entryCount = 0;

		DynamicEntry *dPltRelEntry = getDynamicEntry(DT_PLTREL);
		DynamicEntry *dPltRelSzEntry = getDynamicEntry(DT_PLTRELSZ);
		DynamicEntry *dPltJmpEntry = getDynamicEntry(DT_JMPREL);

		if(dPltRelEntry != NULL && dPltRelSzEntry != NULL && dPltJmpEntry != NULL)
		{
			pltRelocTable.relocType = dPltRelEntry->val;
			pltRelocTable.tableLocation = baseAddress + dPltJmpEntry->ptr;

			if(pltRelocTable.relocType == DT_REL)
			{
				pltRelocTable.entryCount = dPltRelSzEntry->val / sizeof(RelEntry);
			}
			else
			{
				pltRelocTable.entryCount = dPltRelSzEntry->val / sizeof(RelaEntry);
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
}

ElfManager::~ElfManager(){}

Symbol* ElfManager::getStaticSymbol(const char *name)
{
	return (ElfAnalyzer::querySymbol((char*) name,
			&dynamicSymbols, &dynamicHash));
}

/**
 * Returns the first program-header with the type passed.
 *
 * @param typeRequired - type of program-header required
 * @return - the first program-header that has this type; null, if
 * 		none found so.
 */
ProgramHeader *ElfManager::getProgramHeader(PhdrType typeRequired)
{
	unsigned long testIndex = 0;
	unsigned long testCount = phdrCount;
	ProgramHeader *testPhdr = phdrTable;

	while(testIndex < testCount)
	{
		if(testPhdr->entryType == typeRequired)
			return (testPhdr);

		++(testPhdr);
		++(testIndex);
	}

	return (null);
}

/**
 * Searches for the dynamic-tag entry in the dynamic-table for this
 * elf-object.
 *
 * @param tag - the tag of the required entry
 * @return - the dynamic-entry having the given tag; null, if it
 * 		doesn't exist in this elf-object
 */
DynamicEntry *ElfManager::getDynamicEntry(DynamicTag tag)
{
	DynamicEntry *entry = dynamicTable;

	while(entry->tag)
	{
		if(entry->tag == tag)
		{
			return (entry);
		}

		++(entry);
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
