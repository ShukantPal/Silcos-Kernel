/**
 * @file Main.cpp
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
#define NAMESPACE_MAIN
#define NAMESPACE_MEMORY_MANAGER

#include <Module/Elf/ABI/Implementor.h>

#include <ACPI/RSDP.h>
#include <ACPI/RSDT.h>
#include <IA32/HAL.h>
#include <IA32/APBoot.h>
#include <HardwareAbstraction/ADM.h>
#include <HardwareAbstraction/CPUID.h>
#include <HardwareAbstraction/Processor.h>
#include <Executable/RunqueueBalancer.hpp>
#include <Executable/Thread.h>
#include <Memory/Pager.h>
#include <Arch/IA32/PageExplorer.h>
#include <Memory/KFrameManager.h>
#include <Memory/KMemoryManager.h>
#include <Memory/KMemorySpace.h>
#include <Memory/KObjectManager.h>
#include <Module/ModuleLoader.h>
#include <Module/Elf/ElfAnalyzer.hpp>
#include <Module/SymbolLookup.hpp>
#include "../../Initializer/Headers/Generic/Elf/KernelModule.h"
#include <Synch/Spinlock.h>
#include <Debugging.h>
#include <Multiboot2.h>
#include <TYPE.h>
#include <KERNEL.h>
#include <Module/Elf/ELF.h>
#include <Utils/Memory.h>
#include <Utils/Arrays.hpp>
#include <Environment.h>

using namespace HAL;
using namespace Executable;
using namespace Module;
using namespace Module::Elf;

import_asm void BSPGrantPermit();

const char *cpuIdNotSupportedError =
		"Error: 0xAAAE1: Platform does not support CPUID.";

Environment kernelParams;

void ImmatureHang(const char *dbgString){
	DbgLine(dbgString);
	while(TRUE){ asm("hlt"); }
}

extern "C" void ArchMain(); // hal -> Startup.cpp (ia32)

#include <Debugging.h>

extern "C" void WriteInteger(unsigned);

/**
 * Imports all module-ABI based information from the Initor's data
 * tables. It forms the initial ModuleContainer/Namespace tree with
 * and also locates the init-functions.
 *
 * Kernel modules are allowed to use the DT_PREINIT, DT_PREINIT_ARRAY,
 * DT_INIT, and DT_INIT_ARRAY linker entries to get init-time
 * functions invoked.
 *
 * Here, all module's init functions are called, and finally the
 * kernel environment gets fully prepared.
 *
 * @param physEntriesAddress - The physical address where the Initor
 * 						has kept the KernelModule objects.
 */
void ImportLinkerRaw(unsigned long physEntriesAddress)
{
	KernelModule *moduleEnt = (KernelModule *)
			(physEntriesAddress + KERNEL_OFFSET - kernelParams.loadAddress);
	MultibootTagModule *btldrInfo =
			MultibootChannel::getFirstModule();
	ModuleContainer *modObj;
	Module::Elf::DynamicEntry *ltable, *ent;

	kernelParams.bootModules = 0;

	while(btldrInfo != null) {
		++(kernelParams.bootModules);/* seriously, counting so late :) */

		modObj = new ModuleContainer(btldrInfo->CMD_Line, 3050);
		modObj->setBase(moduleEnt->virtualBase);

		ltable = (Module::Elf::DynamicEntry *)
				((unsigned long) moduleEnt->linkerTable
						+ KERNEL_OFFSET - kernelParams.loadAddress);

		ent = ElfAnalyzer::getLinkerEntry(DT_PREINIT_ARRAYSZ, ltable);
		if(ent != null) {
			modObj->preInitFunctorCount = ent->val / 4;
		}

		if(modObj->preInitFunctorCount == 0)
			goto SkipPreinitArray;

		ent = ElfAnalyzer::getLinkerEntry(DT_PREINIT_ARRAY, ltable);
		if(ent != null) {
			modObj->preInitArray = (void (**)())
					(moduleEnt->virtualBase + ent->ptr);
		}

		SkipPreinitArray:

		ent = ElfAnalyzer::getLinkerEntry(DT_INIT, ltable);
		if(ent != null) {
			modObj->initFunctor = (void (*)())
					(moduleEnt->virtualBase + ent->ptr);
		}

		ent = ElfAnalyzer::getLinkerEntry(DT_INIT_ARRAYSZ, ltable);
		if(ent != null) {
			modObj->initFunctorCount = ent->val / 4;
		}

		if(modObj->initFunctorCount == 0)
			goto SkipInitArray;

		ent = ElfAnalyzer::getLinkerEntry(DT_INIT_ARRAY, ltable);
		if(ent != null) {
		modObj->initArray = (void (**)())
				(moduleEnt->virtualBase + ent->ptr);
		}

		SkipInitArray:

		modObj->finiFunctor = null;
		modObj->finiArray = null;
		modObj->finiFunctorCount = 0;

		if(modObj->preInitFunctorCount)
			Arrays::invokeAll(modObj->preInitArray,
				modObj->preInitFunctorCount);

		//if(modObj->initFunctor)
		//	modObj->init(); TODO: Fix bug causing crash here!

		if(modObj->initFunctorCount)
			Arrays::invokeAll(modObj->initArray,
					modObj->initFunctorCount);

		++(moduleEnt);
		btldrInfo = MultibootChannel::getNextModule(btldrInfo);
	}
}

/**
 * Adds all the symbols of the boot-time modules in the global kernel
 * symbol table - as orphaned symbols. This avoids the need to store
 * each ModuleContainer object in an array, and access them by the
 * orderIndex listed in each KernelSymbol object (passed by the Initor
 * module).
 *
 * @param physSymbolTable
 * @param totalSymbols
 */
void ImportAllSymbols(unsigned long physSymbolTable,
		unsigned long totalSymbols)
{
	KernelSymbol *symEnt = (KernelSymbol *)
			(physSymbolTable + KERNEL_OFFSET - kernelParams.loadAddress);

	for(unsigned int symidx = 0;
			symidx < totalSymbols; symidx++, symEnt++) {
		globalKernelSymbolTable->add(symEnt->virtualAddress,
				symEnt->name + KERNEL_OFFSET - kernelParams.loadAddress);
	}
}

/**
 * Initializes basic environmental services before calling the main
 * function. This includes page-protection, constructors (not impl),
 * and other stuff (nothing actually).
 *
 * @param retAddr
 * @param envBase
 * @param envSize
 */
export_asm void EarlyMain(unsigned long retAddr, unsigned long envBase,
		unsigned long envSize)
{
	unsigned long autodat = envBase + envSize - AUTO_DAT;

	U64 *globalTable = (U64*)(autodat);
	U64 *globalDirectory = (U64*)(autodat + PAGE);
	U64 *bootPDPT = (U64*)(autodat + 2 * PAGE);

	memsetf(bootPDPT, 0, 32);
	memsetf(globalDirectory, 0, PAGE);
	memsetf(globalTable, 0, PAGE);

	Pager::init(retAddr, envBase, globalTable,
			globalDirectory, bootPDPT);
}

/**
 * Calls init functions for each subsystem, eventually setting up all
 * basic services. It passes control to the hardware-abstraction module
 * to initialize basic-hardware services.
 *
 * @param bootTable - The physical address of the multiboot-information
 * 					table passed by the bootloader.
 * @param bootKernelBase - The physical address where the kernel was loaded
 * 					by the initor module.
 * @param bootKernelSize - The size of the total text, data copied by the
 * 					initor module, including symbols, strings, and hashes.
 * @param globalSymbols - A central symbol-table holding entries for all
 * 					global symbols in the kernel modules.
 * @param kernelStrings - A table holding addresses of string tables
 * 					of each kernel module, in physical memory.
 * @param globalSymbolHash - A central "standard (elf)" hash-table for
 * 					accessing the symbols in the global symbol table by
 * 					their name, using the standard hashing function
 * 					defined by the ELF standard.
 * @param pmoduleEntries - Physical address of KernelModule objects
 * 					holding information regarding each module
 */
export_asm void Main(unsigned long bootTable, unsigned long bootKernelBase,
		unsigned long bootKernelSize, unsigned long globalSymbols,
		unsigned long totalSymbols, unsigned long kernelStrings,
		void *globalSymbolHash, unsigned long pmoduleEntries)
{
	InitConsole(reinterpret_cast<U8*>(CONSOLE));
	DbgLine("Reporting Control: \"KernelHost\" @Silcos.KernHost.3050");

	unsigned long autodat = KERNEL_OFFSET + bootKernelSize - AUTO_DAT;
	Pager::init2((U64 *) autodat + 512, (U64 *) autodat,
			bootKernelBase + bootKernelSize - AUTO_DAT + PAGE * 2);
	MultibootChannel::init(bootTable);

	kernelParams.loadAddress = bootKernelBase;
	kernelParams.loadSize = bootKernelSize;
	kernelParams.pageframeEntries = getHugePagesSize(bootKernelBase + bootKernelSize);
	kernelParams.multibootTable = bootTable;

	SetupKFrameManager();
	SetupKMemoryManager();
	obSetupAllocator();
	SetupPrimitiveObjects();
	__initHeap();
	MdSetupLoader();
	ImportLinkerRaw(pmoduleEntries);
	ImportAllSymbols(globalSymbols, totalSymbols);

	ArchMain();
}
