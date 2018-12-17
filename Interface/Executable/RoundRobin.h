/**
* Copyright (C) 2017 - Shukant Pal
 *
 * RR (or Round Robin) is the scheduler for uni-priority runner's that are CPU-bound
 * or are batch-runners. They are the lowest priority in the system and thus are given
 *
 */
#ifndef EXEC_RR_H
#define EXEC_RR_H

#include <Executable/ScheduleRoller.h>
#include "../Utils/CircularList.h"
#include "../Utils/RBTree.hpp"
#include "Task.hpp"

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
	Task *add(Task *newTask);
	Task *allocate(Time t, HAL::Processor *cpu);
	Task *update(Time t, HAL::Processor *cpu);
	void free(Time at, HAL::Processor *cpu);
	void remove(Task *tTask);
	void send(HAL::Processor *to, CircularList& list, unsigned long deltaLoad);
	void recieve(Task *first, Task *last, unsigned long count, unsigned long load);
	RoundRobin();
	~RoundRobin();
private:
	Task *mainTask;// circular list -> main task
	Task *mostRecent;// most recently run task
	unsigned long taskCount;// no. of tasks on this cpu
	CircularList pausedTasks;// tasks that are paused without any timer

};

}

#endif/* Exec/RR.h */
