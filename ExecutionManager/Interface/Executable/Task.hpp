/**
 * File: ETask.hpp
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

#ifndef MODULES_EXECUTIONMANAGER_INTERFACE_ETASK_HPP_
#define MODULES_EXECUTIONMANAGER_INTERFACE_ETASK_HPP_

#include <Executable/KTask.h>

#include <System/Time.hpp>

namespace Executable
{

typedef unsigned long ReturnStatus;

/**
 * Class: Task
 *
 * Summary:
 * Encapsulates a schedulable entity (KTask) that is used by the scheduler
 * subsystem to execute various tasks in the system. It provides the base
 * platform for deferred-ISRs, real-time tasks, kernel-routines,
 * kernel-threads, user-threads, subschedulers, thread-groups, user-mode
 * program schedulers, and many more in the kernel.
 *
 * Functions:
 * getTask() - get a Task-object for the corresponding KTask
 * broadcast() - signal all other tasks waiting for this one, to continue
 * 			their execution
 * exit() - exit execution, by self
 * destroy() - signals all tasks waiting on this, and ends its lifecycle
 * pause() - pause the task, until wakeup() is called
 * sleep() - sleep for a specific period of time, unless a signal wakes it
 * wakeup() - continues execution, if this was in a paused/waiting state
 * waitFor() - waits for the other-task to signal
 *
 * Author: Shukant Pal
 */
class Task
{
public:
	enum State
	{
		Start,
		Execute,
		Wait,
		Paused,
		Dead
	};

	static Task *getTask(KTask rinfo);
	void broadcast();
	void broadcast(Task *t);
	void exit(ReturnStatus rs);
	void destroy(ReturnStatus rs);
	void pause();
	void sleep(TIME dur);
	void wakeup();
	void waitFor(Task *other);
protected:
	KTask& rinfo;
	Task(KTask& rinfo);
};

}

#endif/* Executable/Task.hpp */
