/**
 * Copyright (C) 2017 - Shukant Pal
 */
#ifndef IA32_IDT_H
#define IA32_IDT_H

#ifdef NAMESPACE_IA32_IDT

#include <KERNEL.h>

enum GateType
{
	TASK_GATE_286 = 0x5,
	INTERRUPT_GATE_286 = 0x6,
	TRAP_GATE_286 = 0x7,
	INTERRUPT_GATE_386 = 0xE,
	TRAP_GATE_386 = 0xF
};

struct IDTEntry
{
	unsigned short LowOffset;	
	unsigned short Selector;
	unsigned char ReservedSpace;
	unsigned char GateType : 4;
	unsigned char StorageSegment : 1;
	unsigned char DPL : 2;
	unsigned char Present : 1;
	unsigned short HighOffset;
} __attribute__((__packed__));

struct IDTPointer {
	unsigned short Limit;
	unsigned int Base;
} __attribute__((__packed__));

#else
	struct IDTEntry;
#endif/* NAMESPACE_IDT */

extern "C" void MapHandler(
	unsigned short handlerNo,
	unsigned int handlerAddress,
	IDTEntry *pIDT
) kxhide;

#endif /* IA32/IDT.h */
