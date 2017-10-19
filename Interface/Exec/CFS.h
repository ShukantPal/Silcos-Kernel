/**
 * Copyright (C) 2017 - Shukant Pal
 */
#ifndef EXEC_CFS_H
#define EXEC_CFS_H

#include "Exec.h"
#include "ScheduleClass.h"
#include <Util/AVLTree.h>

typedef
struct _SCHED_CFS {
	KSCHED_ROLLER *Roller;
	AVLTREE RunnerMap;
} SCHED_CFS;

VOID ScheduleCFS(
	TIME curTime,
	struct _PROCESSOR *
);

VOID SaveCFS(
	TIME curTime,
	struct _PROCESSOR *
);

VOID UpdateCFS(
	TIME curTime,
	struct _PROCESSOR *
);

/**
 * InsertCFS() - 
 *
 * Summary:
 * This function inserts the KRUNNABLE to the CFS scheduler class.
 */
VOID InsertCFS(
	KRUNNABLE *runner,
	struct _PROCESSOR *
);

VOID RemoveCFS(
	KRUNNABLE *runner,
	struct _PROCESSOR *
);

#endif /* Exec/CFS.h */
