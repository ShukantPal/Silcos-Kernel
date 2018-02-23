///
/// @file Processor.h
/// -------------------------------------------------------------------
/// This program is free software: you can redistribute it and/or modify
/// it under the terms of the GNU General Public License as published by
/// the Free Software Foundation, either version 3 of the License, or
/// (at your option) any later version.
///
/// This program is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
/// GNU General Public License for more details.
///
/// You should have received a copy of the GNU General Public License
/// along with this program.  If not, see <http://www.gnu.org/licenses/>
///
/// Copyright (C) 2017 - Shukant Pal
///

#ifndef X86_PROCESSOR_H
#define X86_PROCESSOR_H

#define NAMESPACE_IA32_GDT
#define NAMESPACE_IA32_IDT
#define NAMESPACE_IA32_TSS

#include "APIC.h"
#include "GDT.h"
#include "IDT.h"
#include "TSS.h"
#include <Executable/IRQHandler.hpp>
#include <HardwareAbstraction/CPUID.h>
#include <Memory/KMemorySpace.h>
#include <Utils/ArrayList.hpp>
#include <Utils/CircularList.h>

//! Alias for the pointer to an HAL::Processor with the given processor Id.
#define GetProcessorById(ID)((HAL::Processor*)(KCPUINFO + 8 * (ID << 12)))

//! Alias for the pointer to the local-irq table for a given CPU
#define GetIRQTableById(id)((HAL::LocalIRQ*)(KCPUINFO + (id << 15) + 24576))

//! Alias for getting the current processor-id
#define PROCESSOR_ID APIC::id()

//! Size of the per-cpu stack for IA32 platforms
#define PROCESSOR_STACK_SIZE 1024

void MxConstructTopology(void *);

namespace HAL
{

///
/// Holds the IRQHandler array for a local interrupt. It is used only
/// for interrupts which have vectors in the range 32 to 192. These can
/// be used by device drivers and are shared among devices unless held
/// exclusively.
///
/// @version 1.0
/// @since Silcos 3.02
/// @author Shukant Pal
///
class LocalIRQ : public Executable::IRQ
{
public:
	static void init(unsigned long tableId);

	void addHandler(IRQHandler *hdlr)
	{
		SpinLock(&lineHdlrs.modl);
		lineHdlrs.add((void*) hdlr);
		SpinUnlock(&lineHdlrs.modl);
	}
private:
	LocalIRQ();
};

///
/// Architectural per-cpu data structure. It holds basic processor information
/// and state data. Many of its members are written during CPUID probing.
///
/// @version 1.2
/// @since Silcos 3.02
/// @author Shukant Pal
///
struct ArchCpu
{
	unsigned int APICID;
	unsigned int ACPIID;
	unsigned int ClusterID;
	unsigned int PackageID;
	unsigned int CoreID;
	unsigned int SMT_ID;
	unsigned int ProcessorStack[256];
	GDTEntry GDT[6] __attribute__((aligned(8)));
	GDTPointer GDTR;
	TSS kTSS;
	IDTEntry IDT[256];//!< obselete @deprecated
	IDTPointer IDTR;//!< obselete @deprecated
	char brandString[64];//!< Brand-string extracted from CPUID
	unsigned long maxBasicLeaf;//!< Maximum CPUID basic-leaf
	unsigned long maxExtLeaf;//!< Maximum CPUID extended-leaf
	unsigned long baseFreq;//!< (display-only) Base-frequency of this cpu
	unsigned long busFreq;//!< (display-only) Bus-frequency for this cpu
	unsigned long nominalFreq;//!< Nominal frequency of the TSC
	unsigned long maxFreq;//!< Maximum frequency for this cpu
	unsigned long tscFreq;//!< Rate at which TSC increments its counter
	unsigned int TopologyIdentifiers[0];

	///
	/// Accumulates the brand string using __cpuid and saves it in ArchCpu
	///
	void detectBrandString()
	{
		__cpuid(CpuId::BrandStringStart, 0, (U32*) brandString);
		__cpuid(CpuId::BrandStringMiddle, 0, (U32*) brandString + 4);
		__cpuid(CpuId::BrandStringEnd, 0, (U32*) brandString + 8);
	}

	///
	/// Detects if the TSC is present in this cpu. If so, it will save the
	/// nominal and TSC frequencies.
	///
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
