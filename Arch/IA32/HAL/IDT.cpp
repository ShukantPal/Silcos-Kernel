/**
 * Copyright (C) 2017 - Shukant Pal
 */

#define NAMESPACE_IA32_IDT

#include <IA32/IDT.h>
#include <IA32/IntrHook.h>
#include <IA32/IO.h>
#include <IA32/Processor.h>
#include <Util/Memory.h>

import_asm void ExecuteLIDT(IDTPointer *);
import_asm void Spurious();
import_asm void KiClockRespond(void);
import_asm void RR_BalanceRunqueue(void);
import_asm void Executable_ProcessorBinding_IPIRequest_Invoker();

IDTEntry defaultIDT[256];
IDTPointer defaultIDTPointer;

void MapHandler(unsigned short handlerNo, unsigned int handlerAddress, IDTEntry *pIDT)
{
	pIDT[handlerNo].LowOffset = (unsigned short) (handlerAddress & 0x0000ffff);
	pIDT[handlerNo].Selector = 0x08;
	pIDT[handlerNo].ReservedSpace = 0;
	pIDT[handlerNo].GateType = INTERRUPT_GATE_386;
	pIDT[handlerNo].StorageSegment = 0;
	pIDT[handlerNo].DPL = 0;
	pIDT[handlerNo].Present = 1;
	pIDT[handlerNo].HighOffset = (unsigned short) (handlerAddress >> 16);
}

static inline void IOWait(void){
    /* Taken from OSDev Wiki. */
    asm volatile ( "jmp 1f\n\t"
                   "1:jmp 2f\n\t"
                   "2:" );
}

void DisablePIC()
{
	WritePort(0xA0, 0x11);
	IOWait();
	WritePort(0x20, 0x11);
	IOWait();
	WritePort(0xA1, 0x30);
	IOWait();
	WritePort(0x21, 0x40);
	IOWait();
	WritePort(0xA1, 4);
	IOWait();
	WritePort(0x21, 2);
	IOWait();
	WritePort(0xA1, 0x1);
	IOWait();
	WritePort(0x21, 0x1);
	IOWait();
	WritePort(0xA1, 0xFF);
	WritePort(0x21, 0xFF);
}

extern "C" void MapIDT()
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
	MapHandler(0x20, (unsigned int) &KiClockRespond, pIDT);
	//MapHandler(0x21, (unsigned int) &RR_BalanceRunqueue, pIDT);

	pIDTPointer->Limit = (sizeof(IDTEntry) * 256) - 1;
	pIDTPointer->Base = (unsigned int) pIDT;
}

/* Part of processor initialization series */
extern "C" void SetupIDT()
{
	IDTPointer *pIDTPointer = &(defaultIDTPointer);
	ExecuteLIDT(pIDTPointer);
}
