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
#include <Executable/Thread.h>
#include <Executable/Scheduler.h>
#include <HAL/Processor.h>
#include <Memory/Address.h>
#include <Memory/Pager.h>
#include <Memory/KMemorySpace.h>
#include <Memory/KMemoryManager.h>
#include <Memory/KObjectManager.h>
#include <KernelRoutine/Init.h>

using namespace Executable;

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
	Dbg("stk-- "); EnsureUsability(kisBase, NULL, FLG_ATOMIC, KernelData); DbgLine(" --alloced");

	Processor *cpu = GetProcessorById(PROCESSOR_ID);
	cpu->ctask = (KTask*) kIdlerThread;
	kIdlerThread->Gate.next = (KTask*) kInitThread;

	KSCHEDINFO *bspSched = &cpu->crolStatus;
	bspSched->CurrentQuanta = 10;
	bspSched->LeftQuanta = 10;

	//cpu->lschedTable[0]->add((KTask*) kIdlerThread);
	cpu->lschedTable[0]->add((KTask*) kInitThread);
}

void Idle(){
	DbgLine("SMP:: AP::IDLE");
	while(TRUE);
}

void APThreadMain(){
	Dbg("APThreadMain"); DbgInt(PROCESSOR_ID); DbgLine(" ");
	
	while(TRUE){ asm volatile("nop"); };
}

#include <Executable/RunqueueBalancer.hpp>
void APInitService(){
	while(TRUE) { asm volatile("nop"); };
}

void SetupRunqueue()
{
	Processor *ap = GetProcessorById(PROCESSOR_ID);

	KTHREAD *idlerThread = (KTHREAD*) KNew(tdInfo, KM_SLEEP);
	KTHREAD *setupThread = (KTHREAD*) KNew(tdInfo, KM_SLEEP);
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
	idlerStack->base = ap->ProcessorStack;
	idlerStack->pointer = ap->ProcessorStack - 64;

	KSTACKINFO *setupStack = &(setupThread->KernelStack);
	unsigned long ssb = KiPagesAllocate(0, ZONE_KMODULE, FLG_ATOMIC);
	setupStack->base = ssb + KPGSIZE - 4;
	setupStack->pointer = ssb + KPGSIZE - 64;
	Dbg("d--"); EnsureUsability(ssb, NULL, FLG_ATOMIC, KernelData); DbgLine("--t");
	
	ap->ctask = (KTask*) idlerThread;
	idlerThread->Gate.next = (KTask*) setupThread;
	
	KSCHEDINFO *apSched = &ap->crolStatus;
	apSched->CurrentQuanta = 10;
	apSched->LeftQuanta = 10;
	
//	KSCHED_ROLLER *rrRoller = &processor->scheduleClasses[RR_SCHED];
//	rrRoller->InsertRunner((KRUNNABLE*) setupThread, processor);

	//ap->lschedTable[0]->add((KTask*) idlerThread);
	ap->lschedTable[0]->add((KTask*) setupThread);
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
	Processor *currentProcessor = GetProcessorById(PROCESSOR_ID);
	KTHREAD *newThread = KNew(tdInfo, KM_SLEEP);
	newThread->Gate.taskFlags = (1 << 0) | (1 << 1);
	newThread->Gate.eip = entry;
	newThread->Gate.mmu = NULL;
	newThread->Flags = 0;
	newThread->Status = Thread_Runnable;
	newThread->ParentID = (NULL);
	newThread->Gate.RRM_ID = log_id++;
	
	KSTACKINFO *threadStack = &(newThread->KernelStack);
	unsigned long stackAddress = KiPagesAllocate(3, ZONE_KMODULE, FLG_ATOMIC);
	threadStack->base = stackAddress + 8 * KPGSIZE - 4;
	threadStack->pointer = stackAddress + 8 * KPGSIZE - 64;
	
	unsigned long offset = 0;
	while(offset < 8)
	{
		//Dbg("mapstk--");
		EnsureUsability(stackAddress + offset * KPGSIZE, NULL, FLG_ATOMIC, KernelData);
		//DbgLine("--alloced");
		++offset;
	}

//	KSCHED_ROLLER *rrRoller = &(currentProcessor->scheduleClasses[RR_SCHED]);
//	rrRoller->InsertRunner((KTASK *) newThread, currentProcessor);

	currentProcessor->lschedTable[0]->add((KTask*)newThread);

	return (newThread);
}

void KThreadDestroy(){

}

void KThreadExit(){

}
