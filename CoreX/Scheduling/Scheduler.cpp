/* Copyright (C) 2017 - Shukant Pal */

#include <Debugging.h>
#include <Executable/Scheduler.h>
#include <Memory/Pager.h>
#include <Util/IndexMap.h>

LinkedList Runqueue[32];
Spinlock SchedSynchronizer;

TIME XMilliTime; /* Maintain 64-bit kernel time. */

unsigned long PickRoller(PROCESSOR *tproc)
{
	unsigned long rollerIndex = 0;

	while(rollerIndex < SCHED_MAX)
	{
		if(tproc->scheduleClasses[rollerIndex].RunnerLoad > 0)
			return (rollerIndex);
		++rollerIndex;
	}

	return (SCHED_MAX);
}

export_asm void Schedule(PROCESSOR *tproc)
{
	ScheduleInfo *tsched = &tproc->crolStatus;
	Executable::ScheduleRoller *lrol = tsched->presRoll;
	Executable::ScheduleRoller *nrol = tproc->lschedTable[0];

	tsched->presRoll = nrol;
	KTask *ntask;

	if(nrol != lrol)
	{
		if(lrol != NULL)
		{
			lrol->free(XMilliTime, tproc);
		}

		ntask = nrol->allocate(XMilliTime, tproc);
	}
	else
	{
		ntask = nrol->update(XMilliTime, tproc);
	}

	tproc->ctask = ntask;
	if(ntask->mmu != NULL)
		SwitchContext(ntask->mmu);
}
