/**
 * Copyright (C) 2017 - Shukant Pal
 */

#define NAMESPACE_IA32_IDT

#include <IA32/IDT.h>
#include <IA32/IntrHook.h>
#include <IA32/IO.h>
#include <IA32/Processor.h>
#include <Util/Memory.h>

extern VOID ExecuteLIDT(IDT_POINTER *);
extern void Spurious();
extern VOID KiClockRespond(VOID);
extern VOID RR_BalanceRunqueue(VOID);

IDT defaultIDT[256];
IDT_POINTER defaultIDTPointer;

VOID MapHandler(USHORT handlerNo, UINT handlerAddress, IDT *pIDT){
	pIDT[handlerNo].LowOffset = (USHORT) (handlerAddress & 0x0000ffff);
	pIDT[handlerNo].Selector = 0x08;
	pIDT[handlerNo].ReservedSpace = 0;
	pIDT[handlerNo].GateType = INTERRUPT_GATE_386;
	pIDT[handlerNo].StorageSegment = 0;
	pIDT[handlerNo].DPL = 0;
	pIDT[handlerNo].Present = 1;
	pIDT[handlerNo].HighOffset = (USHORT) (handlerAddress >> 16);
}

static inline void IOWait(void){
    /* Taken from OSDev Wiki. */
    asm volatile ( "jmp 1f\n\t"
                   "1:jmp 2f\n\t"
                   "2:" );
}

VOID DisablePIC(){
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

VOID MapIDT(){
	IDT *pIDT = defaultIDT;
	IDT_POINTER *pIDTPointer = &(defaultIDTPointer);
	memsetf(pIDT, 0, sizeof(IDT) * 256);

	MapHandler(0x8, (UINT) &DoubleFault, pIDT);
	MapHandler(0xA, (UINT) &InvalidTSS, pIDT);
	MapHandler(0xB, (UINT) &SegmentNotPresent, pIDT);
	MapHandler(0xD, (UINT) &GeneralProtectionFault, pIDT);
	MapHandler(0xE, (UINT) &PageFault, pIDT);
	MapHandler(0xFE, (UINT) &Spurious, pIDT);
	MapHandler(0x20, (UINT) &KiClockRespond, pIDT);
	//MapHandler(0x21, (UINT) &RR_BalanceRunqueue, pIDT);

	pIDTPointer->Limit = (sizeof(IDT) * 256) - 1;
	pIDTPointer->Base = (UINT) pIDT;
}

/* Part of processor initialization series */
VOID SetupIDT(){
	IDT_POINTER *pIDTPointer = &(defaultIDTPointer);
	ExecuteLIDT(pIDTPointer);
}
