/*=++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * File: GDT.h
 *
 * Summary: This file contans the interface to manipulate the GDTs, and also handle their entries. The
 * GDT feature is used to create a flat memory-model for each CPU. The SetupGDT() and
 * SetupDefaultBootGDT() functions are not located here. They are shifted to Processor.h for NS-conflict
 * reasons.
 *
 * Functions:
 * SetGateOn() - Used for adding a GDT entry
 *
 * Copyright (C) 2017 - Shukant Pal
 *+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=*/

#ifndef X86_GDT_H
#define X86_GDT_H

#include <Types.h>

#ifdef NAMESPACE_IA32_GDT

typedef
struct
{
	USHORT Limit;
	USHORT BaseLow;
	UCHAR BaseMiddle;
	UCHAR Access;
	UCHAR Granularity;
	UCHAR BaseHigh;
} __attribute__((__packed__)) Entry;

typedef Entry GDT_ENTRY;
typedef Entry GDTEntry;
typedef Entry GDT;

typedef 
struct Pointer {
	USHORT Limit;
	UINT Base;
} __attribute__((__packed__)) GDTPointer;

typedef GDTPointer GDT_POINTER;

VOID SetGateOn(
	USHORT gateNo,
	UINT base,
	UINT limit,
	UCHAR _access,
	UCHAR gran,
	GDT *gdt
);

#endif

#endif /* GDT.h */
