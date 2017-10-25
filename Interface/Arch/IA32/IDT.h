/**
 * Copyright (C) 2017 - Shukant Pal
 */

#ifndef IA32_IDT_H
#define IA32_IDT_H

#ifdef NAMESPACE_IA32_IDT

#include <TYPE.h>

enum GateType
{
	TASK_GATE_286 = 0x5,
	INTERRUPT_GATE_286 = 0x6,
	TRAP_GATE_286 = 0x7,
	INTERRUPT_GATE_386 = 0xE,
	TRAP_GATE_386 = 0xF
};

typedef
struct IDTEntry
{
	USHORT LowOffset;	
	USHORT Selector;
	UCHAR ReservedSpace;
	UCHAR GateType : 4;
	UCHAR StorageSegment : 1;
	UCHAR DPL : 2;
	UCHAR Present : 1;
	USHORT HighOffset;
} __attribute__((__packed__)) IDT;

typedef
struct _IDT_POINTER {
	USHORT Limit;
	UINT Base;
} __attribute__((__packed__)) IDT_POINTER;

#else
	struct _IDT_ENTRY;
	typedef struct _IDT_ENTRY IDT;
#endif /* NAMESPACE_IDT */

VOID MapHandler(
	USHORT handlerNo,
	UINT handlerAddress,
	IDT *pIDT
);

#endif /* IA32/IDT.h */
