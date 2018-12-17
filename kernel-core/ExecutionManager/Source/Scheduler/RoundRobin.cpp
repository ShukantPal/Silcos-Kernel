/**
 * File: RoundRobin.cpp
 * -------------------------------------------------------------------
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 *
 * Copyright (C) 2017 - Shukant Pal
 */
#include <Executable/RoundRobin.h>
#include <Executable/RunqueueBalancer.hpp>
#include <HardwareAbstraction/Processor.h>
#include <HardwareAbstraction/ProcessorTopology.hpp>
#include <KERNEL.h>

using namespace HAL;
using namespace Executable;

Task *RoundRobin::add(Executable::Task *newTask)
{
	Processor *host = GetProcessorById(PROCESSOR_ID);

	__no_interrupts
(
		if(mainTask != null)
		{
			newTask->next = mainTask;
			newTask->last = mainTask->last;

			mainTask->last->next = newTask;
			mainTask->last = newTask;
		}
		else
		{
			newTask->last = newTask;
			newTask->next = newTask;
		}

		mainTask = newTask;
		++(taskCount);
		++(host->lschedTable[ROUND_ROBIN]->load);

		ProcessorTopology::Iterator::toggleLoad(host, ROUND_ROBIN, 1);
)

	return (newTask);
}

Executable::Task *RoundRobin::allocate(Time tstamp, Processor *proc)
{
	Executable::Task *ntask;

	if(mostRecent == NULL)
		ntask = mainTask;
	else
		ntask = mostRecent->next;

	return (ntask);
}

Executable::Task *RoundRobin::update(Time time, Processor *proc)
{
	mostRecent = proc->ctask;

	RunqueueBalancer::balanceWork(ROUND_ROBIN);
	return mostRecent->next;
}

void RoundRobin::free(Time tstamp, Processor *proc)
{
	mostRecent = (Executable::Task*) proc->ctask;
}

void RoundRobin::remove(Executable::Task *ttask) __no_interrupt_func
(
	if(mostRecent->next != ttask)
	{
		no_intr_ret;
	}
	else
	{
		if(ttask->next)
			ttask->next->last = ttask->last;

		if(ttask->last)
			ttask->last->next = ttask->next;

		if(ttask == mainTask)
			mainTask = ttask->next;

		if(ttask == mostRecent)
			mostRecent = ttask->last;
	}

	ProcessorTopology::Iterator::toggleLoad(NULL, ROUND_ROBIN, -1);
)

/**
 * Method: RoundRobin::send
 * Attributes: huge (really, really)
 *
 * Algorithm:
 * This function is supposed to fill the circular-list of tasks with 'delta'
 * count. The currently executing task cannot be transferred. It works by
 * following these steps ->
 *
 * 1. If delta is 1, then we just need to remove on task out of the list. No
 *    need for isolating a chain of tasks. But if delta is greater then goto
 *    step 4.
 *
 * 2. To remove any one task, there must be atleast one task that is not
 *    executing. That task is to be removed and put in the list
 *
 * Since: Silcos 2.05
 */
void RoundRobin::send(Processor *to, CircularList& list, unsigned long delta)
{
	Processor *from = GetProcessorById(PROCESSOR_ID);

	if(delta == 1)// just remove one
	{
		if(taskCount > 1 || from->ctask != mainTask)
		{
			Executable::Task *rem = mainTask;
			if(rem == from->ctask)
				rem = rem->next;

			rem->last->next = rem->next;
			rem->next->last = rem->last;
			--(taskCount);

			if(taskCount == 0)
				mainTask = null;
			else if(rem == mainTask)
				mainTask = rem->next;

			--(this->load);
			ProcessorTopology::Iterator::toggleLoad(from, ROUND_ROBIN, -1);

			list.lMain = (CircularListNode*) rem;
			rem->next = rem;
			rem->last = rem;
			list.count = 1;
		}
		else
		{
			list.lMain = null;
			list.count = 0;
		}

		return;
	}

	unsigned long orgDelta = delta;
	Executable::Task *first = mainTask;
	Executable::Task *last = mainTask->next;
	--delta;

	Executable::Task *executor = NULL;

	if(last == from->ctask)
		executor = last;

	while(--delta && last != first)
	{
		if(last == from->ctask)
		{
			executor = last;
			++delta;// executor ain't going to hell :)
		}

		last = last->next;
	}

	// 1. if ctask is in the chain, remove it.

	if(executor != null)
	{
		executor->last->next = executor->next;
		executor->next->last = executor->last;
	}

	// 2. now isolate the chain of outgoing tasks

	// if all tasks (other than executor) are getting out...
	if(last == first->last)
	{
		if(executor != null)
		{
			executor->next = executor;
			executor->last = executor;
		}
		else
		{
			mainTask = null;
		}
	} // else if tasks really need to be isolated...
	else
	{
		Executable::Task *newMain = first->last;

		if(executor == null)
		{
			last->next->last = newMain;
			newMain->next = last->next;
		}
		else
		{
			newMain->next = executor;
			executor->last = newMain;

			executor->next = last->next;
			last->next->last = executor;
		}

		first->last = last;
		last->next = first;
	}

	list.lMain = (CircularListNode*) first;
	list.count = orgDelta - delta;
	this->load -= list.count;
	taskCount = load;

	ProcessorTopology::Iterator::toggleLoad(NULL, ROUND_ROBIN, -list.count);
}

RoundRobin::RoundRobin()
{
	this->mainTask = this->mostRecent = null;
	this->taskCount = 0;
}

RoundRobin::~RoundRobin(){}

/**
 * Method: RoundRobin::recieve
 *
 * Summary:
 * Connects the chain of incoming tasks to the mainTask, therefore adding them
 * to the runqueue.
 *
 * Since: Silcos 2.05
 * Author: Shukant Pal
 */
void RoundRobin::recieve(Executable::Task *first, Executable::Task *last, unsigned long count, unsigned long load)
{
	if(mainTask)
	{
		last->next = mainTask->next;
		first->last = mainTask;

		mainTask->next->last = last;
		mainTask->next = first;
	}
	else
	{
		last->next = first;
		first->next = last;
		mainTask = first;
	}

	taskCount += count;
	this->load += count;

	ProcessorTopology::Iterator::toggleLoad(NULL, ROUND_ROBIN, +load);
}
