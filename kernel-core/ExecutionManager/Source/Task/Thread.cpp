/*=+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * File: Thread.c
 *
 * Summary:
 * Threading support is not trivial in the Silcos kernel. It is based on the generic interface for tasks
 * in the scheduler.
 *
 * Copyright (C) 2017 - Shukant Pal
 ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=*/

#define NS_KMEMORYMANAGER

#include <Debugging.h>
#include <HardwareAbstraction/Processor.h>
#include <Executable/Thread.h>
#include <Executable/Scheduler.h>
#include <HardwareAbstraction/Processor.h>
#include <Memory/Address.h>
#include <Memory/Pager.h>
#include <Memory/KMemorySpace.h>
#include <Memory/KMemoryManager.h>
#include <Memory/KObjectManager.h>
#include <KernelRoutine/Init.h>

using namespace Executable;
using namespace HAL;

const char *tdName = "Thread";
ObjectInfo *tdInfo;

Thread *kIdlerThread; /* BSP idler thread */
Thread *kInitThread; /* Kernel-comm thread */

Thread *KeGetThread(ID threadID)
{
	if (threadID > KDYNAMIC) {
		KPAGE *slabPage = (KPAGE*) KPG_AT((threadID & ~(KPGSIZE - 1)));
		if (slabPage->HashCode == (unsigned long) tdInfo)
			return ((Thread*) threadID);
	}

	return (NULL);
}

void KiThreadConstruct(void *kthreadPtr)
{
	Thread *thread = (Thread*) kthreadPtr;
	thread->Gate.userStack = &thread->UserStack;
	thread->Gate.kernelStack = &thread->KernelStack;
	thread->Gate.run = NULL;
}

char *msgInitThread = "Setting up kInitThread...";
void InitTTable(void)
{
	DbgLine(msgInitThread);

	tdInfo = KiCreateType(tdName, sizeof(Thread), sizeof(unsigned long),
			&KiThreadConstruct, NULL);
	kIdlerThread = (Thread*) KNew(tdInfo, KM_NOSLEEP);
	kInitThread = (Thread*) KNew(tdInfo, KM_NOSLEEP);

	/* Initialize Fields */
	Thread *kInit = kInitThread;
	KiSetupRunnable((Executable::Task*) kInit, (ADDRESS) kInit, ThreadTp);
	kInit->Gate.taskFlags = (1 << 0) | (1 << 1);
	kInit->Gate.eip = (void*) &Init;
	kInit->Gate.mmu = NULL;
	kInit->Priority = 0xFF;
	kInit->Privelege = 0xFF;
	kInit->Flags = 0;
	kInit->Status = Thread_Runnable;
	kInit->ProgramCounter = &Init;
	kInit->ParentID = (ID) NULL;

	/* Setup Stack */
	CPUStack *kInitStack = &(kInit->KernelStack);
	ADDRESS kisBase = KiPagesAllocate(0, ZONE_KMODULE, FLG_ATOMIC);
	kInitStack->base = kisBase + KPGSIZE - 4;
	kInitStack->pointer = kisBase + KPGSIZE - 64;
	Pager::use(kisBase, FLG_ATOMIC, KernelData);

	Processor *cpu = GetProcessorById(PROCESSOR_ID);
	cpu->ctask = (Executable::Task*) kIdlerThread;
	kIdlerThread->Gate.next = (Executable::Task*) kInitThread;

	KSCHEDINFO *bspSched = &cpu->crolStatus;
	bspSched->CurrentQuanta = 10;
	bspSched->LeftQuanta = 10;

	cpu->lschedTable[0]->add((Executable::Task*) kInitThread);
}

void Idle()
{
	DbgLine("SMP:: AP::IDLE");
	while (TRUE)
		;
}

void APThreadMain()
{
	Dbg("APThreadMain");
	DbgInt(PROCESSOR_ID);
	DbgLine(" ");

	while (TRUE) {
		asm volatile("nop");
	};
}

#include <Executable/RunqueueBalancer.hpp>
void APInitService()
{
	while (TRUE) {
		asm volatile("nop");
	};
}

void SetupRunqueue()
{
	Processor *ap = GetProcessorById(PROCESSOR_ID);

	Thread *idlerThread = (Thread*) KNew(tdInfo, KM_SLEEP);
	Thread *setupThread = (Thread*) KNew(tdInfo, KM_SLEEP);
	KiSetupRunnable((Executable::Task*) idlerThread, (ADDRESS) idlerThread,
			ThreadTp);

	KiSetupRunnable((Executable::Task*) setupThread, (ADDRESS) setupThread,
			ThreadTp);
	setupThread->Gate.taskFlags = (1 << 0) | (1 << 1);
	setupThread->Gate.eip = (void*) &APInitService;
	setupThread->Gate.mmu = NULL;
	setupThread->Flags = 0;
	setupThread->Status = Thread_Runnable;
	setupThread->ParentID = (ID) NULL;

	CPUStack *idlerStack = &(idlerThread->KernelStack);
	idlerStack->base = ap->ProcessorStack;
	idlerStack->pointer = ap->ProcessorStack - 64;

	CPUStack *setupStack = &(setupThread->KernelStack);
	unsigned long ssb = KiPagesAllocate(0, ZONE_KMODULE, FLG_ATOMIC);
	setupStack->base = ssb + KPGSIZE - 4;
	setupStack->pointer = ssb + KPGSIZE - 64;
	Dbg("d--");
	Pager::use(ssb, FLG_ATOMIC, KernelData);
	DbgLine("--t");

	ap->ctask = (Executable::Task*) idlerThread;
	idlerThread->Gate.next = (Executable::Task*) setupThread;

	KSCHEDINFO *apSched = &ap->crolStatus;
	apSched->CurrentQuanta = 10;
	apSched->LeftQuanta = 10;

	ap->lschedTable[0]->add((Executable::Task*) setupThread);
}

Thread *KThreadCreate(void *entry)
{
	Processor *currentProcessor = GetProcessorById(PROCESSOR_ID);
	Thread *newThread = (Thread*) KNew(tdInfo, KM_SLEEP);
	newThread->Gate.taskFlags = (1 << 0) | (1 << 1);
	newThread->Gate.eip = entry;
	newThread->Gate.mmu = NULL;
	newThread->Flags = 0;
	newThread->Status = Thread_Runnable;
	newThread->ParentID = (NULL);

	CPUStack *threadStack = &(newThread->KernelStack);
	unsigned long stackAddress = KiPagesAllocate(3, ZONE_KMODULE, FLG_ATOMIC);
	threadStack->base = stackAddress + 8 * KPGSIZE - 4;
	threadStack->pointer = stackAddress + 8 * KPGSIZE - 64;

	unsigned long offset = 0;
	while (offset < 8) {
		Pager::use(stackAddress + offset * KPGSIZE, FLG_ATOMIC, KernelData);
		++offset;
	}

	currentProcessor->lschedTable[0]->add((Executable::Task*) newThread);
	return (newThread);
}
