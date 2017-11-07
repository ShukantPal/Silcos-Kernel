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

CHAR *tdName = "KTHREAD";
OBINFO *tdInfo;

KTHREAD *kIdlerThread; /* BSP idler thread */
KTHREAD *kInitThread; /* Kernel-comm thread */

KTHREAD *KeGetThread(ID threadID){
	if(threadID > KDYNAMIC){
		KPAGE *slabPage = (KPAGE*) KPG_AT((threadID & ~(KPGSIZE - 1)));
		if(slabPage->HashCode == (ULONG) tdInfo)
			return ((KTHREAD*) threadID);
	}

	return (NULL);
}

VOID KiThreadConstruct(VOID *kthreadPtr){
	KTHREAD *thread = (KTHREAD*) kthreadPtr;
	thread->Gate.ID = (ULONG) kthreadPtr;
	thread->Gate.UserStack = &thread->UserStack;
	thread->Gate.KernelStack = &thread->KernelStack;	
	thread->Gate.Run = NULL;
}

CHAR *msgInitThread = "Setting up kInitThread...";
VOID InitTTable(VOID){
	DbgLine(msgInitThread);

	tdInfo = KiCreateType(tdName, sizeof(KTHREAD), sizeof(ULONG), &KiThreadConstruct, NULL);
	kIdlerThread = KNew(tdInfo, KM_NOSLEEP);
	kInitThread = KNew(tdInfo, KM_NOSLEEP);

	/* Initialize Fields */
	KTHREAD *kInit = kInitThread;
	KiSetupRunnable((KRUNNABLE*) kInit, (ADDRESS) kInit, ThreadTp);
	kInit->Gate.TaskFlags = (1 << 0) | (1 << 1);
	kInit->Gate.EIP = &Init;
	kInit->Gate.Context = NULL;
	kInit->Priority = 0xFF;
	kInit->Privelege = 0xFF;
	kInit->Flags = 0;
	kInit->Status = Thread_Runnable;
	kInit->ProgramCounter = &Init;
	kInit->ParentID = (ID) NULL;

	/* Setup Stack */
	KSTACKINFO *kInitStack = &(kInit->KernelStack);
	ADDRESS kisBase = KiPagesAllocate(0, ZONE_KMODULE, FLG_ATOMIC);
	kInitStack->Base = kisBase + KPGSIZE - 4;
	kInitStack->Pointer = kisBase + KPGSIZE - 64;
	EnsureUsability(kisBase, NULL, FLG_ATOMIC, KernelData);

	PROCESSOR *cpu = GetProcessorById(PROCESSOR_ID);
	cpu->PoExT = (VOID*) kIdlerThread;
	kIdlerThread->Gate.RightLinker = (KRUNNABLE*) kInitThread;

	KSCHEDINFO *bspSched = &cpu->SchedulerInfo;
	bspSched->CurrentQuanta = 10;
	bspSched->LeftQuanta = 10;

	KSCHED_ROLLER *rrRoller = &cpu->ScheduleClasses[RR_SCHED];
	rrRoller->InsertRunner((KRUNNABLE*) kInitThread, cpu);
}

VOID Idle(){
	DbgLine("SMP:: AP::IDLE");
	while(TRUE);
}

VOID APThreadMain(){
	Dbg("APThreadMain"); DbgInt(PROCESSOR_ID); DbgLine(" ");
	
	while(TRUE){ asm volatile("nop"); };
}

KTHREAD* KThreadCreate(VOID (*)());

VOID APInitService(){
	DbgLine("APInitService");

	KThreadCreate(&APThreadMain);
	KThreadCreate(&APThreadMain);
	KThreadCreate(&APThreadMain);

	while(TRUE) { asm volatile("nop"); };
}

VOID SetupRunqueue(){
	PROCESSOR *processor = GetProcessorById(PROCESSOR_ID);

	KTHREAD *idlerThread = KNew(tdInfo, KM_SLEEP);
	KTHREAD *setupThread = KNew(tdInfo, KM_SLEEP);
	KiSetupRunnable((KRUNNABLE*) idlerThread, (ADDRESS) idlerThread, ThreadTp);
		
	idlerThread->Gate.RRM_ID = 100;
		
	KiSetupRunnable((KRUNNABLE*) setupThread, (ADDRESS) setupThread, ThreadTp);
	setupThread->Gate.TaskFlags = (1 << 0) | (1 << 1);
	setupThread->Gate.EIP = &APInitService;
	setupThread->Gate.Context = NULL;
	setupThread->Flags = 0;
	setupThread->Status = Thread_Runnable;
	setupThread->ParentID = (ID) NULL;
	setupThread->Gate.RRM_ID = 9;
	
	KSTACKINFO *idlerStack = &(idlerThread->KernelStack);
	idlerStack->Base = processor->ProcessorStack;
	idlerStack->Pointer = processor->ProcessorStack - 64;

	KSTACKINFO *setupStack = &(setupThread->KernelStack);
	ULONG ssb = KiPagesAllocate(0, ZONE_KMODULE, FLG_ATOMIC);
	setupStack->Base = ssb + KPGSIZE - 4;
	setupStack->Pointer = ssb + KPGSIZE - 64;
	EnsureUsability(ssb, NULL, FLG_ATOMIC, KernelData);
	
	processor->PoExT = idlerThread;
	idlerThread->Gate.RightLinker = (EXEC *) setupThread;
	
	KSCHEDINFO *apSched = &processor->SchedulerInfo;
	apSched->CurrentQuanta = 10;
	apSched->LeftQuanta = 10;
	
	KSCHED_ROLLER *rrRoller = &processor->ScheduleClasses[RR_SCHED];
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
KTHREAD *KThreadCreate(VOID (*Entry)()){
	PROCESSOR *currentProcessor = GetProcessorById(PROCESSOR_ID);
	KTHREAD *newThread = KNew(tdInfo, KM_SLEEP);
	newThread->Gate.TaskFlags = (1 << 0) | (1 << 1);
	newThread->Gate.EIP = Entry;
	newThread->Gate.Context = NULL;
	newThread->Flags = 0;
	newThread->Status = Thread_Runnable;
	newThread->ParentID = (NULL);
	newThread->Gate.RRM_ID = log_id++;
	
	KSTACKINFO *threadStack = &(newThread->KernelStack);
	ULONG stackAddress = KiPagesAllocate(0, ZONE_KMODULE, FLG_ATOMIC);
	threadStack->Base = stackAddress + KPGSIZE - 4;
	threadStack->Pointer = stackAddress + KPGSIZE - 64;
	EnsureUsability(stackAddress, NULL, FLG_ATOMIC, KernelData);
	
	KSCHED_ROLLER *rrRoller = &(currentProcessor->ScheduleClasses[RR_SCHED]);
	rrRoller->InsertRunner((KTASK *) newThread, currentProcessor);
	return (newThread);
}

VOID KThreadDestroy(){

}

VOID KThreadExit(){

}
