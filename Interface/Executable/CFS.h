/**
 * Copyright (C) 2017 - Shukant Pal
 */
#ifndef EXEC_CFS_H
#define EXEC_CFS_H

#include <Executable/KTask.h>
#include <Executable/ScheduleRoller.h>
#include <Util/AVLTree.h>

typedef
struct _SCHED_CFS {
	KSCHED_ROLLER *Roller;
	AVLTREE RunnerMap;
} SCHED_CFS;

void ScheduleCFS(
	TIME curTime,
	struct Processor *
);

void SaveCFS(
	TIME curTime,
	struct Processor *
);

void UpdateCFS(
	TIME curTime,
	struct Processor *
);

/**
 * InsertCFS() - 
 *
 * Summary:
 * This function inserts the KRUNNABLE to the CFS scheduler class.
 */
KRUNNABLE *InsertCFS(
	KRUNNABLE *runner,
	struct Processor *
);

void RemoveCFS(
	KRUNNABLE *runner,
	struct Processor *
);

#endif /* Exec/CFS.h */
