
///
/// @file Main.cpp
///
/// Early boot-up code is written in this file after the InitRuntime.asm
/// code. Here logical initialization of subsystems is done and basic
/// boot-time checking is done for system compatiblity.
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
#include <Memory/KFrameManager.h>
#include <Memory/KMemoryManager.h>
#include <Memory/KMemorySpace.h>
#include <Memory/KObjectManager.h>
#include <Module/ModuleLoader.h>
#include <Module/SymbolLookup.hpp>
#include <Module/MSI.h>
#include <Synch/Spinlock.h>
#include <Debugging.h>
#include <Multiboot2.h>
#include <TYPE.h>
#include <KERNEL.h>
#include <Module/Elf/ELF.h>
#include <Utils/Memory.h>

using namespace HAL;
using namespace Executable;

import_asm void BSPGrantPermit();

const char *cpuIdNotSupportedError = "Error: 0xAAAE1: Platform does not support CPUID.";/* CPUID test returns zero value, indicating failure */

void ImmatureHang(const char *dbgString){
	DbgLine(dbgString);
	while(TRUE){ asm("hlt"); }
}

extern "C" void ArchMain(); // hal -> Startup.cpp (ia32)
extern "C" void InitAllObjects(); // kernhost -> IA32/Boot/InitRuntime.asm

///
/// This is the logical entry point of the silcos kernel. The KernelHost
/// initializes itself by calling setup-functions for each internal subsystem
/// and then loads the boot-modules. Once all modules are linked & init'ed
/// then the HAL gets the chance to continue.
///
/// @param multibootTable - physical address of the multi-boot table passed by
/// 			    the boot loader in the EBX register
/// @param magicNo - magic no. passed by the boot loader to identify itself as
/// 		     multiboot-compliant.
/// @version 15.12
/// @since Circuit 1.98
/// @author Shukant Pal
///
export_asm void Main(U32 multibootTable, U32 magicNo)
{
	DbgLine("Reporting Load: @(com.silcos.circuit.2030)\t--- Silcos Kernel 2.05! ---");

	if(magicNo != MULTIBOOT2_BOOTLOADER_MAGIC){
		DbgLine("Error : Multiboot-compliant bootloader not found!");
		DbgLine("Please install a multiboot-compliant bootloader, e.g. GRUB2");
		asm volatile("hlt;");
	}

	SetupKFrameManager();
	SetupKMemoryManager();
	obSetupAllocator();
	SetupPrimitiveObjects();
	__initHeap();
	InitAllObjects();

	MdSetupLoader();
	KernelElf::loadBootModules();

	ArchMain();
}
