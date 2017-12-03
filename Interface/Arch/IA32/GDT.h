/**
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
 */
#ifndef IA32_GDT_H__
#define IA32_GDT_H__

#include <Types.h>

#ifdef NAMESPACE_IA32_GDT

struct GDTEntry
{
	unsigned short limit;
	unsigned short baseLow;
	unsigned char baseMiddle;
	unsigned char access;
	unsigned char granularity;
	unsigned char baseHigh;
} __attribute__((__packed__));

struct GDTPointer {
	unsigned short Limit;
	unsigned int Base;
} __attribute__((__packed__));

typedef GDTPointer GDT_POINTER;

void SetGateOn(unsigned short gateNo, unsigned int base, unsigned int limit,
		unsigned char access, unsigned char gran, GDTEntry *gdt);

#endif// NAMESPACE_IA32_GDT

#endif/* GDT.h */
