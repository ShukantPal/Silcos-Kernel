/**
 * File: Processor.c
 *
 * Summary:
 * This file implements processor boot, setup, and management along with proper
 * organization of CPU topology.
 *
 * Copyright (C) 2017 - Shukant Pal
 */
#define NAMESPACEProcessor

#include <IA32/APBoot.h>
#include <IA32/APIC.h>
#include <ACPI/MADT.h>
#include <Exec/Scheduler.h>
#include <Exec/RR.h>
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
	
	KSCHED_ROLLER *rrRoller = &(proc->ScheduleClasses[RR_SCHED]);
	rrRoller->ScheduleRunner = &Schedule_RR;
	rrRoller->SaveRunner = &SaveRunner_RR;
	rrRoller->UpdateRunner = &UpdateRunner_RR;
	rrRoller->InsertRunner = &InsertRunner_RR;
	rrRoller->RemoveRunner = &RemoveRunner_RR;
	rrRoller->TransferRunners = NULL;
	proc->RR.Roller = rrRoller;
	
	KSCHEDINFO *cpuScheduler = &(proc->SchedulerInfo);
	cpuScheduler->CurrentRoller = rrRoller;
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
		TriggerIPI(PE->apicID, LAPIC_DM_INIT | LAPIC_LEVEL_ASSERT);
		TimerWait(1);
		TriggerIPI(PE->apicID, APBootSequenceBuffer | LAPIC_DM_STARTUP);
		TimerWait(1);
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
	ReadMSR(IA32_APIC_BASE, &apicBaseMSR.MSRValue);
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
	SetupProcessor();
	ProcessorTopology::plug();
	SetupRunqueue();

	extern void APWaitForPermit();
	APWaitForPermit();

	FlushTLB(0);
	SetupTick();
	while(TRUE);
}
