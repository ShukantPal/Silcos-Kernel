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

#include <Memory/KObjectManager.h>
#include <Heap.hxx>
#include <Util/HashMap.hxx>
#include <Util/RBTree.hxx>
#include <String.hxx>

#include <Module/Elf/ABI/Implementor.h>

ObjectInfo *tRBTree;
char nmRBTree[] = "@com.silcos.circuit.mdfrwk.RBTree";
ObjectInfo *tRBNode;
char nmRBNode[] = "@com.silcos.circuit.mdfrwk.RBTree::RBNode";
extern String *defaultString;

void* __dso_handle;

//using namespace Util;

static inline void __init()
{
	tString = KiCreateType(nmString, sizeof(String), NO_ALIGN, NULL, NULL);
	tRBTree = KiCreateType(nmRBTree, sizeof(RBTree), NO_ALIGN, NULL, NULL);
	tRBNode = KiCreateType(nmRBNode, sizeof(RBNode), NO_ALIGN, NULL, NULL);

	HashMap::init();

	defaultString  = new(tString) String("@com.silcos.mdfrwk#Object");
}

decl_c void __cxa_pure_virtual()
{
	Dbg("_verr");
}

decl_c void KModuleMain(void)
{
	DbgLine("Reporting Load: com.silcos.obmgr.109");
	__initHeap();
	__init();

	while(1) { asm volatile("nop"); }
}
