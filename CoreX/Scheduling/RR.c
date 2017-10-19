/*=++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  * File: RR.c
  *
  * Summary:
  * RR-scheduling-class is available for low-level tasks in the system that are executed when no
  * task is available.
  *
  * Copyright (C) 2017 - Shukant Pal
  *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=*/

#include <HAL/Processor.h>
#include <Exec/RR.h>
#include <KERNEL.h>

static VOID RrIncreaseLoad(SCHED_DOMAIN *rrSchedulerDomain){
	++rrSchedulerDomain->SchedDomain[RR_SCHED].DomainLoad;
}

static VOID RrDecreaseLoad(SCHED_DOMAIN *rrSchedulerDomain){
	--rrSchedulerDomain->SchedDomain[RR_SCHED].DomainLoad;
}

KRUNNABLE *Schedule_RR(TIME XMilliTime, PROCESSOR *cpu){
	SCHED_RR *rr = &cpu->RR;

	RrBalanceRoutine(cpu);/* Balance-routine is called every tick */

	SpinLock(&rr->Roller->RunqueueLock);	
	KRUNNABLE *lastRunner = rr->LastRunner;
	KRUNNABLE *nextRunner;
	if(lastRunner != NULL)
		nextRunner = lastRunner->RightLinker;
	else
		{ if(PROCESSOR_ID == 1) Dbg("_e"); nextRunner = (KRUNNABLE *) rr->Runqueue.ClnMain;}
	SpinUnlock(&rr->Roller->RunqueueLock);

	return (nextRunner);
}

VOID SaveRunner_RR(TIME XMilliTime, PROCESSOR *cpu){
	SCHED_RR *rr = &cpu->RR;
	rr->LastRunner = cpu->PoExT;
}

KRUNNABLE *UpdateRunner_RR(TIME XMilliTime, PROCESSOR *cpu){
	SCHED_RR *rr = &cpu->RR;

	rr->LastRunner = cpu->PoExT;
	return (Schedule_RR(XMilliTime, cpu));
}

VOID InsertRunner_RR(KRUNNABLE *runner, PROCESSOR *cpu){
	SCHED_RR *rr = &cpu->RR;
	KSCHED_ROLLER *roller = rr->Roller;
	SpinLock(&(roller->RunqueueLock));

	roller->RunnerLoad += 2;
	ClnInsert((CLNODE*) runner, CLN_FIRST, &rr->Runqueue); 
	MxIterateTopology(NULL, &RrIncreaseLoad, 4);

	SpinUnlock(&(roller->RunqueueLock));
}

VOID RemoveRunner_RR(KRUNNABLE *runner){
	SCHED_RR *rr = &runner->Processor->RR;
	KSCHED_ROLLER *roller = rr->Roller;
	SpinLock(&(roller->RunqueueLock));

	ClnRemove((CLNODE *) runner, &rr->Runqueue);
	roller->RunnerLoad -= 2;
	MxIterateTopology(NULL, &RrDecreaseLoad, 4);
	SpinUnlock(&(roller->RunqueueLock));
}

static inline
VOID RrMoveRunner(SCHED_RR *destinationRunqueue, SCHED_RR *sourceRunqueue, ULONG domainDiff, PROCESSOR *highCPU){
	KTASK *runner = sourceRunqueue->Runqueue.ClnMain;
	if(runner == highCPU->PoExT){
		runner = runner->RightLinker;
		if(runner == runner->RightLinker)
			return;
	}
	
	if(sourceRunqueue->LastRunner == runner)
		sourceRunqueue->LastRunner = NULL;
	destinationRunqueue->LastRunner = NULL;
		
	ClnRemove((CLNODE *) runner, &sourceRunqueue->Runqueue);
	ClnInsert((CLNODE *) runner, CLN_FIRST, &destinationRunqueue->Runqueue);

	MxIterateTopology(NULL, &RrIncreaseLoad, 4);
	MxIterateTopology(highCPU, &RrDecreaseLoad, 4);
}

VOID TransferBatchRunners(ULONG transferCount, PROCESSOR *lowCPU, PROCESSOR *highCPU, ULONG domainDiff){
	SCHED_RR *lowRunqueue = &lowCPU->RR;
	SCHED_RR *highRunqueue = &highCPU->RR;
	SpinLock(&lowRunqueue->Roller->RunqueueLock);
	SpinLock(&highRunqueue->Roller->RunqueueLock);

	CLIST *runnerList = (&highRunqueue->Runqueue);
	ULONG deltaLimit = (runnerList->ClnCount + transferCount) / 2;
	deltaLimit = LESS(deltaLimit, runnerList->ClnCount / 2);
	while(runnerList->ClnMain && deltaLimit > 0){
		RrMoveRunner(lowRunqueue, highRunqueue, domainDiff, highCPU);
		--(deltaLimit);
	}

	SpinUnlock(&highRunqueue->Roller->RunqueueLock);
	SpinUnlock(&lowRunqueue->Roller->RunqueueLock);
}

PROCESSOR *RrFindBusiestProcessor(SCHED_GROUP *busyGroup){
	SCHED_DOMAIN *searchDomain = busyGroup;
	CLIST *searchList;
	SCHED_GROUP *highestGroup;
	SCHED_GROUP *tryGroup;
	SCHED_GROUP *tryGroupZ;
	KSCHED_ROLLER_DOMAIN *highestRoller;
	KSCHED_ROLLER_DOMAIN *tryRoller;
	while(searchDomain->Type != PROCESSOR_HIERARCHY_LOGICAL_CPU){
		searchList = &(searchDomain->DomainList);
		tryGroupZ = searchList->ClnMain;
		tryGroup = tryGroupZ;
		highestGroup = NULL;
		highestRoller = NULL;
		tryRoller = &(tryGroup->SchedDomain[RR_SCHED]);
		do {
			if(highestGroup == NULL || (tryRoller->DomainLoad > highestRoller->DomainLoad)){
				highestGroup = tryGroup;
				highestRoller = &(highestGroup->SchedDomain[RR_SCHED]);
			}
		} while(tryGroup != tryGroupZ);
		
		searchDomain = highestGroup;
	}
	
	return (PROCESSOR *) (searchDomain->DomainList.ClnMain);
}

VOID RrBalanceRoutine(PROCESSOR *pCPU){
	SCHED_DOMAIN *searchDomain = pCPU->DomainInfo->ParentDomain;/* Start search for CPUs in same core/lowest toplogy*/
	CLIST *searchList;
	SCHED_GROUP *tryGroup;
	SCHED_GROUP *tryGroupZ;
	SCHED_GROUP *ourGroup = pCPU->DomainInfo;
	KSCHED_ROLLER_DOMAIN *searchRoller;
	KSCHED_ROLLER_DOMAIN *ourRoller;
	ULONG balancingLevel = 1;

	while(searchDomain != NULL){
		searchList = &(searchDomain->DomainList);
		tryGroupZ = searchList->ClnMain;
		tryGroup = tryGroupZ;
		do {
			if(tryGroup != ourGroup){
				searchRoller = &(tryGroup->SchedDomain[RR_SCHED]);
				ourRoller = &(ourGroup->SchedDomain[RR_SCHED]);
				if(searchRoller->DomainLoad > (ourRoller->DomainLoad + balancingLevel * balancingLevel)){
					TransferBatchRunners(balancingLevel * balancingLevel, pCPU, RrFindBusiestProcessor(tryGroup), balancingLevel - 1);
					break;
				}
			}
			tryGroup = tryGroup->NextDomain;
		} while(tryGroup != tryGroupZ);
		ourGroup = ourGroup->ParentDomain;	
		searchDomain = searchDomain->ParentDomain;
	}
}
