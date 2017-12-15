/* Copyright (C) 2017 - Shukant Pal */

#include <HAL/Processor.h>
#include <Exec/CFS.h>
#include <Exec/Scheduler.h>

void ScheduleInCFS(struct Processor *cpu)
{
	SCHED_CFS *cfs = &cpu->CFS;
	KRUNNABLE *runner = MaxValueNode(cfs->RunnerMap.Root);
	return (runner);
}

void InsertRunnerInCFS(KRUNNABLE *runner, struct Processor *cpu)
{
	SCHED_CFS *cfs = &cpu->CFS;
	KSCHED_ROLLER *cfsRoller = cfs->Roller;

	cfsRoller->RunnerLoad += 1;
	AVLInsert(&runner->Node, &cfs->RunnerMap);
}

void RemoveRunnerInCFS(KRUNNABLE *runner, struct Processor *cpu)
{
	SCHED_CFS *cfs = &cpu->CFS;
	KSCHED_ROLLER *cfsRoller = cfs->Roller;

	if(AVLDelete((unsigned long) runner, &cfs->RunnerMap))
		cfsRoller->RunnerLoad -= 1;
}
