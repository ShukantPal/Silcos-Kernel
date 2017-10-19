/**
 * Copyright (C) 2017 - Shukant Pal
 *
 * Modify this file as per you wish, to adapt the kernel to your OS.
 */

#define AI_MODLOADER
#define AI_MULTIBOOT2

#include <Memory/Pager.h>
#include <Memory/PMemoryManager.h>
#include <KERNEL.h>

#include <Module/MSI.h>

VOID bsp_main(){
	DbgLine("thread_main");
	while(TRUE) { asm volatile("nop"); }
}


VOID Init() {
	DbgLine("Loading Executive Modules...");

	MdSetupLoader();

	MsiSetupKernelForLinking();

	MULTIBOOT_TAG_MODULE *muModule = SearchMultibootTag(MULTIBOOT_TAG_TYPE_MODULE);
	MdLoadFile(muModule->ModuleStart, muModule->ModuleEnd-muModule->ModuleStart, &muModule->CMDLine[0]);

	while(TRUE)
		asm volatile("nop;");
}
