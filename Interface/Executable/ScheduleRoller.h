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

#include <Executable/KTask.h>
#include <Synch/Spinlock.h>
#include <Util/Stack.h>

namespace HAL
{
	struct Domain;
}

namespace Executable
{
class RunqueueBalancer;
typedef unsigned long ScheduleClass;

enum
{
	ROUND_ROBIN = 0,
	COMPLETELY_FAIR = 1
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

	/*
	 * Adds the unhosted task into the processor's runqueue in the
	 * scheduling class. It can be called only from outside a interrupt
	 * context.
	 */
	virtual KTask *add(KTask *newTask) = 0;

	/*
	 * Rolls the next task that should be executed when this class
	 * was picked for scheduling (and the previous one was diff.)
	 */
	virtual KTask *allocate(TIME t, Processor *cpu) = 0;

	/*
	 * Rolls the next task assuming that the previous class of tasks
	 * running was the same.
	 */
	virtual KTask *update(TIME t, Processor *cpu) = 0;

	/*
	 * Takes the tasks running back into the runqueue, allowing a
	 * diff. scheduling class to execute now.
	 */
	virtual void free(TIME at, Processor *cpu) = 0;

	/*
	 * Function: ScheduleRoller::remove
	 * Attributes: virtual, interface
	 *
	 * Summary:
	 * Removes the task from the runqueue permanently allowing it to be
	 * shifted or destroyed. This can be called only outside of a interrupt
	 * context.
	 *
	 * Author: Shukant Pal
	 */
	virtual void remove(KTask *tTask) = 0;

	/*
	 * Function: ScheduleRoller::send
	 * Attributes: virtual, interface
	 *
	 * Summary:
	 * Fills a circular-list with tasks which hold the load of delta. The
	 * recieving processor is also given, incase it is of any importance of
	 * calculating loads.
	 *
	 * Author: Shukant Pal
	 */
	virtual void send(Processor *proc, CircularList &list, unsigned long delta) = 0;

	/*
	 * Function: ScheduleRoller::recieve
	 * Attributes: virtual, interface
	 *
	 * Summary:
	 * This function must take all tasks from the chain (first --- last)
	 * and is used by the runqueue-balancer.
	 *
	 * Author: Shukant Pal
	 */
	virtual void recieve(KTask *first, KTask *last, unsigned long count, unsigned long load) = 0;
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
	TIME balanceDelta;// timestamp for the next balance

	ScheduleDomain()
	{
		load = 0;
		balanceDelta = 0;
	}
};

}

#endif /* Exec/ScheduleClass.h */
