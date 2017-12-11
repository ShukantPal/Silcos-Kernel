/**
 * File: Processor.c
 *
 * Summary:
 * This file implements processor boot, setup, and management along with proper
 * organization of CPU topology.
 *
 * Copyright (C) 2017 - Shukant Pal
 */

#define NAMESPACE_PROCESSOR

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

UINT BSP_ID;
UINT BSP_HID;

BOOL x2APICModeEnabled;

SPIN_LOCK apPermitLock = 0;

ULONG APBootSequenceBuffer;

VOID SetupAPICTimer();
import_asm void TimerWait(U32 t);

VOID *RunqueueBalancers[3];

using namespace HAL;

PROCESSOR_SETUP_INFO *ConstructTrampoline(){
	ULONG apTrampoline = ALLOCATE_TRAMPOLINE();
	VOID *apBoot = (VOID *) PADDR_TO_VADDR(apTrampoline);
	memcpy((const void*) &APBoot, apBoot,
			(U32) &APBootSequenceEnd - (U32) &APBoot);
	
	PROCESSOR_SETUP_INFO *setupInfo
			= (VOID*) ((U32) apBoot + ((U32)&apSetupInfo - (U32)&APBoot));
	if(setupInfo->BootStatusRegister != AP_STATUS_INIT){
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

VOID DestructTrampoline(PROCESSOR_SETUP_INFO *trampoline){
	// TODO: FREE_TRAMPOLINE()
}

VOID ConstructProcessor(PROCESSOR *processorInfo){
	processorInfo->ProcessorStack =
		(U32) ((VIRTUAL_T) &processorInfo->Hardware.ProcessorStack + PROCESSOR_STACK_SIZE);
	
	KSCHED_ROLLER *rrRoller = &(processorInfo->ScheduleClasses[RR_SCHED]);
	rrRoller->ScheduleRunner = &Schedule_RR;
	rrRoller->SaveRunner = &SaveRunner_RR;
	rrRoller->UpdateRunner = &UpdateRunner_RR;
	rrRoller->InsertRunner = &InsertRunner_RR;
	rrRoller->RemoveRunner = &RemoveRunner_RR;
	rrRoller->TransferRunners = NULL;
	processorInfo->RR.Roller = rrRoller;
	
	KSCHEDINFO *cpuScheduler = &(processorInfo->SchedulerInfo);
	cpuScheduler->CurrentRoller = rrRoller;
}

VOID AddProcessorInfo(MADTEntryLAPIC *PE){
	PROCESSOR *cpu = GetProcessorById(PE->apicID);
	PROCESSOR_INFO *platformInfo = &cpu->Hardware;
	ULONG apicID = PE->apicID;
	
	if(apicID != BSP_ID){
		EnsureUsability((ADDRESS) cpu, NULL, FLG_NOCACHE, KernelData | PageCacheDisable);
		memset(cpu, 0, sizeof(PROCESSOR));
	}

	platformInfo->APICID = apicID;
	platformInfo->ACPIID = PE->acpiID;
	if(apicID != BSP_ID)
		ConstructProcessor(cpu);

	if(cpu->Hardware.APICID != BSP_ID){
		(VOID) ConstructTrampoline();
		TriggerIPI(PE->apicID, LAPIC_DM_INIT | LAPIC_LEVEL_ASSERT);
		TimerWait(1);
		TriggerIPI(PE->apicID, APBootSequenceBuffer | LAPIC_DM_STARTUP);
		TimerWait(1);
		WriteCMOSRegister(0xF, 0);
	}
}

const char *nmPROCESSOR_TOPOLOGY = "@SMP::ProcessorTopology";
ObjectInfo *tPROCESSOR_TOPOLOGY;

decl_c void SetupBSP(){
	BSP_HID = PROCESSOR_ID;
	BSP_ID = PROCESSOR_ID;

	U32 CPUIDBuffer[4];
	CPUIDBuffer[CPUID_ECX] = 1 << 21;
	CPUID(0x1, 0, &CPUIDBuffer[0]);

	if((CPUIDBuffer[CPUID_ECX] >> 21) & 1){
		DbgLine("x2APIC Supported");
		x2APICModeEnabled = TRUE;
	}
	
	if((CPUIDBuffer[CPUID_EDX] >> 28) & 1){
		DbgLine("HTT Supported");
	}
	
	IA32_APIC_BASE_MSR apicBaseMSR;
	ReadMSR(IA32_APIC_BASE, &apicBaseMSR.MSRValue);
	DbgInt(PROCESSOR_ID);
	CPUID(0xB, 2, &CPUIDBuffer[0]);

	PROCESSOR *cpu = GetProcessorById(PROCESSOR_ID);
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
	ULONG startEIP = APBootSequenceBuffer * KB(4);
	*((volatile USHORT *) PADDR_TO_VADDR(TRAMPOLINE_HIGH)) = (startEIP >> 4);
	*((volatile USHORT *) PADDR_TO_VADDR(TRAMPOLINE_LOW)) = (startEIP & 0xF);

	ProcessorTopology::init();
	ProcessorTopology::plug();
}

decl_c void SetupAPs(){
	EnumerateMADT(&AddProcessorInfo, NULL, NULL);
}

decl_c struct ProcessorInfo *SetupProcessor(){
	PROCESSOR *pCPU = GetProcessorById(PROCESSOR_ID);
	PROCESSOR_INFO *pInfo = &(pCPU->Hardware);

	SetupGDT(pInfo);
	SetupIDT();
	SetupTSS(pInfo);
	
	return (pInfo);
}

export_asm void APMain(){
	PROCESSOR *sceProcessor = GetProcessorById(PROCESSOR_ID);
	SetupProcessor();
	ProcessorTopology::plug();
	SetupRunqueue();

	extern VOID APWaitForPermit();
	APWaitForPermit();

	FlushTLB(0);
	SetupTick();
	while(TRUE);
}
