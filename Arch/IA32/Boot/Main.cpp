/*=++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * File: Main.c
 *
 * Summary:
 * This file contains the initialization code for the IA32 platform. Internal kernel
 * subsystems are setup before waking all other CPUs and preparing SMP in the system.
 *
 * CPUID support is implemented for getting all model-specific information on the
 * system. This allows for the flexibility of the software to bend against changes
 * in the models of a CPU in the architecture.
 *
 *
 * Functions:
 * Main() - Everything starts from here.
 *
 * Copyright (C) 2017 - Shukant Pal
 *+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=*/
 
#define NAMESPACE_MAIN
#define NAMESPACE_MEMORY_MANAGER

#include <ACPI/RSDP.h>
#include <ACPI/RSDT.h>
#include <IA32/HAL.h>
#include <IA32/APBoot.h>
#include <HAL/ADM.h>
#include <HAL/CPUID.h>
#include <HAL/Processor.h>
#include <Exec/Process.h>
#include <Exec/Thread.h>
#include <Memory/Pager.h>
#include <Memory/KFrameManager.h>
#include <Memory/KMemoryManager.h>
#include <Memory/KMemorySpace.h>
#include <Memory/KObjectManager.h>
#include <Util/Memory.h>
#include <Synch/Spinlock.h>
#include <Multiboot.h>
#include <Debugging.h>
#include <Multiboot2.h>
#include <TYPE.h>
#include <KERNEL.h>
#include <Module/Elf/ELF.h>

extern void SetupTick();
import_asm void BSPGrantPermit();

CHAR *cpuIdNotSupportedError = "Error: 0xAAAE1: Platform does not support CPUID.";/* CPUID test returns zero value, indicating failure */

VOID ImmatureHang(CHAR *dbgString){
	DbgLine(dbgString);
	while(TRUE){ asm("hlt"); }
}

PROCESSOR_SETUP_INFO *dInfo;
/******************************************************************************
 * Function: ValidateSupport()
 *
 * Summary: This function validates the required features to run the kernel, by checking for
 * CPUID support and then searching for the list features.
 *
 * @Version 1
 * @Since Circuit 2.03
 ******************************************************************************/
VOID ValidateSupport(){
	if(TestCPUID() != 0) {
		// TODO: Implement CPUID feature searching through ADM subset.
	} else
		ImmatureHang(cpuIdNotSupportedError);
}

/**
 * Function: Main()
 *
 * Summary:
 * This is the logical entry point, for the kernel, in the C language. From here, you can see
 * the early initialization steps for the kernel. Serially, it setups the following subsystems -
 *
 * 1. Console & Debugger
 * 2. Multiboot Check
 * 3. Physical Memory Allocator
 * 4. Virtual Memory Context
 * 5. RSDP & RSDT (ACPI Tables)
 * 6. Processor Management
 * 7. Interrupts & System Calls
 * 8. Threads & Process Init
 * 9. Scheduler Enable
 *
 * At the end, this function returns and after one timer tick, the scheduler starts the first
 * kernel thread, which does further system initialization.
 *
 * Args:
 * bootInfo - Multiboot header, for initialization data
 * magicNo - Conformatory number, to check for multiboot loader
 *
 * Returns: VOID
 *
 * @See Init.c, InitRuntime.asm
 * @Version 13
 * @Since Circuit 1.02
 */
export_asm void Main(U32 bootInfo, U32 magicNo)
{
	DbgLine("Circuit 2.03 Initializing");

	if(magicNo != MULTIBOOT2_BOOTLOADER_MAGIC){
		DbgLine("Error : Multiboot-compliant bootloader not found!");
		DbgLine("Please install a multiboot-compliant bootloader, e.g. GRUB2");
		asm volatile("hlt;");
	}

	SetupKFrameManager();
	SetupKMemoryManager();
	obSetupAllocator();

	ScanRsdp();
	SetupRsdt();
	MapAPIC();
	SetupBSP();

	/*
	 * In object-allocator, setup the utility objects (linked-list, avl-tree, etc.)
	 * now because process-lookup table is ready (kobject-allocator requires it)
	 */
	SetupPrimitiveObjects();

	InitPTable();
	InitTTable();

	SetupAPs();
	SetupTick();

	BSPGrantPermit();

	EraseIdentityPage();
}
