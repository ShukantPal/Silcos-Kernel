/**
 * Copyright (C) 2017 - Shukant Pal
 */

#define NAMESPACE_IA32_IDT

#include <IA32/IDT.h>
#include <IA32/IntrHook.h>
#include <IA32/IO.h>
#include <IA32/Processor.h>
#include "../../../Interface/Utils/Memory.h"

import_asm void ExecuteLIDT(IDTPointer *);
import_asm void Spurious();
import_asm void KiClockRespond(void);
import_asm void RR_BalanceRunqueue(void);
import_asm void Executable_ProcessorBinding_IPIRequest_Invoker();
import_asm void hpetTimer();

IDTEntry defaultIDT[256];
IDTPointer defaultIDTPointer;

/*
 * Maps the given handler to the system IDT using the default flags and
 * parameters.
 *
 * @version 1.0
 * @since Circuit 2.03
 * @author Shukant Pal
 */
decl_c void MapHandler(unsigned short handlerNo, unsigned int handlerAddress,
				IDTEntry *pIDT)
{
	pIDT[handlerNo].offLow = (unsigned short)(handlerAddress & 0xFFFF);
	pIDT[handlerNo].sel = 0x08;
	pIDT[handlerNo].rfield = 0;
	pIDT[handlerNo].gateType = INTERRUPT_GATE_386;
	pIDT[handlerNo].storageSegment = 0;
	pIDT[handlerNo].dpl = 0;
	pIDT[handlerNo].present = 1;
	pIDT[handlerNo].offHigh = (unsigned short)(handlerAddress >> 16);
}

static inline void waitIO(void)
{
	/* Taken from OSDev Wiki. */
	asm volatile("jmp 1f\n\t"
			"1:jmp 2f\n\t"
			"2:");
}

/*
 * Disables the programmable interrupt controller so that the APIC subsystem
 * could be enabled and used without any side-effects.
 *
 * @author Shukant Pal
 */
decl_c void DisablePIC(void)
{
	WritePort(0xA0, 0x11);
	waitIO();
	WritePort(0x20, 0x11);
	waitIO();
	WritePort(0xA1, 0x30);
	waitIO();
	WritePort(0x21, 0x40);
	waitIO();
	WritePort(0xA1, 4);
	waitIO();
	WritePort(0x21, 2);
	waitIO();
	WritePort(0xA1, 0x1);
	waitIO();
	WritePort(0x21, 0x1);
	waitIO();
	WritePort(0xA1, 0xFF);
	WritePort(0x21, 0xFF);
}

/*
 * Initializes the interrupt-descriptor table mapping all pre-defined
 * irq-handlers and validates the pIDTPointer. It should be called only
 * once during initialization.
 *
 * @version 1.0
 * @since Circuit 2.03
 * @author Shukant Pal
 */
decl_c void MapIDT()
{
	IDTEntry *pIDT = defaultIDT;
	IDTPointer *pIDTPointer = &(defaultIDTPointer);
	memsetf(pIDT, 0, sizeof(IDTEntry) * 256);

	MapHandler(0x8, (unsigned int) &DoubleFault, pIDT);
	MapHandler(0xA, (unsigned int) &InvalidTSS, pIDT);
	MapHandler(0xB, (unsigned int) &SegmentNotPresent, pIDT);
	MapHandler(0xD, (unsigned int) &GeneralProtectionFault, pIDT);
	MapHandler(0xE, (unsigned int) &PageFault, pIDT);
	MapHandler(0xFD, (unsigned int) &Executable_ProcessorBinding_IPIRequest_Invoker, pIDT);
	MapHandler(0xFE, (unsigned int) &Spurious, pIDT);

	MapHandler(0xDD, (unsigned int) &hpetTimer, pIDT);
	MapHandler(0x20, (unsigned int) &KiClockRespond, pIDT);
	//MapHandler(0x21, (unsigned int) &RR_BalanceRunqueue, pIDT);

	pIDTPointer->Limit = (sizeof(IDTEntry) * 256) - 1;
	pIDTPointer->Base = (unsigned int) pIDT;
}

/* Part of processor initialization series */
decl_c void SetupIDT()
{
	IDTPointer *pIDTPointer = &(defaultIDTPointer);
	ExecuteLIDT(pIDTPointer);
}
