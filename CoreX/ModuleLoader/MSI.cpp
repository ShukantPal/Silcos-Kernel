/**
 * File: KernelElf.cpp
 *
 * Summary:
 * This file implements abstracting the microkernel elf-binary & loading the
 * boot-modules.
 *
 * Copyright (C) 2017 - Shukant Pal
 */
#include <Exec/Thread.h>
#include <Module/ModuleLoader.h>
#include <Module/ModuleRecord.h>
#include <Memory/KMemorySpace.h>
#include <Memory/KObjectManager.h>
#include <Module/MSI.h>
#include <Util/Memory.h>
#include <Debugging.h>

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
extern ULONG msiKernelDynamicTableStart;

/*
 * Symbol defined, as the location of the end of the microkernel dynamic table
 */
extern ULONG msiKernelDynamicTableEnd;

/*
 * DynamicEntry* array for the microkernel
 */
#define msiDynamicTable ((DynamicEntry *) ((ULONG) &msiKernelDynamicTableStart))

/*
 * DynamicEntry count for the microkernel
 */
#define msiDynamicCount ((ULONG) &msiKernelDynamicTableEnd - (ULONG) &msiKernelDynamicTableStart) / sizeof(DynamicEntry)

extern ULONG kernelSeg;
extern ULONG endKSearch;
#define HDR_SEARCH_START (ULONG) &kernelSeg

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
DynamicEntry *KernelElf::getDynamicEntry(
		enum DynamicTag dRequiredTag
){
	DynamicEntry *dynamicEntry = msiDynamicTable;
	unsigned long dynamicEntryCount = msiDynamicCount;
	unsigned long dynamicEntryIndex = 0;

	while(dynamicEntryIndex < dynamicEntryCount){
		if(dynamicEntry->Tag == dRequiredTag)
			return (dynamicEntry);

		++(dynamicEntry);
		++(dynamicEntryIndex);
	}

	return (NULL);
}

/*
 * Build name for the microkernel
 */
CHAR coreModuleString[] = "Microkernel";

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
			dsmSizeEntry != NULL && dsmHashEntry != NULL){
		coreLink.dynamicSymbols.entryTable = (Symbol *) dsmEntry->refPointer;
		coreLink.dynamicSymbols.nameTable = (CHAR *) dsmNamesEntry->refPointer;

		ULONG *dsmHashContents = (ULONG *) dsmHashEntry->refPointer;
		coreLink.symbolHash.bucketEntries = dsmHashContents[0];
		coreLink.symbolHash.chainEntries = dsmHashContents[1];
		coreLink.symbolHash.bucketTable = dsmHashContents + 2;
		coreLink.symbolHash.chainTable = dsmHashContents + 2 + coreLink.symbolHash.bucketEntries;

		coreLink.dynamicSymbols.entryCount = coreLink.symbolHash.chainEntries;

		ModuleRecord *coreRecord = RecordManager::createRecord(coreModuleString, 1001, 0);
		coreRecord->linkerInfo = &coreLink;

		RecordManager::registerRecord(coreRecord);
		return (coreRecord);
	} else {
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
CHAR nmBlobRegister[] = "BlobRegister";

void KernelElf::loadBootModules()
{
	tBlobRegister = KiCreateType(nmBlobRegister, sizeof(BlobRegister), NO_ALIGN, NULL, NULL);

	/*
	 * List of blobs
	 */
	LinkedList *bmRecordList = new(tLinkedList) LinkedList();

	/*
	 * This part lists all of the boot-time modules with their blob-registers
	 * in a list containing pointers to all registers.
	 */
	BlobRegister *blob;
	ModuleRecord *bmRecord = NULL;
	MultibootTagModule *foundModule = SearchMultibootTag(MULTIBOOT_TAG_TYPE_MODULE);
	while(foundModule != NULL){
		bmRecord = RecordManager::createRecord(foundModule->CMDLine, 0, ModuleType::KMT_EXC_MODULE);

		blob = new(tBlobRegister) BlobRegister();
		blob->loadAddr = foundModule->ModuleStart;
		blob->blobSize = foundModule->ModuleEnd - foundModule->ModuleStart;
		blob->cmdLine = &foundModule->CMDLine[0];
		blob->regForm = bmRecord;

		AddElement((LinkedListNode*) blob, bmRecordList);

		foundModule = SearchMultibootTagFrom(foundModule, MULTIBOOT_TAG_TYPE_MODULE);
	}

	ModuleLoader::loadBundle(*bmRecordList);

	/*
	 * Free all useless blobs
	 */
	BlobRegister *nextBlob;
	blob = (BlobRegister*) bmRecordList->Head;
	while(blob != NULL){
		KThreadCreate((void(*)()) blob->regForm->entryAddr);
		nextBlob = (BlobRegister*) blob->liLinker.Next;
		kobj_free((kobj*) blob, tBlobRegister);
		blob = nextBlob;
	}

	KiDestroyType(tBlobRegister);
}
