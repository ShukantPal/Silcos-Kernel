///
/// @file MSI.cpp
/// @module KernelHost
///
/// Implements elf-object holder for the KernelHost module as its
/// file is not available at boot-time.
/// -------------------------------------------------------------------
/// This program is free software: you can redistribute it and/or modify
/// it under the terms of the GNU General Public License as published by
/// the Free Software Foundation, either version 3 of the License, or
/// (at your option) any later version.
///
/// This program is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
/// GNU General Public License for more details.
///
/// You should have received a copy of the GNU General Public License
/// along with this program.  If not, see <http://www.gnu.org/licenses/>
///
/// Copyright (C) 2017 - Shukant Pal
///

#include <Module/ModuleLoader.h>
#include <Module/ModuleRecord.h>
#include <Memory/KMemorySpace.h>
#include <Memory/KObjectManager.h>
#include <Module/MSI.h>
#include <Debugging.h>
#include <Executable/Thread.h>
#include <Utils/Memory.h>

using namespace Module;
using namespace Module::Elf;

KCOR_MSICACHE msiKernelSections;

///
/// Contains the microkernel dynamic-link info
///
static DynamicLink coreLink;

///
/// Symbol defined, as the location of the microkernel dynamic table
///
extern unsigned long msiKernelDynamicTableStart;

///
/// Symbol defined, as the location of the end of the microkernel dynamic table
///
extern unsigned long msiKernelDynamicTableEnd;

///
/// DynamicEntry* array for the microkernel
///
#define msiDynamicTable ((DynamicEntry *) ((unsigned long) &msiKernelDynamicTableStart))

///
/// DynamicEntry count for the microkernel
///
#define msiDynamicCount ((unsigned long) &msiKernelDynamicTableEnd - (unsigned long) &msiKernelDynamicTableStart) / sizeof(DynamicEntry)
#define HDR_SEARCH_START (unsigned long) &kernelSeg

///
/// Special function for getting a dynamic entry of the KernelHost
/// binary.
///
/// @param dRequiredTag - required dynamic tag
/// @return dynamic-entry associated with the given tag
/// @version 1.0
/// @since Circuit 2.03
/// @author Shukant Pal
///
DynamicEntry *KernelElf::getDynamicEntry(enum DynamicTag dRequiredTag)
{
	DynamicEntry *dynamicEntry = msiDynamicTable;
	unsigned long dynamicEntryCount = msiDynamicCount;
	unsigned long dynamicEntryIndex = 0;

	while(dynamicEntryIndex < dynamicEntryCount)
	{
		if(dynamicEntry->Tag == dRequiredTag)
		{
			return (dynamicEntry);
		}

		++(dynamicEntry);
		++(dynamicEntryIndex);
	}

	return (NULL);
}

///
/// Build-time name of this module
///
char coreModuleString[] = "Microkernel";

///
/// Extracts a dynamic-link struct for the KernelHost, as the normal
/// ElfAnalyzer function can't be used.
///
/// Args: NONE
/// @return module-record for the KernelHost, extracted using KernelElf
/// 		utility functions only
/// @author Shukant Pal
///
ModuleRecord *KernelElf::registerDynamicLink()
{
	DynamicEntry *dsmEntry = KernelElf::getDynamicEntry(DT_SYMTAB);
	DynamicEntry *dsmNamesEntry = KernelElf::getDynamicEntry(DT_STRTAB);
	DynamicEntry *dsmSizeEntry = KernelElf::getDynamicEntry(DT_SYMENT);
	DynamicEntry *dsmHashEntry = KernelElf::getDynamicEntry(DT_HASH);

	if(dsmEntry != NULL && dsmNamesEntry != NULL &&
			dsmSizeEntry != NULL && dsmHashEntry != NULL)
	{
		coreLink.dynamicSymbols.entryTable = (Symbol *) dsmEntry->refPointer;
		coreLink.dynamicSymbols.nameTable = (char *) dsmNamesEntry->refPointer;

		unsigned long *dsmHashContents = (unsigned long *) dsmHashEntry->refPointer;
		coreLink.symbolHash.bucketEntries = dsmHashContents[0];
		coreLink.symbolHash.chainEntries = dsmHashContents[1];
		coreLink.symbolHash.bucketTable = dsmHashContents + 2;
		coreLink.symbolHash.chainTable = dsmHashContents + 2 + coreLink.symbolHash.bucketEntries;

		coreLink.dynamicSymbols.entryCount = coreLink.symbolHash.chainEntries;

		ModuleRecord *coreRecord = RecordManager::create(coreModuleString, 1001, 0);
		coreRecord->BaseAddr = 0;// symbols already contain absolute values
		coreRecord->linkerInfo = &coreLink;

		RecordManager::registerRecord(coreRecord);
		return (coreRecord);
	}
	else
	{
		DbgLine("ERROR: DYNAMIC SYMBOLS NOT FOUND IN MICROKERNEL");
		return (NULL);
	}
}

ObjectInfo *tBlobRegister;
char nmBlobRegister[] = "BlobRegister";

/**
 * Function: KernelElf::loadBootModules
 *
 * Summary:
 * Loads all the boot-modules which were loaded into memory according to the
 * multiboot specification.
 *
 * Changes:
 * # Way to skip the entry point (for module that ain't have any entry), using
 *   the NO_ENTRY_ADDR symbol-value as the poopy linker because sets the
 *   entry point the the first byte in .text
 *
 * # Allow the kernel-host (silcos 3.02) to also be linked
 */
void KernelElf::loadBootModules()
{
	tBlobRegister = KiCreateType(nmBlobRegister, sizeof(BlobRegister), NO_ALIGN, NULL, NULL);
	LinkedList *bmRecordList = new(tLinkedList) LinkedList();

	/*
	 * This part lists all of the boot-time modules with their blob-registers
	 * in a list containing pointers to all registers.
	 */
	BlobRegister *blob;
	ModuleRecord *bmRecord = NULL;

	// Firstly add the "special" kernel-host elf-manager
	// but there ain't no blob - cause the bootloader only "loads" the host, not other modules
	// but again for linking the "false file" we create a blob & link the manager with blob
	ElfManager kernhost;
	BlobRegister kernhblob;

	bmRecord = RecordManager::create("Kernel Host", 3020, ModuleType::KMT_EXC_MODULE);
	RecordManager::registerRecord(bmRecord);
	kernhblob.manager = (void*) &kernhost;

	DynamicLink *khlinks = kernhost.exportDynamicLink();
	bmRecord->linkerInfo = khlinks;

	MultibootTagModule *foundModule = (MultibootTagModule*)
			SearchMultibootTagFrom(NULL, MULTIBOOT_TAG_TYPE_MODULE);
	while(foundModule != NULL)
	{
		bmRecord = RecordManager::create(foundModule->CMDLine, 0, ModuleType::KMT_EXC_MODULE);

		blob = new(tBlobRegister) BlobRegister();
		blob->loadAddr = foundModule->ModuleStart;
		blob->blobSize = foundModule->ModuleEnd - foundModule->ModuleStart;
		blob->cmdLine = &foundModule->CMDLine[0];
		blob->regForm = bmRecord;

		AddElement((LinkedListNode*) blob, bmRecordList);

		foundModule = (MultibootTagModule*)
				SearchMultibootTagFrom(foundModule, MULTIBOOT_TAG_TYPE_MODULE);
	}

	ModuleLoader::loadBundle(*bmRecordList);
	ModuleLoader::linkFile(ABI::ELF, kernhblob);
	DbgLine("Boot Bundle Loaded");

	BlobRegister *nextBlob;
	blob = (BlobRegister*) bmRecordList->head;
	while(blob != NULL)
	{
		ModuleLoader::init(*blob);
		blob = (BlobRegister*) blob->liLinker.next;
	}

	DbgLine("Initialized all kernel boot modules");

	blob = (BlobRegister*) bmRecordList->head;
	while(blob != NULL)
	{
		nextBlob = (BlobRegister*) blob->liLinker.next;
		kobj_free((kobj*) blob, tBlobRegister);
		blob = nextBlob;
	}

	KiDestroyType(tBlobRegister);
}
