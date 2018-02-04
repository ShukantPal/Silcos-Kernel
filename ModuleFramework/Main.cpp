/**
 * File: Main.cpp
 *
 * Summary:
 * This file contains the 'initialization' & 'live' code for the framework.
 * 
 * Functions:
 * __initString - Setup string-allocation
 * __cxa_pure_virtual - ABI specific
 * KModuleMain - Init
 *
 * Copyright (C) 2017 - Shukant Pal
 */
#include <KERNEL.h>

#include <Atomic.hpp>
#include <Memory/KObjectManager.h>
#include <Heap.hxx>
#include <Util/HashMap.hpp>
#include <Util/RBTree.hpp>
#include <String.hxx>

#include <Module/Elf/ABI/Implementor.h>

#include <KERNEL.h>

ObjectInfo *tRBTree;
char nmRBTree[] = "@com.silcos.circuit.mdfrwk.RBTree";
ObjectInfo *tRBNode;
char nmRBNode[] = "@com.silcos.circuit.mdfrwk.RBTree::RBNode";
extern String *defaultString;

extern "C" void stupid()
{
	DbgLine("stupid() function called, so host is a bit stupid");
}

extern "C" void __init()
{
	__initHeap();

	tString = KiCreateType(nmString, sizeof(String), NO_ALIGN, NULL, NULL);
	tRBTree = KiCreateType(nmRBTree, sizeof(RBTree), NO_ALIGN, NULL, NULL);
	tRBNode = KiCreateType(nmRBNode, sizeof(RBNode), NO_ALIGN, NULL, NULL);

	HashMap::init();

	defaultString  = new(tString) String("@com.silcos.mdfrwk#Object");

	unsigned long a=1, b=2;
	Atomic::xchg(b, &a);
	Dbg("a: "); DbgInt(a); DbgLine(" ");
}

#include <HAL/Processor.h>
extern "C" void KModuleMain(void)
{
	DbgLine("Reporting Load: com.silcos.mdfrwk.109");

	while(1) { asm volatile("nop"); }
}
