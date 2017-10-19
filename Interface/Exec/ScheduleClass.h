/*=+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
 * File: ScheduleClass.h
 *
 * ScheduleClass is the interface for dividing the scheduler into seperate
 * scheduler classes, namely - RT, CFS, RR.
 *
 * Copyright (C) 2017 - Shukant Pal
 *+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=*/
#ifndef EXEC_SCHEDULECLASS_H
#define EXEC_SCHEDULECLASS_H

#include "Exec.h"
#include "Scheduler.h"
#include <Synch/Spinlock.h>
#include <Util/Stack.h>

/**
 * KSCHED_ROLLER  -
 *
 * Summary:
 * The kernel scheduler roller (or KSCHED_ROLLER) is a type used for interfacing
 * with various scheduler classes. It contains the functions (ptr) for operating with
 * the scheduler. It is present on every CPU.
 *
 * @Version 1
 * @Since Circuit 2.03
 */
typedef
struct _KSCHED_ROLLER {
	ULONG RunnerLoad;
	SPIN_LOCK RunqueueLock;

	STACK RqTransferBuffer;
	ULONG RqLoadRequired;
	ULONG RqTransferDiff;
	
	KRUNNABLE *(*ScheduleRunner)(
		TIME curTime,
		struct _PROCESSOR *
	);/* Actual scheduling algorithm*/

	VOID *(*SaveRunner)(
		TIME curTime,
		struct _PROCESSOR *
	);/* Save state for switching to other scheduler */

	/* Clock-tick occured, thus, update the runner (if required) */
	KRUNNABLE *(*UpdateRunner)(TIME curTime, struct _PROCESSOR *);

	/* Used for inserting a new runner */
	VOID (*InsertRunner) (KRUNNABLE *runner, struct _PROCESSOR *);

	/* Used for removing a runner */
	VOID (*RemoveRunner) (KRUNNABLE *runner);

	/* Used for transfering of load (SMP). */
	VOID (*TransferRunners) (struct _KSCHED_ROLLER *roller); 
} KSCHED_ROLLER;

typedef
struct {
	UINT DomainLoad;
	TIME BalanceInterval;
} KSCHED_ROLLER_DOMAIN;

#endif /* Exec/ScheduleClass.h */
