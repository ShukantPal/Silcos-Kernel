/**
 * File: ModuleLoader.cpp
 *
 * Summary:
 * This files implements the so-called cross-ABI module-loader.
 *
 * Author: Shukant Pal
 */
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

decl_c VOID elf_dbg() { Dbg("ELF PROGRAME CALLS MK");}
CHAR elf_data;

ObjectInfo *tKMOD_RECORD;

CHAR nmElfManager[] = "Module::Elf::ElfManager";
CHAR nmDynamicLink[] = "Module::DynamicLink";

/*
 * ElfManager objects are allocated dynamically
 */
struct ObjectInfo *tElfManager;// class ElfManager - Slab-Allocator

/*
 * Dynamic-linker objects are allocated for registration
 */
struct ObjectInfo *tDynamicLink;// struct DynamicLink - Slab-Allocator

LINKED_LIST LoadedModules;

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
void *ModuleLoader::moveFileIntoMemory(
		BlobRegister &blob
){
	blob.blobSize = NextPowerOf2(blob.blobSize);
	ULONG fileSize = blob.blobSize;

	#ifdef ARCH_32
		return_if(fileSize > MB(2), FALSE);
	#else // ARCH_64
		return_if(fileSize > MB(4), FALSE);
	#endif

	Void *memoryArena = (VOID *) KiPagesAllocate(HighestBitSet(fileSize >> KPGOFFSET), ZONE_KMODULE, FLG_NONE);
	EnsureAllMappings((ULONG) memoryArena, blob.loadAddr, blob.blobSize, NULL, PRESENT | READ_WRITE);

	return (memoryArena);
}

/**
 * Function: ModuleLoader::globalDynamic
 *
 * Summary:
 * Exports the dynamic-symbols for the module
 *
 * Args:
 * void *moduleMemory - Module's file in the virtual memory
 * ModuleRecord& kmRecord - Record of the module, as filled by client
 * BlobRegister& blob - Register for blob-information
 *
 * Author: Shukant Pal
 */
ABI ModuleLoader::globalizeDynamic(
		void *moduleMemory,
		ModuleRecord& kmRecord,
		BlobRegister &blob
){
	if(ElfAnalyzer::validateBinary(moduleMemory)){
		ElfManager *moduleHandler = new(tElfManager) ElfManager((ElfHeader*) moduleMemory);
		DynamicLink *linkHandle = moduleHandler->exportDynamicLink();

		kmRecord.linkerInfo = linkHandle;
		kmRecord.BaseAddr = moduleHandler->baseAddress;

		if(moduleHandler->binaryHeader->entryAddress)
			kmRecord.entryAddr =
					kmRecord.BaseAddr + moduleHandler->binaryHeader->entryAddress;

		blob.manager = moduleHandler;

		return (ABI::ELF);
	} else
		return (ABI::INVALID);
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
void ModuleLoader::linkFile(
		ABI binaryIfc,
		BlobRegister& blob
){
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
 * CHAR *cmdLine - Command line option loaded for the module
 * struct ModuleRecord *record - Optional, a module record for the binary
 *
 * Version: 1
 * Since: Circuit 2.03
 * Author: Shukant Pal
 */
void ModuleLoader::loadFile(
		BlobRegister &blob
){
	Void *modMemory = ModuleLoader::moveFileIntoMemory(blob);

	blob.regForm->linkerInfo = NULL;
	RecordManager::registerRecord(blob.regForm);

	blob.abiFound =
			ModuleLoader::globalizeDynamic(modMemory, *blob.regForm, blob);

	ModuleLoader::linkFile(blob.abiFound, blob);
}

/**
 * Function: ModuleLoader::loadBundle
 *
 * Summary:
 * This function loads & links a bundle of modules. Dynamic-linking is done after
 * registration of all modules, to avoid any inter-depedencies that would cause
 * a not-found linkage error.
 *
 * Args:
 * LinkedList& blobList - List of blob-registers
 *
 * Author: Shukant Pal
 */
void ModuleLoader::loadBundle(
		LinkedList &blobList
){
	BlobRegister *blob = (BlobRegister*) blobList.Head;

	while(blob != NULL){
		blob->fileAddr = (ULONG) ModuleLoader::moveFileIntoMemory(*blob);
		blob->regForm->linkerInfo = NULL;
		RecordManager::registerRecord(blob->regForm);
		blob->abiFound =
			ModuleLoader::globalizeDynamic((void*) blob->fileAddr, *blob->regForm, *blob);

		blob = (BlobRegister*) blob->liLinker.Next;
	}

	blob = (BlobRegister*) blobList.Head;

	while(blob != NULL){
		ModuleLoader::linkFile(blob->abiFound, *blob);
		blob = (BlobRegister*) blob->liLinker.Next;
	}
}

/* @See(Module/Elf/ABI/icxxabi.h,__cxa_atexit.h,__cxa_finalize.h) */
const char *nmElf_ABI_Exitor_Func_ = "Elf::ABI::ExitorFunc (__cxa)";

/* @See(Module/Elf/ABI/icxxabi.h,__cxa_atexit.h,__cxa_finalize.h) */
ObjectInfo *tElf_ABI_ExitorFunc_;

VOID MdSetupLoader()
{
	tKMOD_RECORD = KiCreateType("Module::KMOD_RECORD", sizeof(KMOD_RECORD),
					sizeof(ULONG), NULL, NULL);

	tElfManager = KiCreateType(nmElfManager, sizeof(ElfManager),
					sizeof(ULONG), NULL, NULL);

	tDynamicLink = KiCreateType(nmDynamicLink, sizeof(DynamicLink),
					sizeof(unsigned long), NULL, NULL);

	tElf_ABI_ExitorFunc_ = KiCreateType(nmElf_ABI_Exitor_Func_,
				sizeof(::Elf::ABI::ExitorFunction),
				sizeof(long), NULL, NULL);
}

KMOD_RECORD *MdCreateModule(CHAR *moduleName, ULONG moduleVersion, ULONG moduleType){
	KMOD_RECORD *mdRecord = (KMOD_RECORD*) KNew(tKMOD_RECORD, KM_SLEEP);
	memcpy(moduleName, &mdRecord->buildName, 16);
	mdRecord->buildVersion = moduleVersion;
	mdRecord->serviceType = (KM_TYPE) moduleType;

	AddElement(&mdRecord->LiLinker, &LoadedModules);
	return (mdRecord);
}
