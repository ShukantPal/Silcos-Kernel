/**
 * Copyright (C) 2017 - Shukant Pal
 *
 * RR (or Round Robin) is the scheduler for uni-priority runner's that are CPU-bound
 * or are batch-runners. They are the lowest priority in the system and thus are given

 */
#ifndef EXEC_RR_H
#define EXEC_RR_H

#include <Executable/KTask.h>
#include <Executable/ScheduleRoller.h>
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

namespace Executable
{

/**
 * Class: RoundRobin
 * Attributes: implement(ScheduleRoller)
 *
 * Summary:
 * Implements the round-robin scheme for scheduling tasks. It is primarily used
 * uniform execution of kernel background routines. CPU-bound tasks should also
 * be put in this class.
 *
 * Author: Shukant Pal
 */
class RoundRobin final : public ScheduleRoller
{
public:
	virtual KTask *add(KTask *newTask);
	virtual KTask *allocate(TIME t, Processor *cpu);
	virtual KTask *update(TIME t, Processor *cpu);
	virtual void free(TIME at, Processor *cpu);
	virtual void remove(KTask *tTask);
	virtual void send(Processor *to, CircularList& list, unsigned long deltaLoad);
	virtual void recieve(KTask *first, KTask *last, unsigned long count, unsigned long load);
	RoundRobin();
private:
	KTask *mainTask;// circular list -> main task
	KTask *mostRecent;// most recently run task
	unsigned long taskCount;// no. of tasks on this cpu
	~RoundRobin();
};

}

#endif/* Exec/RR.h */
