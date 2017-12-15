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
	unsigned long RunnerLoad;
	SPIN_LOCK RunqueueLock;

	STACK RqTransferBuffer;
	unsigned long RqLoadRequired;
	unsigned long RqTransferDiff;
	
	KRUNNABLE *(*ScheduleRunner)(
		TIME curTime,
		struct Processor *
	);/* Actual scheduling algorithm*/

	void *(*SaveRunner)(
		TIME curTime,
		struct Processor *
	);/* Save state for switching to other scheduler */

	/* Clock-tick occured, thus, update the runner (if required) */
	KRUNNABLE *(*UpdateRunner)(TIME curTime, struct Processor *);

	/* Used for inserting a new runner */
	void (*InsertRunner) (KRUNNABLE *runner, struct Processor *);

	/* Used for removing a runner */
	void (*RemoveRunner) (KRUNNABLE *runner);

	/* Used for transfering of load (SMP). */
	void (*TransferRunners) (struct _KSCHED_ROLLER *roller); 
} KSCHED_ROLLER;

typedef
struct {
	unsigned int DomainLoad;
	TIME BalanceInterval;
} KSCHED_ROLLER_DOMAIN;

#endif /* Exec/ScheduleClass.h */
