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

#include <KERNEL.h>

ObjectInfo *tRBTree;
char nmRBTree[] = "@com.silcos.circuit.mdfrwk.RBTree";
ObjectInfo *tRBNode;
char nmRBNode[] = "@com.silcos.circuit.mdfrwk.RBTree::RBNode";
extern String *defaultString;

extern "C" void __init()
{
	__initHeap();

	tString = KiCreateType(nmString, sizeof(String), NO_ALIGN, NULL, NULL);
	tRBTree = KiCreateType(nmRBTree, sizeof(RBTree), NO_ALIGN, NULL, NULL);
	tRBNode = KiCreateType(nmRBNode, sizeof(RBNode), NO_ALIGN, NULL, NULL);

	HashMap::init();

	defaultString  = new(tString) String("@com.silcos.mdfrwk#Object");
}

#include <Process/MemoryImage.hpp>

using namespace Process;
using namespace Resource;

/*
 * Test case for resource-manager
 */

void test_rsmgr()
{
	Process::MemoryImage *mem = Process::MemoryImage::getImage();

	mem->insertRegion(KB(64), 4, GetConfigFlags(MemorySection::Type::Code, 0), 0);
	mem->insertRegion(KB(84), 1, GetConfigFlags(MemorySection::Type::Data, 0), 0);
	mem->insertRegion(KB(92), 2, GetConfigFlags(MemorySection::Type::Code, 0), 0);

	DbgErro(mem->removeRegion(KB(64), 8, MemorySection::Type::Any), 16);

	DbgLine("__printal");
	mem->printAll();
}

extern "C" void KModuleMain(void)
{
	DbgLine("Reporting Load: com.silcos.mdfrwk.109");
	test_rsmgr();

	while(1) { asm volatile("nop"); }
}
