/*=++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * File: Processor.c
 *
 * Summary:
 * This file implements processor boot, setup, and management along with proper
 * organization of CPU topology.
 *
 * Copyright (C) 2017 - Shukant Pal
 *+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=*/

#define NAMESPACE_PROCESSOR

#include <IA32/APBoot.h>
#include <IA32/APIC.h>
#include <ACPI/MADT.h>
#include <Exec/Scheduler.h>
#include <Exec/RR.h>
#include <HAL/CPUID.h>
#include <HAL/Processor.h>
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
VOID TimerWait(U32 t);

VOID *RunqueueBalancers[3];

PROCESSOR_SETUP_INFO *ConstructTrampoline(){
	ULONG apTrampoline = ALLOCATE_TRAMPOLINE();
	VOID *apBoot = (VOID *) PADDR_TO_VADDR(apTrampoline);
	memcpy(&APBoot, apBoot, (U32) &APBootSequenceEnd - (U32) &APBoot);
	
	PROCESSOR_SETUP_INFO *setupInfo
			= (VOID*) ((U32) apBoot + ((U32)&apSetupInfo - (U32)&APBoot));
	if(setupInfo->BootStatusRegister != AP_STATUS_INIT){
	//	BUG_ADM_CONST_ERR("IA32::BOOT::SMP.apSetupInfo.BootStatusRegister");
		DbgLine("ERR: SMP_BUG; ADM: COMPILE-TIME CONSTANT (PROCES \
			SOR_SETUP_INFO.StatusRegister) MODIFIED");
		while(TRUE);
	}

	GDT_POINTER *bootGDTPointer = &setupInfo->DefaultBootGDTPointer;
	bootGDTPointer->Limit = (sizeof(GDT) * 3) - 1;
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

VOID AddProcessorInfo(MADT_ENTRY_LAPIC *PE){
	PROCESSOR *cpu = GetProcessorById(PE->APICID);
	PROCESSOR_INFO *platformInfo = &cpu->Hardware;
	ULONG apicID = PE->APICID;
	
	if(apicID != BSP_ID){
		EnsureUsability((ADDRESS) cpu, NULL, FLG_NOCACHE, KernelData | PageCacheDisable);
		memset(cpu, 0, sizeof(PROCESSOR));
	}

	platformInfo->APICID = apicID;
	platformInfo->ACPIID = PE->ACPIID;
	if(apicID != BSP_ID)
		ConstructProcessor(cpu);

	if(cpu->Hardware.APICID != BSP_ID){
		(VOID) ConstructTrampoline();
		TriggerIPI(PE->APICID, LAPIC_DM_INIT | LAPIC_LEVEL_ASSERT);
		TimerWait(1);
		TriggerIPI(PE->APICID, APBootSequenceBuffer | LAPIC_DM_STARTUP);
		TimerWait(1);
		WriteCMOSRegister(0xF, 0);
	}
}

CHAR *nmPROCESSOR_TOPOLOGY = "SMP::PROCESSOR_TOPOLOGY";
OBINFO *tPROCESSOR_TOPOLOGY;

VOID SetupBSP(){
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
	
	tPROCESSOR_TOPOLOGY = KiCreateType(nmPROCESSOR_TOPOLOGY, sizeof(PROCESSOR_TOPOLOGY), 16, NULL, NULL);
	MxRegisterProcessor();
}

VOID SetupAPs(){
	EnumerateMADT(&AddProcessorInfo, NULL, NULL);
}

PROCESSOR_INFO *SetupProcessor(){
	PROCESSOR *pCPU = GetProcessorById(PROCESSOR_ID);
	PROCESSOR_INFO *pInfo = &(pCPU->Hardware);

	SetupGDT(pInfo);
	SetupIDT();
	SetupTSS(pInfo);
	
	return (pInfo);
}

PROCESSOR_TOPOLOGY systemTopology;
SPIN_LOCK TopologyLock;

VOID MxConstructTopology(VOID *ptr){
	PROCESSOR_TOPOLOGY *pTopology = ptr;
	pTopology->WriteLock = 0;
	pTopology->DomainList.ClnMain = NULL;
	pTopology->DomainList.ClnCount = 0;
}

VOID UpdateProcessorCount(SCHED_DOMAIN *domain){ ++domain->ProcessorCount; }

MX_REGISTER_STATUS MxRegisterProcessor(){
	PROCESSOR *pCPU = GetProcessorById(PROCESSOR_ID);
	PROCESSOR_INFO *pInfo = &pCPU->Hardware;

	UINT *topologyIdentifiers = &(pInfo->TopologyIdentifiers[0]);

	if(x2APICModeEnabled){pInfo->SMT_ID = 'N';}
	else {
		APIC_ID apicId = pInfo->APICID;
		extern U32 FindMaskWidth(U32);

		U32 CPUIDBuffer[4];
		CPUID(4, 0, CPUIDBuffer);
		U32 coreIdSubfieldSize = FindMaskWidth((CPUIDBuffer[0] >> 26) + 1);
		CPUID(1, 0, CPUIDBuffer);
		U32 smtIdSubfieldSize = FindMaskWidth((CPUIDBuffer[1] >> 16) & 0xFF) - coreIdSubfieldSize;
		pInfo->SMT_ID = apicId & ((1 << smtIdSubfieldSize) - 1);
		pInfo->CoreID = (apicId >> smtIdSubfieldSize) & ((1 << coreIdSubfieldSize) - 1);
		pInfo->PackageID = (apicId >> (coreIdSubfieldSize + smtIdSubfieldSize));
		pInfo->ClusterID = 0;

		topologyIdentifiers[0] = pInfo->SMT_ID;
		topologyIdentifiers[1] = pInfo->CoreID;
		topologyIdentifiers[2] = pInfo->PackageID;
		topologyIdentifiers[3] = pInfo->ClusterID;
	}
	
	/* Update all the domains in which the current processor exists */
	PROCESSOR_TOPOLOGY *currentTopology = &systemTopology;
	PROCESSOR_TOPOLOGY *domainZero;
	PROCESSOR_TOPOLOGY *domainNode;
	ULONG domainID;
	CLIST *domainList;
	BOOL domainBuiltLastTime = TRUE;

	LONG domainLevel = 3;
	while(domainLevel >= 0){
		if(!domainBuiltLastTime)
			SpinLock(&currentTopology->WriteLock);

		domainList = &(currentTopology->DomainList);
		domainZero = (PROCESSOR_TOPOLOGY *) domainList->ClnMain;
		domainID = topologyIdentifiers[domainLevel];
			
		if(domainZero != NULL){
			domainNode = domainZero;
			do {
				if(domainNode->DomainID == domainID)
					goto DomainBuildNotRequired;			
				domainNode = (PROCESSOR_TOPOLOGY *) (((CLNODE *) domainNode)->ClnNext);
			} while(domainNode != domainZero);
		} else
			goto DomainBuildRequired;
			
		DomainBuildRequired:
		domainNode = KNew(tPROCESSOR_TOPOLOGY, KM_NOSLEEP);
		domainNode->DomainID = domainID;
		domainNode->ParentDomain = currentTopology;
		if(domainLevel != 0)
			SpinLock(&domainNode->WriteLock);
		ClnInsert((CLNODE *) domainNode, CLN_FIRST, domainList);
		domainBuiltLastTime = TRUE;
		goto ContinueDomainBuild;

		DomainBuildNotRequired:
		if(!domainLevel){
			SpinUnlock(&currentTopology->WriteLock);
			return (MXRS_PROCESSOR_ALREADY_EXISTS);
		}
		domainBuiltLastTime = FALSE;
		
		ContinueDomainBuild:
		SpinUnlock(&currentTopology->WriteLock);
		currentTopology = domainNode;
		--(domainLevel);
	}

	currentTopology->Type = PROCESSOR_HIERARCHY_LOGICAL_CPU;
	currentTopology->DomainList.ClnMain = (CLNODE *) pCPU;/* Back register CPU */
	pCPU->DomainInfo = domainNode;
	MxIterateTopology(NULL, &UpdateProcessorCount, 4);
	return (MXRS_PROCESSOR_REGISTERED);
}

VOID MxIterateTopology(PROCESSOR *pCPU, VOID (*domainUpdater)(SCHED_DOMAIN *), ULONG domainLevel){
	pCPU = (pCPU == NULL) ? GetProcessorById(PROCESSOR_ID) : pCPU;
	SCHED_DOMAIN *schedDomain = pCPU->DomainInfo;
	while(schedDomain != NULL && domainLevel){
		SpinLock(&schedDomain->WriteLock);
		domainUpdater(schedDomain);
		SpinUnlock(&schedDomain->WriteLock);
		schedDomain = schedDomain->ParentDomain;
		--(domainLevel);
	}
}

VOID MxWithdrawTopology(){
	// TODO: Implement domain removl
}

VOID APMain(){
	PROCESSOR *sceProcessor = GetProcessorById(PROCESSOR_ID);
	SetupProcessor();
	MxRegisterProcessor();
	SetupRunqueue();

	extern VOID APWaitForPermit();
	APWaitForPermit();

	FlushTLB(0);
	SetupTick();
	while(TRUE);
}
