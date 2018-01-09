/**
 * File: Processor.c
 *
 * Summary:
 * This file implements processor boot, setup, and management along with proper
 * organization of CPU topology.
 *
 * Function:
 * extern "C" Executable_ProcessorBinding_IPIRequest_Handler - handles incoming
 * 				inter-processor requests on the actionRequest
 * 				list.
 *
 * Copyright (C) 2017 - Shukant Pal
 */
#define NAMESPACEProcessor
#define NS_KFRAMEMANAGER

#include <IA32/APBoot.h>
#include <IA32/APIC.h>
#include <ACPI/MADT.h>
#include <Executable/Scheduler.h>
#include <Executable/RoundRobin.h>
#include <Executable/RunqueueBalancer.hpp>
#include <HAL/CPUID.h>
#include <HAL/Processor.h>
#include <HAL/ProcessorTopology.hpp>
#include <IA32/IO.h>
#include <Memory/Pager.h>
#include <Memory/KMemorySpace.h>
#include <Memory/KObjectManager.h>
#include <Memory/MemoryTransfer.h>
#include <Module/ModuleRecord.h>
#include <Util/CtPrim.h>
#include <Synch/Spinlock.h>
#include <KERNEL.h>

using namespace HAL;
using namespace Executable;

unsigned int BSP_ID;
unsigned int BSP_HID;
bool x2APICModeEnabled;
SPIN_LOCK apPermitLock = 0;
unsigned long APBootSequenceBuffer;
void SetupAPICTimer();
import_asm void TimerWait(U32 t);
void *RunqueueBalancers[3];

extern unsigned long halLoadAddr;
extern unsigned long halLoadPAddr;

using namespace HAL;

PROCESSOR_SETUP_INFO *ConstructTrampoline()
{
	unsigned long apTrampoline = ALLOCATE_TRAMPOLINE();
	void *apBoot = (void *) PADDR_TO_VADDR(apTrampoline);
	memcpy((const void*) &APBoot, apBoot,
			(U32) &APBootSequenceEnd - (U32) &APBoot);
	
	PROCESSOR_SETUP_INFO *setupInfo = (PROCESSOR_SETUP_INFO*) ((U32) apBoot + ((U32)&apSetupInfo - (U32)&APBoot));

	if(setupInfo->BootStatusRegister != AP_STATUS_INIT)
	{
	//	BUG_ADM_CONST_ERR("IA32::BOOT::SMP.apSetupInfo.BootStatusRegister");
		DbgLine("ERR: SMP_BUG; ADM: COMPILE-TIME CONSTANT (PROCES \
			SOR_SETUP_INFO.StatusRegister) MODIFIED");
		while(TRUE);
	}

	GDT_POINTER *bootGDTPointer = &setupInfo->DefaultBootGDTPointer;
	bootGDTPointer->Limit = (sizeof(GDTEntry) * 3) - 1;
	bootGDTPointer->Base = ((U32) &setupInfo->DefaultBootGDT) - KERNEL_OFFSET;
	
	APBootSequenceBuffer = apTrampoline >> 12;
	
	return (PROCESSOR_SETUP_INFO *) (apBoot);
}

void DestructTrampoline(PROCESSOR_SETUP_INFO *trampoline)
{
	// TODO: FREE_TRAMPOLINE()
}

/**
 * Function: ConstructProccessor
 *
 * Summary:
 * Constructor for struct Processor in C-style.
 *
 * Author: Shukant Pal
 */
void ConstructProcessor(Processor *proc)
{
	proc->ProcessorStack = (U32) ((ADDRESS) &proc->Hardware.ProcessorStack + PROCESSOR_STACK_SIZE);
	proc->lschedTable[0] = &proc->rrsched;
	new((void*) proc->lschedTable[0]) Executable::RoundRobin();
	proc->crolStatus.presRoll = proc->lschedTable[0];
}

/**
 * Function: AddProcessorInfo
 *
 * Summary:
 * Adds the processor information to the processor table and does the wakeup
 * sequence on it, assuming it is a application processor.
 *
 * Args:
 * MADTEntryLAPIC *pe - the MADT entry containing information about its local
 *                      APIC
 *
 * Author: Shukant Pal
 */
void AddProcessorInfo(MADTEntryLAPIC *PE)
{
	Processor *cpu = GetProcessorById(PE->apicID);
	PROCESSOR_INFO *platformInfo = &cpu->Hardware;
	unsigned long apicID = PE->apicID;
	
	if(apicID != BSP_ID)
	{
		EnsureUsability((ADDRESS) cpu, NULL, FLG_NOCACHE, KernelData | PageCacheDisable);
		memsetf(cpu, 0, sizeof(Processor));
	}

	platformInfo->APICID = apicID;
	platformInfo->ACPIID = PE->acpiID;

	if(apicID != BSP_ID)
		ConstructProcessor(cpu);

	if(cpu->Hardware.APICID != BSP_ID)
	{
		(void) ConstructTrampoline();
		APIC::wakeupSequence(PE->apicID, (U8) APBootSequenceBuffer);
		WriteCMOSRegister(0xF, 0);
	}
}

const char *nmPROCESSOR_TOPOLOGY = "@SMP::ProcessorTopology";
ObjectInfo *tPROCESSOR_TOPOLOGY;

extern bool oballocNormaleUse;

/**
 * Function: SetupBSP
 *
 * Summary:
 * Performs all pre-SMP functions which are to be done by a single cpu in the
 * system (namely, boot-strap processor).
 *
 * 1. Sets the x2APICModeEnabled trigger
 * 2. Checks whether hyper-threading is available
 * 3. Loads this cpu's APIC id.
 * 4. Constructs this cpu's Processor struct.
 * 5. Disable the programmable interrupt controller, map the IDT, and then load
 *    the GDT, IDT, and TSS for this cpu.
 * 6. Enable the preboot timer.
 * 7. Initalizes the processor-topology subsystem and plug this cpu in it.
 *
 * Author: Shukant Pal
 */
extern "C" void SetupBSP()
{
	BSP_HID = PROCESSOR_ID;
	BSP_ID = PROCESSOR_ID;

	U32 CPUIDBuffer[4];
	CPUIDBuffer[CPUID_ECX] = 1 << 21;
	CPUID(0x1, 0, &CPUIDBuffer[0]);

	if((CPUIDBuffer[CPUID_ECX] >> 21) & 1)
	{
		DbgLine("x2APIC Supported");
		x2APICModeEnabled = TRUE;
	}
	
	IA32_APIC_BASE_MSR apicBaseMSR;
	ReadMSR(IA32_APIC_BASE, &apicBaseMSR.msrValue);
	DbgInt(PROCESSOR_ID);
	CPUID(0xB, 2, &CPUIDBuffer[0]);

	Processor *cpu = GetProcessorById(PROCESSOR_ID);
	EnsureUsability((ADDRESS) cpu, NULL, KF_NOINTR | FLG_NOCACHE, KernelData | PageCacheDisable);
	memset(cpu, 0, sizeof(Processor));
	ConstructProcessor(cpu);
	DisablePIC();
	MapIDT();
	SetupProcessor();
	oballocNormaleUse = true;

	WtApicRegister(APIC_REGISTER_ESR, 0);
	SetupAPICTimer();
	FlushTLB(0);
	
	const ModuleRecord *halRecord = Module::RecordManager::search("kernel.silcos.hal");
	if(!halRecord)
	{
		DbgLine("hal record not found! ");
		while(TRUE){ asm volatile("hlt"); }
	}
	else
	{
		halLoadAddr = halRecord->BaseAddr;
		MMFRAME* halframe = GetFrames(halLoadAddr, 1, null);
		halLoadPAddr = FRADDRESS(halframe);
	}

	WriteCMOSRegister(0xF, 0xA);
	unsigned long startEIP = APBootSequenceBuffer * KB(4);
	*((volatile unsigned short *) PADDR_TO_VADDR(TRAMPOLINE_HIGH)) = (startEIP >> 4);
	*((volatile unsigned short *) PADDR_TO_VADDR(TRAMPOLINE_LOW)) = (startEIP & 0xF);

	ProcessorTopology::init();
	ProcessorTopology::plug();
}

extern "C" void SetupAPs()
{
	EnumerateMADT(&AddProcessorInfo, NULL, NULL);
}

extern "C" ProcessorInfo *SetupProcessor()
{
	Processor *pCPU = GetProcessorById(PROCESSOR_ID);
	ProcessorInfo *pInfo = &(pCPU->Hardware);

	SetupGDT(pInfo);
	SetupIDT();
	SetupTSS(pInfo);
	
	return (pInfo);
}

/**
 * Function: APMain
 *
 * Summary:
 * Sort of initializer for application processors & their runqueues.
 *
 * Author: Shukant Pal
 */
extern "C" void APMain()
{
	SetupProcessor();
	ProcessorTopology::plug();
	SetupRunqueue();

	extern void APWaitForPermit();
	APWaitForPermit();

	FlushTLB(0);
	SetupTick();

	while(TRUE)
	{
		asm volatile("hlt;");
	}
}

/*
 * Function: CPURequestHandler
 *
 * Summary:
 * Handles a inter-processor request sent by another CPU to this CPU. It
 * has separate handlers for each request-type.
 *
 * Author: Shukant Pal
 */
extern "C" void Executable_ProcessorBinding_IPIRequest_Handler()
{
	Processor *tcpu = GetProcessorById(PROCESSOR_ID);
	IPIRequest *req = CPUDriver::readRequest(tcpu);

	if(req != null)
	{
		switch(req->type)
		{
		case AcceptTasks:
		{
			RunqueueBalancer::Accept *acc = (RunqueueBalancer::Accept*) req;
			tcpu->lschedTable[acc->type]->recieve((KTask*) acc->taskList.lMain, (KTask*) acc->taskList.lMain->last,
								acc->taskList.count, acc->load);
			break;
		}
		case RenounceTasks:
		{
			RunqueueBalancer::Renounce *ren = (RunqueueBalancer::Renounce*) req;
			ScheduleClass type = ren->taskType;

			unsigned long loadDelta = ren->src.lschedTable[type]->load - ren->dst.lschedTable[type]->load;
			unsigned long deltaFrac = ren->donor.level + 1;

			loadDelta *= deltaFrac;
			loadDelta /= deltaFrac + 1;

			// for cpus in same domain -> transfer 1/2 of load-delta (making same load)
			// for cpus in different domains -> transfer 2/3, 3/4, 4/5, etc.

			RunqueueBalancer::Accept *reply = new(tRunqueueBalancer_Accept)
					RunqueueBalancer::Accept(type, ren->donor, ren->taker);

			ren->src.lschedTable[type]->send(&ren->dst, reply->taskList, loadDelta);
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
}
