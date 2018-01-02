/**
 * File: RoundRobin.cpp
 *
 * Summary:
 * 
 * Functions:
 *
 * Origin:
 *
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

#include <Exec/RoundRobin.h>
#include <Exec/RunqueueBalancer.hpp>
#include <HAL/Processor.h>
#include <HAL/ProcessorTopology.hpp>
#include <KERNEL.h>

using namespace HAL;
using namespace Executable;

KTask *RoundRobin::add(KTask *newTask)
{
	newTask->next = mainTask;
	newTask->last = (mainTask) ? mainTask->last : NULL;

	__no_interrupts
	(
		mainTask = newTask;
		++(taskCount);
		ProcessorTopology::Iterator::toggleLoad(NULL, ROUND_ROBIN, 1);
	)

	return (newTask);
}

KTask *RoundRobin::allocate(TIME tstamp, Processor *proc)
{
	KTask *ntask;

	if(mostRecent == NULL)
	{
		ntask = mainTask;
	}
	else
	{
		ntask = mostRecent->next;
	}

	return (ntask);
}

KTask *RoundRobin::update(TIME time, Processor *proc)
{
	mostRecent = proc->ctask;
	RunqueueBalancer::balanceWork(ROUND_ROBIN);
	return mostRecent->next;
}

void RoundRobin::free(TIME tstamp, Processor *proc)
{
	mostRecent = proc->ctask;
}

void RoundRobin::remove(KTask *ttask)
{
	__no_interrupts
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
}

void RoundRobin::send(Processor *from, CircularList& list, unsigned long delta)
{
	KTask *first = mainTask;
	KTask *last = mainTask;

	if(delta < taskCount)
	{
		--delta;
		while(last != first && --delta)
			last = last->next;

		mainTask = first->last;
		KTask *newlast = last->next;
		mainTask->last = newlast;
		newlast->next = mainTask;

		list.lMain = (CircularListNode*) first;
		list.lMain->last = (CircularListNode*) last;
		last->next = first;
		list.count = delta;

		taskCount -= delta;

		ProcessorTopology::Iterator::toggleLoad(NULL, ROUND_ROBIN, -delta);
	}
	else
	{
		// Send all tasks;
		list.lMain = (CircularListNode*) first;
		list.count = taskCount;

		ProcessorTopology::Iterator::toggleLoad(NULL, ROUND_ROBIN, -taskCount);

		mainTask = NULL;
		taskCount = 0;
	}
}

void *RoundRobin::transfer(ScheduleRoller& idle)
{

}

RoundRobin::RoundRobin()
{
	this->mainTask =
			this->mostRecent =
					NULL;
	this->taskCount = 0;
	//DbgLine("__rr initialed");
}

// for the doggy compiler who requires this shit for dtor
inline void operator delete(void *d, unsigned int r){}


RoundRobin::~RoundRobin(){

}

KTask *RoundRobin::recieve(KTask *first, KTask *last, unsigned long load)
{
	/*
	 * Here, as threads are given fixed-quantums, load = no. of tasks
	 */

	KTask *tlast = first;
	while(--load && tlast != last)
		tlast = first->next;

	KTask *notTaken = tlast->next;
	KTask *oldmain = mainTask;
	mainTask = first;

	if(oldmain)
	{
		oldmain->last->next = mainTask;
		mainTask = oldmain->last;
		tlast->next = oldmain;
	}
	else
	{
		tlast->next = first;
		mainTask->last = tlast;
	}

	ProcessorTopology::Iterator::toggleLoad(NULL, ROUND_ROBIN, -load);

	return (notTaken);
}
