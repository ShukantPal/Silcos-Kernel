/**
 * Copyright (C) 2017 - Shukant Pal
 *
 * Modify this file as per you wish, to adapt the kernel to your OS.
 */
#include <Module/Elf/ElfManager.hpp>
#include <Module/ModuleRecord.h>
#include <Module/ModuleLoader.h>
#include <Memory/Pager.h>
#include <KERNEL.h>
#include <Multiboot2.h>
#include <Module/MSI.h>

using namespace Module;

void bsp_main(){
	DbgLine("thread_main");
	while(TRUE) { asm volatile("nop"); }
}

extern "C" void stupid();
import_asm void BSPGrantPermit();

void Init() {
	DbgLine("Hello\nThis is a newline. ");
	BSPGrantPermit();

	while(TRUE) { asm volatile("nop"); }
}
