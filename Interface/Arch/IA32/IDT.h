/* @file IDT.h
 *
 * Copyright (C) 2017 - Shukant Pal
 */
#ifndef HAL_IA32_IDT_H__
#define HAL_IA32_IDT_H__

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

/*
 * An IDT-entry is used for controlling how interrupts are handled. It
 * is used for mapping and masking irq-handlers.
 *
 * @version 1.0
 * @since Circuit 2.01
 * @author Shukant Pal
 */
struct IDTEntry
{
	unsigned short offLow;
	unsigned short sel;
	unsigned char rfield;
	unsigned char gateType		: 4;
	unsigned char storageSegment	: 1;
	unsigned char dpl		: 2;
	unsigned char present		: 1;
	unsigned short offHigh;
} __attribute__((__packed__));

struct IDTPointer {
	unsigned short Limit;
	unsigned int Base;
} __attribute__((__packed__));

#else
	struct IDTEntry;
#endif/* NAMESPACE_IDT */

decl_c void MapHandler(unsigned short handlerNo, unsigned int handlerAddress,
				IDTEntry *pIDT) kxhide;

#endif/* IA32/IDT.h */
