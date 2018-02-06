/**
 * File: TimerEvent.hpp
 * Module: ExecutionManager (kernel.silcos.excmgr)
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
#ifndef EXCMGR_TIMER_EVENT_HPP__
#define EXCMGR_TIMER_EVENT_HPP__

#include <Atomic.hpp>
#include <Executable/Task.hpp>
#include <Executable/Timer/Time.hpp>
#include "../../Utils/LinkedList.h"

namespace Executable
{
namespace Timer// don't include if using another event-management subsystems
{

typedef void (*EventHandler)(class Event *);

class Event : public LinkedListNode
{
public:
	Time getTriggerPoint()
	{
		return (triggerPoint);
	}

	EventHandler getHandler()
	{
		return (handler);
	}

	bool setHandler(EventHandler other)
	{
		Atomic::xchg((unsigned long) other, (unsigned long*) &handler);
		return (!triggerSwitch);
	}

	bool triggered()
	{
		return (triggerSwitch);
	}

	static Event *create(Time delay, TimeAlign crt, EventHandler handler);
	bool cancel();
	bool finalize();
	void postpone(Time delay);
protected:
	Event(Time delay, TimeAlign crt, EventHandler handler);
	~Event();
private:
	Time triggerPoint;// point in time, when the event will be triggered
	EventHandler handler;
	unsigned long triggerSwitch;// indicates whether the event was triggered
};

}// namespace Timer
}// namespace Executable

#endif/* Executable/TimerEvent.hpp */
