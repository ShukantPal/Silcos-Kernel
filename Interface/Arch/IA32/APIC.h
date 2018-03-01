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

#define IA32_APIC_BASE_EN		11
#define IA32_APIC_BASE_EXD		10
#define IA32_APIC_BASE_BSP		7

#define TRAMPOLINE_LOW	0x467
#define TRAMPOLINE_HIGH	0x469

extern unsigned int VAPICBase;

typedef U32 APIC_ID;
typedef U8 APIC_VECTOR;

static inline void MapAPIC(){
	VAPICBase = ArchBlock * 4096;
	++ArchBlock;

	Pager::map(VAPICBase, DefaultAPICBase,
			KF_NOINTR | FLG_NOCACHE,
			KernelData | PageCacheDisable);
}

decl_c void DisablePIC(void);
decl_c void MapIDT(void);

extern bool x2APICModeEnabled;

/* @class APIC
 *
 * Controls interrupt-handling and management at the processor-level. At
 * hardware-level, it controls the local-apic and is flexible enough to
 * incorporate support for APIC, xAPIC and x2APIC modes.
 *
 * @version 1.0
 * @since Silcos 3.02
 * @author Shukant Pal
 */
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
		Idle = 0,
		SendPending = 1
	};

	enum IntrLevel
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
		NoShorthand 	= 0b00,// destination should be specified
		Self 		= 0b01,// send to the issuing local APIC
		AllIncludingSelf= 0b10,// send to all processors + this one
		AllExcludingSelf= 0b11 // send to all processors except this
	};

	static inline unsigned long id()
	{
		return ((x2APICModeEnabled) ?
				0 : xAPICDriver::read(Id) >> 24);
	}

	static inline unsigned long getBase()
	{
		APICBaseMSR r;
		ReadMSR(IA32_APIC_BASE, &r.msrValue);
		return (r.apicBase);
	}

	static void setupEarlyTimer();
	static void setupScheduleTicks();
	static void triggerIPI(U32 apicId, U8 vect);
	static void wakeupSequence(U32 apicId, U8 page);
private:
	enum xAPIC
	{
		Id		= 0x0020,
		Version		= 0x0030,
		TaskPriority	= 0x0080,
		ArbPriority	= 0x0090,
		ProcPriority	= 0x00A0,
		EOI		= 0x00B0,
		RemoteRead	= 0x00C0,
		LogicalDest	= 0x00D0,
		DestFormat	= 0x00E0,
		SpurIntrVector	= 0x00F0,
		InService	= 0x0100,
		rTriggerMode	= 0x0180,
		IntrRequest	= 0x0200,
		ErrorStatus	= 0x0280,
		CMCI		= 0x02F0,
		ICR_Low		= 0x0300,
		ICR_High	= 0x0310,
		LVT_Timer	= 0x0320,
		LVT_ThermSensor	= 0x0330,
		LVT_PerfMontCntr= 0x0340,
		LVT_LINT0	= 0x0350,
		LVT_LINT1	= 0x0360,
		LVT_Error	= 0x0370,
		InitialCount	= 0x0380,
		CurrentCount	= 0x0390,
		DivideConfig	= 0x03E0
	};

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
			this->delvMode = Fixed;
			this->destMode = Physical;
			this->delvStatus = Idle;
			this->level = Deassert;
			this->trigMode = TriggerMode::Edge;
			this->destShorthand = NoShorthand;
		}

		ICRLow(DeliveryMode delmode, IntrLevel lev, TriggerMode trm)
		{
			value = 0;
			this->vectorNo = 0;
			this->delvMode = delmode;
			this->destMode = Physical;
			this->level = lev;
			this->trigMode = Edge;
			this->destShorthand = NoShorthand;
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
