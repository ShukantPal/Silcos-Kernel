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
#include <Module/SymbolLookup.hpp>
#include <Module/Elf/ABI/icxxabi.h>
#include <Module/Elf/ELF.h>
#include <Module/Elf/ElfManager.hpp>
#include <Module/Elf/ElfAnalyzer.hpp>
#include <Module/Elf/ElfLinker.hpp>
#include <Utils/Arrays.hpp>
#include <KERNEL.h>

using namespace Module;
using namespace Module::Elf;

extern "C" void elf_dbg() { Dbg("ELF PROGRAME CALLS MK");}

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

///
/// Dynamically allocates memory to store the elf-object's file so that
/// it could be mapped and used.
///
/// @param blob - the registration "form" of the module
///
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

	void *memoryArena = (void *) KiPagesAllocate(
			HighestBitSet(fileSize >> KPGOFFSET), ZONE_KMODULE,
			FLG_NONE);

	Pager::mapAll((unsigned long) memoryArena, blob.loadAddr,
			fileSize, FLG_ATOMIC, PRESENT | READ_WRITE);

	memoryArena += blob.loadAddr % KPGSIZE;
	return (memoryArena);
}

static inline void holdPreinitArray(ElfManager *em, ModuleContainer *mc)
{
	DynamicEntry *pinitAddr = em->getDynamicEntry(DT_PREINIT_ARRAY);
	DynamicEntry *pinitCount = em->getDynamicEntry(DT_PREINIT_ARRAYSZ);

	if(pinitAddr && pinitCount)
	{
		mc->preInitArray = (void (**)())(pinitAddr->ptr +
				mc->getBase());
		mc->preInitFunctorCount = pinitCount->val / sizeof(void(**)());
	}
}

static inline void holdInitArray(ElfManager *em, ModuleContainer *mc)
{
	DynamicEntry *initArrayAddr = em->getDynamicEntry(DT_INIT_ARRAY);
	DynamicEntry *initArraySize = em->getDynamicEntry(DT_INIT_ARRAYSZ);

	if(initArrayAddr && initArraySize)
	{
		mc->initArray = (void (**)())(initArrayAddr->ptr +
				mc->getBase());
		mc->initFunctorCount = initArraySize->val / sizeof(void(**)());
	}
}

static inline void holdFiniArray(ElfManager *em, ModuleContainer *mc)
{
	DynamicEntry *finiArrayAddr = em->getDynamicEntry(DT_FINI_ARRAY);
	DynamicEntry *finiArraySize = em->getDynamicEntry(DT_FINI_ARRAYSZ);

	if(finiArrayAddr && finiArraySize)
	{
		mc->finiArray = (void (**)())(finiArrayAddr->ptr +
				mc->getBase());
		mc->finiFunctorCount = finiArraySize->val / sizeof(void(**)());
	}
}

///
/// Exports all dynamic-linking information gathered from the elf-abi
/// handling objects, notes down init, main, fini functors, and then exports
/// all symbolic definitions to the ptp pool.
///
/// @param moduleMemory
/// @param kmRecord
/// @param blob
/// @return - the ABI of the given module
///
ABI ModuleLoader::globalizeDynamic(void *moduleMemory, BlobRegister &blob)
{
	if(!ElfAnalyzer::validateBinary(moduleMemory))
		return (ABI::INVALID);

	ElfManager *obfHdlr = new(tElfManager) ElfManager((ElfHeader*)
			moduleMemory);
	DynamicLink *linkHandle = obfHdlr->exportDynamicLink();
	ModuleContainer *mc = blob.fileBox;

	mc->setLinker(linkHandle);
	mc->setBase(obfHdlr->baseAddress);

	if(obfHdlr->binaryHeader->entryAddress)
	{
		mc->kMain = reinterpret_cast<void (*)()>(mc->getBase() +
				obfHdlr->binaryHeader->entryAddress);
	}

	DynamicEntry* dynamicIniter = obfHdlr->getDynamicEntry(DT_INIT);
	if(dynamicIniter)
	{
		mc->initFunctor = reinterpret_cast<void (*)()>(mc->getBase() +
				dynamicIniter->ptr);
	}
	else
	{
		Symbol *__initer = obfHdlr->getStaticSymbol("__init");
		if(__initer)
		{
			mc->initFunctor = reinterpret_cast<void (*)()>(
					mc->getBase() + __initer->value);
		}
	}

	DynamicEntry* dynamicFinier = obfHdlr->getDynamicEntry(DT_FINI);
	if(dynamicFinier)
	{
		mc->initFunctor = reinterpret_cast<void (*)()>(mc->getBase() +
				dynamicFinier->ptr);
	}

	holdPreinitArray(obfHdlr, mc);
	holdInitArray(obfHdlr, mc);
	holdFiniArray(obfHdlr, mc);

	blob.manager = obfHdlr;
	mc->ptpResolvableLinks->addAll(linkHandle->dynamicSymbols, mc);

	return (ABI::ELF);
}

///
/// Finishes the linking part of the module by filling in the relocation
/// entries.
///
/// @param binaryIfc - binary-interface of the given module
/// @param blob - the blob-struct for this module
///
void ModuleLoader::linkFile(ABI binaryIfc, BlobRegister& blob)
{
	ElfManager *manager;
	switch(binaryIfc)
	{
	case ABI::ELF:
		manager = (ElfManager*) blob.manager;
		ElfLinker::resolveLinkage(*manager, blob.fileBox);
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
 * PhysAddr moduleAddress - Physical address of the loaded module
 * PhysAddr moduleSIze - Size of the module, in bytes
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

	blob.abiFound = ModuleLoader::globalizeDynamic(modMemory, blob);

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

		blob->abiFound = ModuleLoader::globalizeDynamic(
				(void*) blob->fileAddr, *blob);

		if(blob->abiFound == ABI::INVALID)
		{
			DbgLine("Invalid boot-module given ");
			while(TRUE);
		}

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

///
/// Calls the initialization functions of the kernel module, according to
/// the specific ABI.
///
/// By default, the ELF ABI defines the __preinit array, _init() function and
/// the __init array to be called in the given sequence. It is done so here,
/// except that they are stored in the ModuleContainer while linking, and
/// hence, the symbol lookup is not required.
///
/// ---------------------------------------------------------------------------
///
/// Note on declaring C initialization functions to allocate new object types
/// for the slab allocator:
///
/// Declaring functions in the __preinit array is not supported by GCC, as of
/// my knowlegde in code, therefore, function which just allocate primitive
/// types in the slab allocator must be declared as constructors with higher
/// priority. By kernel-standard, this priority value is 100.
///
/// Note on stability of this sequence:
///
/// It is seen that this part of the kernel crashes quite a bit on adding new
/// features and all. This may be because of some ELF-support leftover. You
/// could try to add a __while_true at the end to check whether the crash
/// occurs here.
///
/// @param bl - Reference to the BlobRegister of the module given, to access
/// 		its module container and its ABi.
/// @version - 5.1
/// @author Shukant Pal
///
void ModuleLoader::init(BlobRegister &bl)
{
	ModuleContainer *mc = bl.fileBox;

	switch(bl.abiFound)
	{
	case ELF:
		if(mc->preInitFunctorCount != 0)
			Arrays::invokeAll(mc->preInitArray,
					mc->preInitFunctorCount);

		if(mc->initFunctor != null)
			mc->initFunctor();

		if(mc->initFunctorCount != 0 && mc->initFunctorCount != 0)
			Arrays::invokeAll(mc->initArray,
					mc->initFunctorCount);
		break;
	}
}

/* @see(Module/Elf/ABI/icxxabi.h,__cxa_atexit.h,__cxa_finalize.h) */
const char *nmElf_ABI_Exitor_Func_ = "Elf::ABI::ExitorFunc (__cxa)";

/* @see(Module/Elf/ABI/icxxabi.h,__cxa_atexit.h,__cxa_finalize.h) */
ObjectInfo *tElf_ABI_ExitorFunc_;

void MdSetupLoader()
{
	tElfManager = KiCreateType(nmElfManager, sizeof(ElfManager),
			sizeof(unsigned long), NULL, NULL);

	tDynamicLink = KiCreateType(nmDynamicLink, sizeof(DynamicLink),
			sizeof(unsigned long), NULL, NULL);

	tElf_ABI_ExitorFunc_ = KiCreateType(nmElf_ABI_Exitor_Func_,
			sizeof(::Elf::ABI::ExitorFunction),
			sizeof(long), NULL, NULL);

	globalKernelSymbolTable = new SymbolLookup();
	defPtpGroup = new SymbolLookup();
}
