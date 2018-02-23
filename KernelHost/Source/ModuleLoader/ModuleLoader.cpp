///
/// @file ModuleLoader.cpp
/// @module KernelHost
///
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

#define NS_PMFLGS // Paging Flags

#include <Memory/Pager.h>
#include <Memory/KMemoryManager.h>
#include <Memory/KObjectManager.h>
#include <Module/ModuleLoader.h>
#include <Module/Elf/ABI/icxxabi.h>
#include <Module/Elf/ELF.h>
#include <Module/Elf/ElfManager.hpp>
#include <Module/Elf/ElfAnalyzer.hpp>
#include <Module/Elf/ElfLinker.hpp>
#include <KERNEL.h>

using namespace Module;
using namespace Module::Elf;

extern "C" void elf_dbg() { Dbg("ELF PROGRAME CALLS MK");}
char elf_data;

ObjectInfo *tKMOD_RECORD;

char nmElfManager[] = "Module::Elf::ElfManager";
char nmDynamicLink[] = "Module::DynamicLink";

/*
 * ElfManager objects are allocated dynamically
 */
struct ObjectInfo *tElfManager;// class ElfManager - Slab-Allocator

/*
 * Dynamic-linker objects are allocated for registration
 */
struct ObjectInfo *tDynamicLink;// struct DynamicLink - Slab-Allocator

LinkedList LoadedModules;

/**
 * Function: ModuleLoader::moveFileIntoMemory
 *
 * Summary:
 * Moves a binary-blob into the kernel memory by mapping its physical addresses
 *
 * Args:
 * BlobRegister& blob - Blob parameters
 *
 * Author: Shukant Pal
 */
void *ModuleLoader::moveFileIntoMemory(BlobRegister &blob)
{
	blob.blobSize = NextPowerOf2(blob.blobSize);
	unsigned long fileSize = blob.blobSize;

	if(fileSize < KPGSIZE)
	{
		fileSize = KPGSIZE;
		blob.blobSize = fileSize;
	}

	#ifdef ARCH_32
		return_if(fileSize > MB(2), FALSE);
	#else // ARCH_64
		return_if(fileSize > MB(4), FALSE);
	#endif

	Void *memoryArena = (void *) KiPagesAllocate(HighestBitSet(fileSize >> KPGOFFSET), ZONE_KMODULE, FLG_NONE);
	EnsureAllMappings((unsigned long) memoryArena, blob.loadAddr, blob.blobSize, NULL, PRESENT | READ_WRITE);

	return (memoryArena);
}

///
/// Exports the dynamic-link record of the given "loaded" blob and notes
/// any initialization/finalization functions presents. Here, the __init()
/// or any other initialization function is called.
///
/// @param moduleMemory
/// @param kmRecord
/// @param blob
/// @return - the ABI of the given module
///
ABI ModuleLoader::globalizeDynamic(void *moduleMemory, ModuleRecord& kmRecord, BlobRegister &blob)
{
	if(!ElfAnalyzer::validateBinary(moduleMemory))
		return (ABI::INVALID);

	ElfManager *moduleHandler = new(tElfManager) ElfManager((ElfHeader*) moduleMemory);
	DynamicLink *linkHandle = moduleHandler->exportDynamicLink();

	kmRecord.linkerInfo = linkHandle;
	kmRecord.BaseAddr = moduleHandler->baseAddress;

	if(moduleHandler->binaryHeader->entryAddress)
	{
		kmRecord.entryAddr = kmRecord.BaseAddr +
				moduleHandler->binaryHeader->entryAddress;
	}

	Symbol *__initer = moduleHandler->getStaticSymbol("__init");
	if(__initer)
	{
		kmRecord.init = (void (*)())(kmRecord.BaseAddr +
					__initer->Value);
	}
	else
	{
		DynamicEntry* dynamicIniter = moduleHandler->getDynamicEntry(DT_INIT);
		if(dynamicIniter)
		{
			DbgLine("dyn-init");
			kmRecord.init = (void (*)())(kmRecord.BaseAddr +
					dynamicIniter->refPointer);
		}
	}
	DynamicEntry* dynamicFinier = moduleHandler->getDynamicEntry(DT_FINI);
	if(dynamicFinier)
	{
		kmRecord.fini = (void (*)())(kmRecord.BaseAddr + dynamicFinier->refPointer);
	}

	blob.manager = moduleHandler;
	return (ABI::ELF);
}

/**
 * Function: ModuleLoader::linkFile
 *
 * Summary:
 * This function finishes linkage of the module.
 *
 * Args:
 * void *modmem - Module's address in virtual memory
 *
 * Author: Shukant Pal
 */
void ModuleLoader::linkFile(ABI binaryIfc, BlobRegister& blob)
{
	ElfManager *manager;
	switch(binaryIfc)
	{
	case ABI::ELF:
		manager = (ElfManager*) blob.manager;
		ElfLinker::resolveLinkage(*manager);
		manager->~ElfManager();
		kobj_free((kobj*) manager, tElfManager);
		break;
	case ABI::INVALID:
		DbgLine("ModuleLoader::linkFile - INVALID ABI is not recognized.");
		break;
	default:
		DbgLine("Unknown ABI Given");
		break;
	}
}

/**
 * Function: ModuleLoader::loadFile
 *
 * Summary:
 * This function loads & links the module-file fully. The module-record is also
 * registered in the end.
 *
 * Args:
 * PADDRESS moduleAddress - Physical address of the loaded module
 * PADDRESS moduleSIze - Size of the module, in bytes
 * char *cmdLine - Command line option loaded for the module
 * struct ModuleRecord *record - Optional, a module record for the binary
 *
 * Version: 1
 * Since: Circuit 2.03
 * Author: Shukant Pal
 */
void ModuleLoader::loadFile(BlobRegister &blob)
{
	Void *modMemory = ModuleLoader::moveFileIntoMemory(blob);

	blob.regForm->linkerInfo = NULL;
	RecordManager::registerRecord(blob.regForm);

	blob.abiFound = ModuleLoader::globalizeDynamic(modMemory,
			*blob.regForm, blob);

	ModuleLoader::linkFile(blob.abiFound, blob);
}

///
///
/// This function loads & links a bundle of modules. Dynamic-linking is done after
/// registration of all modules, to avoid any inter-depedencies that would cause
/// a not-found linkage error.
///
/// @param LinkedList& blobList - List of blob-registers
/// @since Circuit 2.03
/// @author Shukant Pal
///
void ModuleLoader::loadBundle(LinkedList &blobList)
{
	BlobRegister *blob = (BlobRegister*) blobList.head;

	unsigned int ctr = 0;
	while(blob != NULL)
	{
		blob->fileAddr = (unsigned long)
				ModuleLoader::moveFileIntoMemory(*blob);
		blob->regForm->linkerInfo = NULL;

		RecordManager::registerRecord(blob->regForm);

		blob->abiFound = ModuleLoader::globalizeDynamic(
				(void*) blob->fileAddr, *blob->regForm, *blob);

		blob = (BlobRegister*) blob->liLinker.next;
		++ctr;
	}

	blob = (BlobRegister*) blobList.head;

	while(blob != NULL)
	{
		ModuleLoader::linkFile(blob->abiFound, *blob);
		blob = (BlobRegister*) blob->liLinker.next;
	}
}

/* @See(Module/Elf/ABI/icxxabi.h,__cxa_atexit.h,__cxa_finalize.h) */
const char *nmElf_ABI_Exitor_Func_ = "Elf::ABI::ExitorFunc (__cxa)";

/* @See(Module/Elf/ABI/icxxabi.h,__cxa_atexit.h,__cxa_finalize.h) */
ObjectInfo *tElf_ABI_ExitorFunc_;

void MdSetupLoader()
{
	tKMOD_RECORD = KiCreateType("Module::KMOD_RECORD",
			sizeof(KMOD_RECORD), sizeof(unsigned long),
			NULL, NULL);

	tElfManager = KiCreateType(nmElfManager, sizeof(ElfManager),
			sizeof(unsigned long), NULL, NULL);

	tDynamicLink = KiCreateType(nmDynamicLink, sizeof(DynamicLink),
			sizeof(unsigned long), NULL, NULL);

	tElf_ABI_ExitorFunc_ = KiCreateType(nmElf_ABI_Exitor_Func_,
			sizeof(::Elf::ABI::ExitorFunction),
			sizeof(long), NULL, NULL);
}

KMOD_RECORD *MdCreateModule(char *moduleName, unsigned long moduleVersion, unsigned long moduleType)
{
	KMOD_RECORD *mdRecord = (KMOD_RECORD*) KNew(tKMOD_RECORD, KM_SLEEP);
	memcpy(moduleName, &mdRecord->buildName, 16);
	mdRecord->buildVersion = moduleVersion;
	mdRecord->serviceType = (KM_TYPE) moduleType;

	AddElement(&mdRecord->LiLinker, &LoadedModules);
	return (mdRecord);
}
