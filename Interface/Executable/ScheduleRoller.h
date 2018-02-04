/**
 * File: ScheduleClass.h
 *
 * ScheduleClass is the interface for dividing the scheduler into seperate
 * scheduler classes, namely - RT, CFS, RR.
 *
 * Copyright (C) 2017 - Shukant Pal
 */
#ifndef EXEC_SCHEDULECLASS_H
#define EXEC_SCHEDULECLASS_H

#include <Synch/Spinlock.h>
#include <Util/Stack.h>
#include "Task.hpp"

namespace HAL { struct Domain; struct Processor; }

namespace Executable
{
class RunqueueBalancer;
typedef unsigned long ScheduleClass;

enum
{
	ROUND_ROBIN = 0,
	COMPLETELY_FAIR = 1,
	SCHED_ROLLER_TYPES = 3
};

/**
 * Class: ScheduleRoller
 *
 * Summary:
 * Rolls schedules by allocating tasks to the core-scheduler. It is basically
 * a interface that a scheduler-module must implement to extend the base
 * scheduling mechanism. It is stored on a per-cpu basis which requires
 * seperate runqueues per-processor.
 *
 * Functions:
 * allocate - get any task to run on the cpu
 * free - take-back a task after a time-interval of executing it
 * add - spawn a new task in the system
 * remove - destroy the task from the system
 * transfer - move tasks to the other roller with the loaded transfer-config
 *
 * Author: Shukant Pal
 */
class ScheduleRoller
{
public:
	unsigned long getLoad()
	{
		return (load);
	}

	void request(unsigned long rload)
	{
		loadRequested = rload;
	}

	virtual Task *add(Task *newTask) = 0;
	virtual Task *allocate(Time t, HAL::Processor *cpu) = 0;
	virtual Task *update(Time t, HAL::Processor *cpu) = 0;
	virtual void free(Time at, HAL::Processor *cpu) = 0;
	virtual void remove(Task *tTask) = 0;
	virtual void send(HAL::Processor *proc, CircularList &list, unsigned long delta) = 0;
	virtual void recieve(Task *first, Task *last, unsigned long count, unsigned long load) = 0;
protected:
	ScheduleRoller() kxhide;
	virtual ~ScheduleRoller() kxhide;
public:
	unsigned long load;// current load
	unsigned long loadRequested;// requested-load for transfer
	unsigned long transferDelta;// delta on transfer
	Stack transferBuffer;// buffer for incoming tasks
	Spinlock lock;

	friend class RunqueueBalancer;
};

struct ScheduleDomain
{
	long load;// task-load for the domain
	Time balanceDelta;// timestamp for the next balance

	ScheduleDomain()
	{
		load = 0;
		balanceDelta = 0;
	}
};

}

#endif /* Exec/ScheduleClass.h */
