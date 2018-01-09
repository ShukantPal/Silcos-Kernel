/**
 * Copyright (C) 2017 - Shukant Pal
 *
 * 
 */

#ifndef EXEC_SCHEDULER_H
#define EXEC_SCHEDULER_H

#include <HAL/Processor.h>
#include <Debugging.h>
#include <Executable/KTask.h>
#include <Executable/Thread.h>
#include <Synch/Spinlock.h>

#define NoThreadExecutable 140

#define RT_SCHED 0
#define CFS_SCHED 1
#define RR_SCHED 2
#define SCHED_MAX 3

enum
{
	RT_Queue = 0,
	RR_Queue = 1,
	FIFO_Queue = 2
};

extern
LinkedList Runqueue[32];

extern /* Sleeping threads */
LinkedList Sleepqueue;

extern
SPIN_LOCK SchedSynchronizer;

extern
TIME XMilliTime;

#define InitSchedOperation SpinLock(&SchedSynchronizer)
#define CompleteSchedOperation SpinUnlock(&SchedSynchronizer)

export_asm void Schedule(Processor*);

static inline TIME getSystemTime()
{
	return (XMilliTime);
}

#endif
