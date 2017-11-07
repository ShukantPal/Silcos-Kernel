/**
 * File: ModuleLoader.h
 *
 * Summary:
 * ModuleLoader provides the interface to dynamically load and deload module binaries
 * which are present in memory already. It is a linker for the kernel & module and
 * can load a variety of file formats into the kernel.
 *
 * Copyright (C) 2017 - Shukant Pal
 */
#ifndef __INTERFACE_MODULE_MODULE_LOADER_H__
#define __INTERFACE_MODULE_MODULE_LOADER_H__

#include "ModuleRecord.h"
#include <Memory/Pager.h> // For PADDRESS type

struct Module::DynamicLink *MdLoadFile(PADDRESS moduleAddress, ULONG moduleSize, CHAR *cmdLine, Module::ModuleRecord *kmRecord);

VOID MdSetupLoader(
	VOID
);

namespace Module
{

/**
 * Struct: BlobRegister
 *
 * Summary:
 * This structure contains information about modules, that can be packaged into
 * multiple-blob-loader lists. These lists enable the module-loader to load the
 * modules that are inter-depedent by passing them on to the RecordManager at
 * once.
 *
 * Origin:
 * This was created to allow inter-depedent modules to load at once, for the
 * ModuleLoader class.
 *
 * Version: 1.1
 * Since: Circuit 2.03++
 * Author: Shukant Pal
 */
struct BlobRegister
{
	LinkedListNode liLinker;/* Used for participating in blob lists */
	PADDRESS loadAddr;/* Load address in physical memory */
	ULONG blobSize;/* Size of file/blob loaded */
	ULONG fileAddr;/* Used by loader for recording file's address in kernel-mem */
	CHAR *cmdLine;/* Command-line provided by client/NULL */
	ModuleRecord *regForm;/* Registration-forum given by client */
	ABI abiFound;/* Filled by loader - type of abi found */
	Void *manager;/* ABI-manager for the module (elf->ElfManager)*/
};

/**
 * Class: ModuleLoader
 *
 * Summary:
 * The ModuleLoader contains all the functions to load kernel-modules.
 *
 * Functions;
 * loadFile - Used for loading a unknown binary-blob
 * loadBundle - Used for loading a bundle of blobs
 *
 * Version: 1.1
 * Since: Circuit 2.03++
 * Author: Shukant Pal
 */
class ModuleLoader
{
public:
	static void loadFile(BlobRegister &blob);
	static void loadBundle(LinkedList &blobList);

private:
	static void *moveFileIntoMemory(BlobRegister &blob);
	static ABI globalizeDynamic(void *moduleMemory, ModuleRecord& kmRecord, BlobRegister &blob);
	static void linkFile(ABI binaryIfc, BlobRegister& blob);
};

}

#endif/* Module/ModuleLoader.h */
