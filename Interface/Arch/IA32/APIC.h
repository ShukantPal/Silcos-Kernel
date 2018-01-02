/**
 * File: APIC.h
 *
 * Summary:
 * Embedded xAPIC/x2APIC kernel-driver is given here to be used by other
 * components to control/modify the behavior of the local APIC. It is
 * used mostly by CPU-driver.
 *
 * Functions:
 *
 * Copyright (C) 2017 - Shukant Pal
 */
#ifndef IA32_APIC_H
#define IA32_APIC_H

#include "MSR.h"
#include <Memory/Pager.h>
#include <Memory/KMemorySpace.h>
#include <TYPE.h>

#define DefaultAPICBase 0xFEE00000

/* xAPIC register relative offsets */
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

extern unsigned int VAPICBase;

typedef U32 APIC_ID;
typedef U8 APIC_VECTOR;

static inline void MapAPIC(){
	VAPICBase = ArchBlock * 4096;
	++ArchBlock;

	EnsureMapping(VAPICBase, DefaultAPICBase, NULL, KF_NOINTR | FLG_NOCACHE, KernelData | PageCacheDisable);
}

#define ReadX2APICRegister ReadMSR
#define WriteX2APICRegister WriteMSR

static inline unsigned int RdApicRegister(unsigned int Reg){
	 return *((unsigned int volatile*) (VAPICBase + Reg));
}

static inline void WtApicRegister(unsigned int Reg, unsigned int Value)
{
	unsigned int volatile *VirtualAPICReg = (unsigned int*) (VAPICBase + Reg); 
 	*VirtualAPICReg = Value;
}

void SetupTick();

void TriggerIPI(U32, U32);
void DisablePIC(void);
extern "C" void MapIDT(void);

extern bool x2APICModeEnabled;

class APIC final
{
public:
	enum DeliveryMode
	{
		Fixed = 0b000,
		LowestPriority = 0b001,
		SMI = 0b010,
		Reserved = 0b011,
		NMI = 0b100,
		INIT = 0b101,
		INIT_Deassert = 0b101,
		StartUp = 0b110
	};

	enum DestinationMode
	{
		Physical = 0,
		Logical = 1
	};

	enum DeliveryStatus
	{
		Idle = 0,		// Indicates the local APIC has completed sending previous IPIs
		SendPending = 1		// Indicates the local APIC has some IPIs pending
	};

	enum Level
	{
		Deassert = 0,
		Other = 1
	};

	enum TriggerMode
	{
		Edge = 0,
		Level = 1
	};

	enum DestinationShorthand
	{
		NoShorthand = 0b00,	// The destination should be specified
		Self = 0b01,		// Send the IPI to the issuing local APIC
		AllIncludingSelf = 0b10,// Send the IPI to all processors including the issuing local APIC
		AllExcludingSelf = 0b11	// Send the IPI to all processors in the system except this one
	};

	static inline unsigned long getBase()
	{
		APICBaseMSR r;
		ReadMSR(IA32_APIC_BASE, &r.msrValue);
		return (r.apicBase);
	}

	static void triggerIPI(U32 apicId, U8 vect);
	static void wakeupSequence(U32 apicId, U8 page);
private:
	class xAPICDriver
	{
	public:
		static inline U32 read(unsigned long roff)
		{
			return *(U32 volatile*) (VAPICBase + roff);
		}

		static inline void write(unsigned long roff, U32 val)
		{
			U32 volatile *rptr = (U32 *volatile) (VAPICBase + roff);
			*rptr = val;
		}

		/* not for general use guys */
		static inline void put(unsigned long roff, U32 val)
		{
			U32 volatile *rptr = (U32 *volatile) VAPICBase + roff;
			volatile U32 oval = *rptr | val;
			*rptr = oval;
		}

	private:
		xAPICDriver();
	};

	union ICRLow
	{
		struct
		{
			U32 vectorNo : 8;// vector
			U32 delvMode : 3;// delivery mode
			U32 destMode : 1;// destination mode
			U32 delvStatus : 1;// delivery status
			U32 r0 : 1;// Reserved (bit-13)
			U32 level : 1;
			U32 trigMode : 1;// trigger mode
			U32 r1 : 2;// reserved (bit 16-17)
			U32 destShorthand : 2;
			U32 reserved : 12;
		};
		U32 value;

		ICRLow()
		{
			value = 0;
			this->vectorNo = 0;
			this->delvMode = DeliveryMode::Fixed;
			this->destMode = DestinationMode::Physical;
			this->delvStatus = DeliveryStatus::Idle;
			this->level = Level::Deassert;
			this->trigMode = TriggerMode::Edge;
			this->destShorthand = DestinationShorthand::NoShorthand;
		}

		ICRLow(DeliveryMode delmode, enum APIC::Level lev, TriggerMode trm)
		{
			value = 0;
			this->vectorNo = 0;
			this->delvMode = delmode;
			this->destMode = DestinationMode::Physical;
			this->level = lev;
			this->trigMode = TriggerMode::Edge;
			this->destShorthand = DestinationShorthand::NoShorthand;
		}
	};

	union ICRHigh
	{
		struct
		{
			U32 reserved : 24;
			U32 destField : 8;
		};
		U32 value;

		ICRHigh(U32 dst)
		{
			value = 0;
			this->destField = dst;
		}
	};


	void dd();
	APIC();
};


#endif /* IA32/APIC.h */
