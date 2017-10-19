/* Copyright (C) 2017 - Shukant Pal */

#include <HAL/Processor.h>
#include <Exec/CFS.h>
#include <Exec/Scheduler.h>

VOID ScheduleInCFS(struct _PROCESSOR *cpu){
	SCHED_CFS *cfs = &cpu->CFS;
	KRUNNABLE *runner = MaxValueNode(cfs->RunnerMap.Root);
	return (runner);
}

VOID InsertRunnerInCFS(KRUNNABLE *runner, struct _PROCESSOR *cpu){
	SCHED_CFS *cfs = &cpu->CFS;
	KSCHED_ROLLER *cfsRoller = cfs->Roller;

	cfsRoller->RunnerLoad += 1;
	AVLInsert(&runner->Node, &cfs->RunnerMap);
}

VOID RemoveRunnerInCFS(KRUNNABLE *runner, struct _PROCESSOR *cpu){
	SCHED_CFS *cfs = &cpu->CFS;
	KSCHED_ROLLER *cfsRoller = cfs->Roller;

	if(AVLDelete((ULONG) runner, &cfs->RunnerMap))
		cfsRoller->RunnerLoad -= 1;
}
