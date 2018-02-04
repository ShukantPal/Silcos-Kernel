/**
 * File: Processor.h
 *
 * Summary:
 * Declares how the IA32-processor-specific data is maintained in each per-CPU
 * struct. Also defines some initialization specific code used during SMP-wake
 * to initialize application-processors.
 *
 * Copyright (C) 2017 - Shukant Pal
 */
#ifndef X86_PROCESSOR_H
#define X86_PROCESSOR_H

#define NAMESPACE_IA32_GDT
#define NAMESPACE_IA32_IDT
#define NAMESPACE_IA32_TSS

#include "APIC.h"
#include "GDT.h"
#include "IDT.h"
#include "TSS.h"
#include <HAL/CPUID.h>
#include <Memory/KMemorySpace.h>
#include <Util/CircularList.h>

#define GetProcessorById(ID) ((HAL::Processor*) (KCPUINFO + (ID << 12)))
#define PROCESSOR_ID APIC::id()
#define PROCESSOR_STACK_SIZE 1024

void MxConstructTopology(void *);

namespace HAL
{

typedef
struct ArchCpu
{
	unsigned int APICID;
	unsigned int ACPIID;
	unsigned int ClusterID;
	unsigned int PackageID;
	unsigned int CoreID;
	unsigned int SMT_ID;
	unsigned int ProcessorStack[256];
	GDTEntry GDT[6];
	GDTPointer GDTR;
	TSS kTSS;
	IDTEntry IDT[256];
	IDTPointer IDTR;
	char brandString[64];
	unsigned long maxBasicLeaf;
	unsigned long maxExtLeaf;
	unsigned long baseFreq;
	unsigned long busFreq;
	unsigned long nominalFreq;
	unsigned long maxFreq;
	unsigned long tscFreq;
	unsigned int TopologyIdentifiers[0];

	void detectBrandString()
	{
		__cpuid(CpuId::BrandStringStart, 0, (U32*) brandString);
		__cpuid(CpuId::BrandStringMiddle, 0, (U32*) brandString + 4);
		__cpuid(CpuId::BrandStringEnd, 0, (U32*) brandString + 8);
	}

	void detectTSC()
	{
		CpuId::cRegs ot;
		__cpuid(CpuId::TimeStampCounter, 0, &ot.output[0]);

		if(ot.ebx != 0 && ot.ecx != 0)
		{
			nominalFreq = ot.ecx;
			tscFreq = ot.ecx * (ot.ebx/ot.eax);
		}
		else
		{
			nominalFreq = ot.ecx;
			tscFreq = 0;
		}
	}

	bool enumFreqInfo()
	{
		CpuId::cRegs ot;
		__cpuid(CpuId::FrequencyInfo, 0, &ot.output[0]);

		baseFreq = ot.eax & 0xFFFF0000;
		busFreq = ot.ecx & 0xFFFF0000;
		maxFreq = ot.ebx & 0xFFFF0000;

		return (baseFreq);
	}

	unsigned long extractBaseFrequency();

	void init()
	{
		detectBrandString();
		detectTSC();
		if(!enumFreqInfo())
			extractBaseFrequency();
	}
};

typedef
struct {
	GDTEntry DefaultBootGDT[3];
	unsigned short BootStatusRegister;
	GDTPointer DefaultBootGDTPointer;
} PROCESSOR_SETUP_INFO;

#define __INTR_ON asm("sti");
#define __INTR_OFF asm("cli");

}

extern "C"
{
void SetupDefaultBootGDT(void);
void SetupGDT(HAL::ArchCpu *);
void SetupTSS(HAL::ArchCpu *);
void SetupIDT(void);
void SetupBSP(void);
void SetupAPs(void);
HAL::ArchCpu *SetupProcessor(void);
}

#endif/* IA32/Processor.h */
