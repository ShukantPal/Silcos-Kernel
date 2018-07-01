///
/// @file EventTrigger.hpp
/// -------------------------------------------------------------------
/// This program is free software: you can redistribute it and/or modify
/// it under the terms of the GNU General Public License as published by
/// the Free Software Foundation, either version 3 of the License, or
/// (at your option) any later version.
///
/// This program is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
/// GNU General Public License for more details.
///
/// You should have received a copy of the GNU General Public License
/// along with this program.  If not, see <http://www.gnu.org/licenses/>
///
/// Copyright (C) 2017 - Shukant Pal
///

#ifndef EVENTTRIGGER_HPP_
#define EVENTTRIGGER_HPP_

#include <Atomic.hpp>
#include <Heap.hpp>
#include <Executable/Task.hpp>
#include <Executable/Timer/Time.hpp>
#include <Synch/ReadWriteSerializer.h>
#include <Utils/Arrays.hpp>
#include <Utils/LinkedList.h>

namespace Executable
{
namespace Timer// don't include if using another event-management subsystems
{

typedef void (*EventCallback)(void *);

typedef U64 Timestamp;//! Relative timestamp with nanosecond precision
typedef U64 Delay;//! Nanosecond delay in time

///
/// Trigger-object for an event will should be executed in a fixed-range
/// in time.
///
struct EventTrigger final
{
	Timestamp triggerRange[2];//!< Range in which the event should be
	 	 	 	  //! be executed by calling the functor given.

	EventCallback handler;//!< The functor to be called at execution time.

	union
	{
		void *eventObject;
		struct
		{
			unsigned long live		: 1;
			unsigned long weakPaused: 1;
			unsigned long reserved	: 2;
		};
	};

	EventTrigger(Timestamp trigger, Delay shiftAllowed,
			EventCallback handler, void *eventObject)
	{
		triggerRange[0] = trigger;
		triggerRange[1] = trigger + shiftAllowed;
		this->handler = handler;
		this->eventObject = eventObject;
		this->live = 1;
	}

	void execute()
	{
		handler(eventObject);
	}
};

}// namespace Timer
}// namespace Executable

#endif /* EVENTTRIGGER_HPP_ */
