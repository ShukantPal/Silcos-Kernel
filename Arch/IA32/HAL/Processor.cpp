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

#include <IA32/APBoot.h>
#include <IA32/APIC.h>
#include <ACPI/MADT.h>
#include <Exec/Scheduler.h>
#include <Exec/RoundRobin.h>
#include <Exec/RunqueueBalancer.hpp>
#include <HAL/CPUID.h>
#include <HAL/Processor.h>
#include <HAL/ProcessorTopology.hpp>
#include <IA32/IO.h>
#include <Memory/Pager.h>
#include <Memory/KMemorySpace.h>
#include <Memory/KObjectManager.h>
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

void ConstructProcessor(Processor *proc)
{
	proc->ProcessorStack = (U32) ((ADDRESS) &proc->Hardware.ProcessorStack + PROCESSOR_STACK_SIZE);
	
	KSCHED_ROLLER *rrRoller = &(proc->scheduleClasses[RR_SCHED]);
	rrRoller->ScheduleRunner = &Schedule_RR;
	rrRoller->SaveRunner = &SaveRunner_RR;
	rrRoller->UpdateRunner = &UpdateRunner_RR;
	rrRoller->InsertRunner = &InsertRunner_RR;
	rrRoller->RemoveRunner = &RemoveRunner_RR;
	rrRoller->TransferRunners = NULL;
	proc->RR.Roller = rrRoller;
	
	proc->lschedTable[0] = &proc->rrsched;
	new((void*) proc->lschedTable[0]) Executable::RoundRobin();

	proc->crolStatus.presRoll = proc->lschedTable[0];
}

void AddProcessorInfo(MADTEntryLAPIC *PE)
{
	PROCESSOR *cpu = GetProcessorById(PE->apicID);
	PROCESSOR_INFO *platformInfo = &cpu->Hardware;
	unsigned long apicID = PE->apicID;
	
	if(apicID != BSP_ID)
	{
		EnsureUsability((ADDRESS) cpu, NULL, FLG_NOCACHE, KernelData | PageCacheDisable);
		memset(cpu, 0, sizeof(PROCESSOR));
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
	
	if((CPUIDBuffer[CPUID_EDX] >> 28) & 1)
	{
		DbgLine("HTT Supported");
	}
	
	IA32_APIC_BASE_MSR apicBaseMSR;
	ReadMSR(IA32_APIC_BASE, &apicBaseMSR.msrValue);
	DbgInt(PROCESSOR_ID);
	CPUID(0xB, 2, &CPUIDBuffer[0]);

	Processor *cpu = GetProcessorById(PROCESSOR_ID);
	EnsureUsability((ADDRESS) cpu, NULL, KF_NOINTR | FLG_NOCACHE, KernelData | PageCacheDisable);
	memset(cpu, 0, sizeof(PROCESSOR));
	ConstructProcessor(cpu);
	DisablePIC();
	MapIDT();
	SetupProcessor();

	WtApicRegister(APIC_REGISTER_ESR, 0);

	SetupAPICTimer();

	FlushTLB(0);
	
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

extern "C" void APMain()
{
	PROCESSOR *sceProcessor = GetProcessorById(PROCESSOR_ID);
	DbgLine("ddddddd");
	SetupProcessor();
	Dbg("hellg");
	ProcessorTopology::plug();
	DbgInt((int)sceProcessor->domlink); Dbg(" <=================");
	SetupRunqueue();

	extern void APWaitForPermit();
	APWaitForPermit();
	DbgLine("got the permit");

	FlushTLB(0);
	SetupTick();
	while(TRUE);
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

	if(req)
	{
		switch(req->type)
		{
		case AcceptTasks:
			RunqueueBalancer::Accept *acc = (RunqueueBalancer::Accept*) req;
			tcpu->lschedTable[acc->type]->recieve((KTask*) acc->taskList.lMain, (KTask*) acc->taskList.lMain->last,
								acc->taskList.count);
			break;
		case RenounceTasks:
			DbgLine("EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE");
			RunqueueBalancer::Renounce *ren = (RunqueueBalancer::Renounce*) req;
			ScheduleClass type = ren->taskType;

			unsigned long loadDelta = ren->src.lschedTable[type]->load - ren->dst.lschedTable[type]->load;
			unsigned long deltaFrac = ren->donor.level + 1;

			loadDelta *= deltaFrac - 1;
			loadDelta /= deltaFrac;

			// for cpus in same domain -> transfer 1/2 of load-delta (making same load)
			// for cpus in different domains -> transfer 2/3, 3/4, 4/5, etc.

			RunqueueBalancer::Accept *reply = new(tRunqueueBalancer_Accept)
					RunqueueBalancer::Accept(type, ren->donor, ren->taker);
			ren->src.lschedTable[type]->send(&ren->dst, reply->taskList, loadDelta);
			CPUDriver::writeRequest(*reply, &ren->src);

			kobj_free((kobj*) ren, tRunqueueBalancer_Renounce);

			DbgLine("renounce not imple yet");
			break;
		default:
			return;
		}
	}
}
