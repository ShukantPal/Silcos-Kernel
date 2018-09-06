///
/// @file Processor.cpp
///
/// Implements the management of processor-related hardware and data. These
/// are broadly categorized as - local-IRQs, per-CPU structs, ArchCpu handling,
/// and inter-processor request handling.
///
/// This file is huge and wants to become larger, pack more features here.
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

#define NAMESPACEProcessor
#define NS_KFRAMEMANAGER

#include <Character.hpp>
#include <Object.hpp>
#include <IA32/APBoot.h>
#include <IA32/APIC.h>
#include <ACPI/MADT.h>
#include <ACPI/HPET.h>
#include <Executable/Scheduler.h>
#include <Executable/RoundRobin.h>
#include <Executable/RunqueueBalancer.hpp>
#include <HardwareAbstraction/CPUID.h>
#include <HardwareAbstraction/IOAPIC.hpp>
#include <HardwareAbstraction/Processor.h>
#include <HardwareAbstraction/ProcessorTopology.hpp>
#include <IA32/IO.h>
#include <Memory/Pager.h>
#include <Memory/KMemorySpace.h>
#include <Memory/KObjectManager.h>
#include <Memory/MemoryTransfer.h>
#include <Module/ModuleRecord.h>
#include <Module/ModuleContainer.hpp>
#include <Utils/Memory.h>
#include <Synch/Spinlock.h>
#include <KERNEL.h>

#include <Environment.h>

using namespace HAL;
using namespace HAL::CpuId;
using namespace Executable;
using namespace Module;
using namespace Module::Elf;

unsigned int BSP_ID;
unsigned int BSP_HID;
bool x2APICModeEnabled;
Spinlock apPermitLock = 0;
unsigned long APBootSequenceBuffer;
import_asm void TimerWait(U32 t);
void *RunqueueBalancers[3];

extern unsigned long halLoadAddr;
extern unsigned long halLoadPAddr;

using namespace HAL;

///
/// Initializes the local-IRQ block for the given table-id. This id is same as
/// the processor's id for which the IRQ's exist.
///
/// @param id - processor id for the irq's
/// @version 1.0
/// @since Silcos 3.02
/// @author Shukant Pal
///
void LocalIRQ::init(unsigned long id)
{
	LocalIRQ *table = GetIRQTableById(id);

	for(unsigned long i = 0; i < 160; i++)
		new(table + i) LocalIRQ();
}

LocalIRQ::LocalIRQ() : IRQ()
{

}

///
/// Extracts the processor's base frequency from its brand-string. It should be
/// used if the frequency-info leaf is not supported. The base-frequency given
/// should not be used for purposes other than displaying information on the
/// console, as it is neither accurate nor relevant for software-usage.
///
/// The brand-string is scanned in reverse order to match the substring "MHz",
/// "GHz" or "THz" to get the frequency-unit. On matching "unit" substrings, the
/// multipler for that unit is calculated and then the decimal value given in
/// the string is multiplied with the unit/multiplier. This occurs without using
/// floating-point decimal values, by dividing the multiplier by 10 until a
/// decimal point is found.
///
/// @arg brandString - the brand string detected using __cpuid
/// @return base-frequency of the cpu, if detected successfully; if not, 0;
/// @version 1.0
/// @since Silcos 3.02
/// @author Shukant Pal
///
unsigned long ArchCpu::extractBaseFrequency()
{
	char *tc = brandString + 62;
	char cmul;
	int ctr = 0;
	while(ctr < 60)
	{
		if(*tc == 'z' && *(tc - 1) == 'H')
		{
			cmul = *(tc - 2);
			if(cmul == 'M' || cmul == 'G' || cmul == 'T')
				break;
		}

		++(ctr);
		--(tc);
	}

	tc -= 3;
	if(ctr == 60)
		return (0);

	unsigned long mult;
	switch(cmul)
	{
	case 'M':
		mult = 1000000;
		break;
	case 'G':
		mult = 1000000000;
		break;
	default:
		DbgLine("ERROR: THz not supported");
		while(TRUE) asm volatile("hlt;");
		break;
	}

	unsigned long freq = 0, digits = 0, pow10 = 1, pointOffset = 64;
	while(*tc != ' ')
	{
		if(*tc == '.')
		{
			pointOffset = digits;
		}
		else if(Character::isDigit(*tc))
		{
			freq += pow10 * (*tc - 48);

			if(pointOffset == 64)
				mult /= 10;

			pow10 *= 10;
			++(digits);
		}
		else
			return (0);

		--(tc);
	}

	if(pointOffset == 64)
	{
		while(digits > 0)
		{
			--(digits);
			mult *= 10;
		}
	}

	freq *= mult;
	return (freq);
}

///
/// Constructs the trampoline for booting application processors. It copies the
/// code & data present b/w APBootSequenceStart & APBootSequenceEnd to a
/// hardwired physical-address.
///
/// @version 1.2
/// @since Circuit 2.03
/// @author Shukant Pal
/// @see APBoot.asm
///
decl_c PROCESSOR_SETUP_INFO *ConstructTrampoline()
{
	unsigned long apTrampoline = ALLOCATE_TRAMPOLINE();
	void *apBoot = (void *) PADDR_TO_VADDR(apTrampoline);
	memcpy((const void*) &APBoot, apBoot,
			(U32) &APBootSequenceEnd - (U32) &APBoot);
	
	PROCESSOR_SETUP_INFO *setupInfo = (PROCESSOR_SETUP_INFO*)
			((U32) apBoot + ((U32)&apSetupInfo - (U32)&APBoot));

	if(setupInfo->BootStatusRegister != AP_STATUS_INIT)
	{
		DbgLine("ERR: SMP_BUG; ADM: COMPILE-TIME CONSTANT (PROCES \
			SOR_SETUP_INFO.StatusRegister) MODIFIED");
		while(TRUE);
	}

	GDT_POINTER *bootGDTPointer = &setupInfo->DefaultBootGDTPointer;
	bootGDTPointer->Limit = (sizeof(GDTEntry) * 3) - 1;
	bootGDTPointer->Base = ((U32) &setupInfo->DefaultBootGDT) - KERNEL_OFFSET;
	
	APBootSequenceBuffer = apTrampoline >> 12;
	
	return ((PROCESSOR_SETUP_INFO *)(apBoot));
}

extern "C" void DestructTrampoline(PROCESSOR_SETUP_INFO *trampoline)
{
}

decl_c void ConstructProcessor(Processor *proc)
{
	proc->ProcessorStack = (U32) ((ADDRESS) &proc->hw.ProcessorStack + PROCESSOR_STACK_SIZE);
	proc->lschedTable[0] = &proc->rrsched;
	new((void*) proc->lschedTable[0]) Executable::RoundRobin();
	proc->crolStatus.presRoll = proc->lschedTable[0];
}

///
/// Maps the processor's per-CPU struct into its index in the CPU-table. It also
/// writes basic-information into it using the ACPI 2.0 MADT entry for its local
/// ACPI and using __cpuid functions. After initializing the per-CPU struct, it
/// invokes the APIC wakeup-sequence through which the processor gets-up and
/// starts running.
///
/// @param pe - the ACPI 2.0 MADT entry for the LAPIC of the given cpu
/// @version 1.0
/// @since Silcos 3.02
/// @author Shukant Pal
///
decl_c void AddProcessorInfo(MADTEntryLAPIC *PE)
{
	Processor *cpu = GetProcessorById(PE->apicID);
	HAL::ArchCpu *platformInfo = &cpu->hw;
	unsigned long apicID = PE->apicID;
	
	if(apicID != BSP_ID)
	{
		Pager::use((ADDRESS) cpu, FLG_NOCACHE, KernelData);
		Pager::use((ADDRESS) cpu + 4096, FLG_NOCACHE, KernelData);

		unsigned long irt = (unsigned long) GetIRQTableById(PE->apicID);
		Pager::use((ADDRESS) irt, FLG_NOCACHE, KernelData);
		Pager::use((ADDRESS) irt + 4096, FLG_NOCACHE, KernelData);

		LocalIRQ::init(PE->apicID);

		memsetf(cpu, 0, sizeof(Processor));
	}

	platformInfo->APICID = apicID;
	platformInfo->ACPIID = PE->acpiID;

	if(apicID != BSP_ID)
		ConstructProcessor(cpu);

	if(cpu->hw.APICID != BSP_ID)
	{
		(void) ConstructTrampoline();
		APIC::wakeupSequence(PE->apicID, (U8) APBootSequenceBuffer);
		WriteCMOSRegister(0xF, 0);
	}
}

const char *nmPROCESSOR_TOPOLOGY = "@SMP::ProcessorTopology";
ObjectInfo *tPROCESSOR_TOPOLOGY;

extern bool oballocNormaleUse;
extern "C" void Spurious();

///
/// Executes the roles of the boot-strap processor relevant to the system
/// startup. It checks if all features are available and enables interrupts,
/// memory-segmentation, and the task-state segment. It enables the pre-boot
/// kernel timer and initializes cpu-topology allowing other processors to
/// wakeup later.
///
/// @version 1.0
/// @since Silcos 3.02
/// @author Shukant Pal
///
decl_c void SetupBSP()
{
	BSP_HID = PROCESSOR_ID;
	BSP_ID = PROCESSOR_ID;

	cRegs ot;
	__cpuid(0x1, 0, &ot.output[0]);

	if((ot.ecx >> 21) & 1)
	{
		DbgLine("x2APIC Supported");
		x2APICModeEnabled = TRUE;
	}
	
	IA32_APIC_BASE_MSR apicBaseMSR;
	ReadMSR(IA32_APIC_BASE, &apicBaseMSR.msrValue);
	::__cpuid(0xB, 2, &ot.output[0]);

	Processor *cpu = GetProcessorById(PROCESSOR_ID);
	Pager::use((ADDRESS) cpu, KF_NOINTR | FLG_NOCACHE,
			KernelData | PageCacheDisable);

	unsigned long irt = (unsigned long) GetIRQTableById(PROCESSOR_ID);
	Pager::use((ADDRESS) irt, FLG_NOCACHE, KernelData);
	Pager::use((ADDRESS) irt + 4096, FLG_NOCACHE, KernelData);
	LocalIRQ::init(PROCESSOR_ID);

	memsetf(cpu, 0, sizeof(Processor));

	ConstructProcessor(cpu);
	DisablePIC();
	MapIDT();
	SetupProcessor();
	oballocNormaleUse = true;

	APIC::setupEarlyTimer();

	ModuleContainer *halCtr = reinterpret_cast<ModuleContainer*>(
			Namespace::search("::kernel.silcos.hal", false));

	if(!halCtr)
	{
		DbgLine("hal record not found! ");
		while(TRUE){ asm volatile("hlt"); }
	}
	else
	{
		halLoadAddr = halCtr->getBase();
		MMFRAME* halframe = GetFrames(halLoadAddr, 1, KERNEL_CONTEXT);
		halLoadPAddr = FRADDRESS(halframe);
	}

	WriteCMOSRegister(0xF, 0xA);

	cpu->hw.init();

	ProcessorTopology::init();
	ProcessorTopology::plug();

}

static IOAPIC::RedirectionEntry __init_input_for_timer(
		IOAPIC::RedirectionEntry re)
{
	re.delMod = 0;
	re.destMod = 0;
	re.intMask = 0;
	re.destination = 0;
	re.triggerMode = 1;
	re.pinPolarity = 1;

	re.localVector = 36;
	re.destination = 0;

	return (re);
}

///
/// Setups the application-processors and registers the IOAPICs.
///
decl_c void SetupAPs()
{
	EnumerateMADT(&AddProcessorInfo, &IOAPIC::registerIOAPIC, null);

	IOAPIC *pHub = IOAPIC::getIterable();

	if(pHub == null) {
		DbgLine("Warning: No IOAPIC present, XT-PIC fallback not impl");
	} else {
		pHub->writeRedirection(2, &__init_input_for_timer);

		InitKernelHPET();
	}

	// testing - phase
//	testhpet();
}

///
/// Initializes the platform-dependent structures for this CPU. These are
/// namely - GDT, IDT, and TSS.
///
/// @version 1.0
/// @since Circuit 2.03
///
decl_c ArchCpu *SetupProcessor()
{
	Processor *pCPU = GetProcessorById(PROCESSOR_ID);
	ArchCpu *pInfo = &(pCPU->hw);

	SetupGDT(pInfo);
	SetupTSS(pInfo);
	SetupIDT();

	return (pInfo);
}

///
/// Initialization routine for application processors; Once the higher-half
/// kernel is enabled and the C++ runtime is set this function is called
/// which performs topology-registering and then starts the init-thread for
/// this processor.
///
/// @version 1.0
/// @since Circuit 2.03
/// @author Shukant Pal
///
decl_c void APMain()
{
	SetupProcessor();
	ProcessorTopology::plug();
	SetupRunqueue();

	unsigned long b[2];
	extern void APWaitForPermit(unsigned long*);
	APWaitForPermit(b);

	FlushTLB(0);
	APIC::setupScheduleTicks();

	while(TRUE)
	{
		asm volatile("hlt;");
	}
}

///
/// Handles inter-processor requests pending for the current CPU. It popps
/// out one request entry from the circular-list and then follows the
/// request accordingly. This is called when the requesting CPU hooks its
/// request on this CPU's actionRequest buffer and then fires an IPI to this
/// processor.
///
/// It is mainly used for runqueue-balancing and accessing data structures
/// which are exclusive for this CPU.
///
/// @version 1.0
/// @since Circuit 2.03
/// @author Shukant Pal
///
decl_c void Executable_ProcessorBinding_IPIRequest_Handler()
{
	Processor *tcpu = GetProcessorById(PROCESSOR_ID);
	IPIRequest *req = CPUDriver::readRequest(tcpu);

	if(req == null)
		return;

	switch(req->type)
	{
	case AcceptTasks:
	{
		RunqueueBalancer::Accept *acc =
				(RunqueueBalancer::Accept*) req;
		tcpu->lschedTable[acc->type]->recieve(
				(Executable::Task*) acc->taskList.lMain,
				(Executable::Task*) acc->taskList.lMain->last,
				acc->taskList.count, acc->load);
		break;
	}
	case RenounceTasks:
	{
		RunqueueBalancer::Renounce *ren =
				(RunqueueBalancer::Renounce*) req;
		ScheduleClass type = ren->taskType;
		unsigned long loadDelta = ren->src.lschedTable[type]->load
				- ren->dst.lschedTable[type]->load;
		unsigned long deltaFrac = ren->donor.level + 1;

		loadDelta *= deltaFrac;
		loadDelta /= deltaFrac + 1;

		// for cpus in same domain -> transfer 1/2 of load-delta (making same load)
		// for cpus in different domains -> transfer 2/3, 3/4, 4/5, etc.

		RunqueueBalancer::Accept *reply = new(tRunqueueBalancer_Accept)
				RunqueueBalancer::Accept(type, ren->donor, ren->taker);

		ren->src.lschedTable[type]->send(&ren->dst,
				reply->taskList, loadDelta);
		reply->load = loadDelta;
		CPUDriver::writeRequest(*reply, &ren->dst);

		kobj_free((kobj*) ren, tRunqueueBalancer_Renounce);
		break;
	}
	default:
		Dbg("NODF");
		break;
	}
}
