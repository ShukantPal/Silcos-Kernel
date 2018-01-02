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
#include <HAL/Processor.h>
#include <Exec/Thread.h>
#include <Exec/Process.h>
#include <Exec/SchedList.h>
#include <Exec/Scheduler.h>
#include <HAL/Processor.h>
#include <Memory/Address.h>
#include <Memory/Pager.h>
#include <Memory/KMemorySpace.h>
#include <Memory/KMemoryManager.h>
#include <Memory/KObjectManager.h>
#include <KernelRoutine/Init.h>

const char *tdName = "KTHREAD";
ObjectInfo *tdInfo;

KTHREAD *kIdlerThread; /* BSP idler thread */
KTHREAD *kInitThread; /* Kernel-comm thread */

KTHREAD *KeGetThread(ID threadID){
	if(threadID > KDYNAMIC){
		KPAGE *slabPage = (KPAGE*) KPG_AT((threadID & ~(KPGSIZE - 1)));
		if(slabPage->HashCode == (unsigned long) tdInfo)
			return ((KTHREAD*) threadID);
	}

	return (NULL);
}

void KiThreadConstruct(void *kthreadPtr){
	KTHREAD *thread = (KTHREAD*) kthreadPtr;
	thread->Gate.id = (unsigned long) kthreadPtr;
	thread->Gate.userStack = &thread->UserStack;
	thread->Gate.kernelStack = &thread->KernelStack;	
	thread->Gate.run = NULL;
}

char *msgInitThread = "Setting up kInitThread...";
void InitTTable(void){
	DbgLine(msgInitThread);

	tdInfo = KiCreateType(tdName, sizeof(KTHREAD), sizeof(unsigned long), &KiThreadConstruct, NULL);
	kIdlerThread = KNew(tdInfo, KM_NOSLEEP);
	kInitThread = KNew(tdInfo, KM_NOSLEEP);

	/* Initialize Fields */
	KTHREAD *kInit = kInitThread;
	KiSetupRunnable((KRUNNABLE*) kInit, (ADDRESS) kInit, ThreadTp);
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
	KSTACKINFO *kInitStack = &(kInit->KernelStack);
	ADDRESS kisBase = KiPagesAllocate(0, ZONE_KMODULE, FLG_ATOMIC);
	kInitStack->base = kisBase + KPGSIZE - 4;
	kInitStack->pointer = kisBase + KPGSIZE - 64;
	EnsureUsability(kisBase, NULL, FLG_ATOMIC, KernelData);

	PROCESSOR *cpu = GetProcessorById(PROCESSOR_ID);
	cpu->ctask = (void*) kIdlerThread;
	kIdlerThread->Gate.next = (KRUNNABLE*) kInitThread;

	KSCHEDINFO *bspSched = &cpu->crolStatus;
	bspSched->CurrentQuanta = 10;
	bspSched->LeftQuanta = 10;

	KSCHED_ROLLER *rrRoller = &cpu->scheduleClasses[RR_SCHED];
	rrRoller->InsertRunner((KRUNNABLE*) kInitThread, cpu);
}

void Idle(){
	DbgLine("SMP:: AP::IDLE");
	while(TRUE);
}

void APThreadMain(){
	Dbg("APThreadMain"); DbgInt(PROCESSOR_ID); DbgLine(" ");
	
	while(TRUE){ asm volatile("nop"); };
}

void APInitService(){
	DbgLine("APInitService");

	KThreadCreate((void*) &APThreadMain);
	KThreadCreate((void*)&APThreadMain);
	KThreadCreate((void*)&APThreadMain);

	while(TRUE) { asm volatile("nop"); };
}

void SetupRunqueue(){
	PROCESSOR *processor = GetProcessorById(PROCESSOR_ID);

	KTHREAD *idlerThread = KNew(tdInfo, KM_SLEEP);
	KTHREAD *setupThread = KNew(tdInfo, KM_SLEEP);
	KiSetupRunnable((KRUNNABLE*) idlerThread, (ADDRESS) idlerThread, ThreadTp);
		
	idlerThread->Gate.RRM_ID = 100;
		
	KiSetupRunnable((KRUNNABLE*) setupThread, (ADDRESS) setupThread, ThreadTp);
	setupThread->Gate.taskFlags = (1 << 0) | (1 << 1);
	setupThread->Gate.eip = (void*) &APInitService;
	setupThread->Gate.mmu = NULL;
	setupThread->Flags = 0;
	setupThread->Status = Thread_Runnable;
	setupThread->ParentID = (ID) NULL;
	setupThread->Gate.RRM_ID = 9;
	
	KSTACKINFO *idlerStack = &(idlerThread->KernelStack);
	idlerStack->base = processor->ProcessorStack;
	idlerStack->pointer = processor->ProcessorStack - 64;

	KSTACKINFO *setupStack = &(setupThread->KernelStack);
	unsigned long ssb = KiPagesAllocate(0, ZONE_KMODULE, FLG_ATOMIC);
	setupStack->base = ssb + KPGSIZE - 4;
	setupStack->pointer = ssb + KPGSIZE - 64;
	EnsureUsability(ssb, NULL, FLG_ATOMIC, KernelData);
	
	processor->ctask = idlerThread;
	idlerThread->Gate.next = (EXEC *) setupThread;
	
	KSCHEDINFO *apSched = &processor->crolStatus;
	apSched->CurrentQuanta = 10;
	apSched->LeftQuanta = 10;
	
	KSCHED_ROLLER *rrRoller = &processor->scheduleClasses[RR_SCHED];
	rrRoller->InsertRunner((KRUNNABLE*) setupThread, processor);
}

int log_id = 2;
/**
 * Function: ThreadManager::createThread (will be changed)
 *
 * Summary:
 * This function creates a kernel-thread by its entry-point.
 *
 * Author: Shukant Pal
 */
KTHREAD *KThreadCreate(void *entry){
	PROCESSOR *currentProcessor = GetProcessorById(PROCESSOR_ID);
	KTHREAD *newThread = KNew(tdInfo, KM_SLEEP);
	newThread->Gate.taskFlags = (1 << 0) | (1 << 1);
	newThread->Gate.eip = entry;
	newThread->Gate.mmu = NULL;
	newThread->Flags = 0;
	newThread->Status = Thread_Runnable;
	newThread->ParentID = (NULL);
	newThread->Gate.RRM_ID = log_id++;
	
	KSTACKINFO *threadStack = &(newThread->KernelStack);
	unsigned long stackAddress = KiPagesAllocate(2, ZONE_KMODULE, FLG_ATOMIC);
	threadStack->base = stackAddress + 8 * KPGSIZE - 4;
	threadStack->pointer = stackAddress + 8 * KPGSIZE - 64;
	
	unsigned long offset = 0;
	while(offset < 8)
	{
		EnsureUsability(stackAddress + offset * KPGSIZE, NULL, FLG_ATOMIC, KernelData);
		++offset;
	}

	KSCHED_ROLLER *rrRoller = &(currentProcessor->scheduleClasses[RR_SCHED]);
	rrRoller->InsertRunner((KTASK *) newThread, currentProcessor);
	return (newThread);
}

void KThreadDestroy(){

}

void KThreadExit(){

}
