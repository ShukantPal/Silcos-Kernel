/**
 * Copyright (C) 2017 - Shukant Pal
 *
 * RR (or Round Robin) is the scheduler for uni-priority runner's that are CPU-bound
 * or are batch-runners. They are the lowest priority in the system and thus are given

 */
#ifndef EXEC_RR_H
#define EXEC_RR_H

#include "Exec.h"
#include "ScheduleClass.h"
#include <HAL/Processor.h>
#include <Util/CircularList.h>

typedef
struct SCHED_RR {
	KSCHED_ROLLER *Roller;
	KRUNNABLE *LastRunner;
	CLIST Runqueue;
} SCHED_RR;

KRUNNABLE *Schedule_RR(
	TIME curTime,
	PROCESSOR *cpu
);

void SaveRunner_RR(
	TIME curTime,
	PROCESSOR *cpu
);

KRUNNABLE *UpdateRunner_RR(
	TIME curTime,
	PROCESSOR *cpu
);

void InsertRunner_RR(
	KRUNNABLE *runner,
	PROCESSOR *cpu
);

void RemoveRunner_RR(
	KRUNNABLE *runner
);

#endif /* Exec/RR.h */
