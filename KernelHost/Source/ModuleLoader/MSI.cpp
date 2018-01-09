/**
 * File: KernelElf.cpp
 *
 * Summary:
 * This file implements abstracting the microkernel elf-binary & loading the
 * boot-modules.
 *
 * Copyright (C) 2017 - Shukant Pal
 */
#include <Module/ModuleLoader.h>
#include <Module/ModuleRecord.h>
#include <Memory/KMemorySpace.h>
#include <Memory/KObjectManager.h>
#include <Module/MSI.h>
#include <Util/Memory.h>
#include <Debugging.h>
#include <Executable/Thread.h>

using namespace Module;
using namespace Module::Elf;

KCOR_MSICACHE msiKernelSections;

/*
 * Contains the microkernel dynamic-link info
 */
static struct Module::DynamicLink coreLink;

/*
 * Contains the module-record for the microkernel
 */
static ModuleRecord coreRecord;

/*
 * Symbol defined, as the location of the microkernel dynamic table
 */
extern unsigned long msiKernelDynamicTableStart;

/*
 * Symbol defined, as the location of the end of the microkernel dynamic table
 */
extern unsigned long msiKernelDynamicTableEnd;

/*
 * DynamicEntry* array for the microkernel
 */
#define msiDynamicTable ((DynamicEntry *) ((unsigned long) &msiKernelDynamicTableStart))

/*
 * DynamicEntry count for the microkernel
 */
#define msiDynamicCount ((unsigned long) &msiKernelDynamicTableEnd - (unsigned long) &msiKernelDynamicTableStart) / sizeof(DynamicEntry)
#define HDR_SEARCH_START (unsigned long) &kernelSeg

/**
 * KernelElf::getDynamicEntry
 *
 * Summary:
 * Special function for getting a dynamic entry of the microkernel (only).
 *
 * Args:
 * DynamicTag dRequiredTag - required dynamic tag
 *
 * Author: Shukant Pal
 */
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

/*
 * Build name for the microkernel
 */
char coreModuleString[] = "Microkernel";

/**
 * Function: KernelElf::registerDynamicLink
 *
 * Summary:
 * This function will setup the microkernel-record & its dynamic link-info for use
 * by module-loaders.
 *
 * Args: NONE
 *
 * Author: Shukant Pal
 */
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

/*
 * Temporary object-allocator for getting blob-registers for the boot modules.
 *
 * @Note - Future builds will contain a general-use heap from which they will
 * be taken
 */
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

	ModuleRecord *khrec = bmRecord;

	MultibootTagModule *foundModule = (MultibootTagModule*) SearchMultibootTagFrom(NULL, MULTIBOOT_TAG_TYPE_MODULE);
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
		if(blob->regForm->init)
		{
			blob->regForm->init();
		}

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
