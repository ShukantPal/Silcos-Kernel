/**
 * Copyright (C) 2017 - Shukant Pal
 */

#define NS_KMEMORYMANAGER

#include <Exec/Process.h>
#include <Exec/Thread.h>
#include <HAL/Processor.h>
#include <Memory/Pager.h>
#include <Memory/KMemorySpace.h>
#include <Memory/KMemoryManager.h>
#include <Memory/KObjectManager.h>
#include <Util/CtPrim.h>

CHAR *psName = "KPROCESS";
OBINFO *psInfo;


KPROCESS *psKernel;

KPROCESS *KeGetProcess(ID PID){
	if(PID > KERNEL_OFFSET) {
		KPAGE *psPage = (KPAGE*) KPG_AT((PID & ~(KPGSIZE - 1)));
		if(psPage->HashCode == (ULONG) psInfo)
			return (KPROCESS*) (PID);
	}

	return (NULL);
}

CHAR *msgInitProcess = "Setting up psKernel...";
VOID InitPTable(){
	DbgLine(msgInitProcess);
	psInfo = KiCreateType(psName, sizeof(KPROCESS), sizeof(ULONG), NULL, NULL);
	psKernel = KNew(psInfo, KM_NOSLEEP);
	KiSetupRunnable((KRUNNABLE*) psKernel, (ADDRESS) psKernel, ProcessTp);

	psKernel->Priority = 0xff;
	psKernel->Privelege = 0xff;

	GID(psKernel) = 0;
	UID(psKernel) = 0;

	psKernel->State = Process_Runnable;
	psKernel->AddressSpace = &SystemCxt;
	OwnerID(Context(psKernel)) = (ADDRESS) psKernel;

	memset(&EPI(psKernel), 0, sizeof(EPI));
}

CONTEXT *GetContext()
{
	PROCESSOR *pCPU = GetProcessorById(PROCESSOR_ID);
	KTHREAD *pThread = pCPU->PoExT;
	KPROCESS *pProcess = KeGetProcess(pThread->ParentID);
	return (pProcess->AddressSpace);
}