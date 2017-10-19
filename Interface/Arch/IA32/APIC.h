/*=++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * File: APIC.h
 *
 * Summary: This file contains the interface for using the APIC and management of interrupts -
 * recieving and sending interrupts, handling IPIs, mapping system calls and much more!
 *
 * Functions:
 *
 * Copyright (C) 2017 - Shukant Pal
 *+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=*/
#ifndef IA32_APIC_H
#define IA32_APIC_H

#include "MSR.h"
#include <Memory/Pager.h>
#include <Memory/KMemorySpace.h>
#include <TYPE.h>

#define DefaultAPICBase 0xFEE00000

/* APIC register relative offsets */
#define APIC_REGISTER_ID 		0x20
#define APIC_REGISTER_VERSION		0x30
#define APIC_REGISTER_TPR 		0x80
#define APIC_REGISTER_APR 		0x90
#define APIC_REGISTER_PPR 		0xA0
#define APIC_REGISTER_EOI 		0xB0
#define APIC_REGISTER_RRD 		0xC0
#define APIC_REGISTER_LDR 		0xD0
#define APIC_REGISTER_DFR 		0xE0
#define APIC_REGISTER_SIVR 		0xF0
#define APIC_REGISTER_ISR 		0x100
#define APIC_REGISTER_TMR 		0x180
#define APIC_REGISTER_IRR 		0x200
#define APIC_REGISTER_ESR 		0x280
#define APIC_REGISTER_LVT_CMCIR 	0x2F0
#define APIC_REGISTER_ICR 		0x300
#define APIC_REGISTER_ICR_LOW 	0x300
#define APIC_REGISTER_ICR_HIGH 	0x310
#define APIC_REGISTER_LVT_TR 		0x320
#define APIC_REGISTER_LVT_TSR 	0x330
#define APIC_REGISTER_LVT_PMCR 	0x340
#define APIC_REGISTER_LVT_LINT0R 	0x350
#define APIC_REGISTER_LVT_LINT1R 	0x360
#define APIC_REGISTER_LVT_ERROR 	0x370
#define APIC_REGISTER_TIMER_ICR 	0x380
#define APIC_REGISTER_TIMER_CCR 	0x390
#define APIC_REGISTER_TIMER_DCR 	0x3E0

#define IA32_APIC_BASE_EN		11
#define IA32_APIC_BASE_EXD		10
#define IA32_APIC_BASE_BSP		7

/* APIC LVT delivery modes */
#define FIXED					0
#define SMI 					0b01000000000
#define NMI 					0b10000000000
#define INIT 					0b10100000000
#define EXT_INT 				0b11100000000

#define LAPIC_DM_FIXED 			(0b000 << 8)
#define LAPIC_DM_SMI 			(0b010 << 8)
#define LAPIC_DM_NMI 			(0b100 << 8)
#define LAPIC_DM_INIT 			(0b101 << 8)
#define LAPIC_DM_STARTUP 		(0b110 << 8)
#define LAPIC_DM_EXT_INT 		(0b111 << 8)

#define LAPIC_LEVEL_DEASSERT 		0
#define LAPIC_LEVEL_ASSERT 		(1 << 14)

#define LAPIC_TM_EDGE 			0
#define LAPIC_TM_LEVEL 			(1 << 15)

/* APIC Delivery Status (Read-only) */
#define APIC_IDLE				0
#define APIC_PENDING			1

#define TRAMPOLINE_LOW 			0x467
#define TRAMPOLINE_HIGH 			0x469

extern UINT VAPICBase;

typedef U32 APIC_ID;
typedef U8 APIC_VECTOR;

static inline
VOID MapAPIC(){
	VAPICBase = ArchBlock * 4096;
	++ArchBlock;

	EnsureMapping(VAPICBase, DefaultAPICBase, NULL, KF_NOINTR | FLG_NOCACHE, KernelData | PageCacheDisable);
}

#define ReadX2APICRegister ReadMSR
#define WriteX2APICRegister WriteMSR

static inline
UINT RdApicRegister(UINT Reg){
	 return *((UINT volatile*) (VAPICBase + Reg));
}

static inline
VOID WtApicRegister(UINT Reg, UINT Value){
	UINT volatile *VirtualAPICReg = (UINT*) (VAPICBase + Reg); 
 	*VirtualAPICReg = Value;
}

VOID SetupTick();

ULONG TriggerIPI(U32, U32);

VOID DisablePIC(
	VOID
);

VOID MapIDT(
	VOID
);


#endif /* IA32/APIC.h */
