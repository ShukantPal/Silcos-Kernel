/* Copyright (C) 2017 - Shukant Pal */

#include <Debugging.h>
#include <Exec/Scheduler.h>
#include <Memory/Pager.h>
#include <Util/IndexMap.h>

LINKED_LIST Runqueue[32];
Spinlock SchedSynchronizer;

TIME XMilliTime; /* Maintain 64-bit kernel time. */

ULONG PickRoller(PROCESSOR *curProcessor){
	ULONG rollerIndex = 0;
	while(rollerIndex < SCHED_MAX){
		if(curProcessor->ScheduleClasses[rollerIndex].RunnerLoad > 0)
			return (rollerIndex);
		++rollerIndex;
	}

	return (SCHED_MAX);
}

export_asm void Schedule(PROCESSOR *curProcessor){
	ULONG kRoller = PickRoller(curProcessor);
	if(kRoller == SCHED_MAX){ DbgInt(PROCESSOR_ID); DbgLine("FREEZE"); while(TRUE); }
	else {
		KSCHEDINFO *rtInfo = &curProcessor->SchedulerInfo;
		KSCHED_ROLLER *lastRoller = rtInfo->CurrentRoller;
		KSCHED_ROLLER *newRoller = &curProcessor->ScheduleClasses[kRoller];

		rtInfo->CurrentRoller = newRoller;
		KRUNNABLE *nextRunner;

		if(newRoller != lastRoller){
			if(lastRoller)
				lastRoller->SaveRunner(XMilliTime, curProcessor);
			nextRunner = newRoller->ScheduleRunner(XMilliTime, curProcessor);
		} else {
			nextRunner = newRoller->UpdateRunner(XMilliTime, curProcessor);
		}

		curProcessor->PoExT = nextRunner;
		if(nextRunner->Context != NULL)
			SwitchContext(nextRunner->Context);
	}
}
