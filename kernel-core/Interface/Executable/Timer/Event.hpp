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

#ifndef EXECMGR_TIMER_EVENT_HPP_
#define EXECMGR_TIMER_EVENT_HPP_

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

/**
 * The event-triggering object is used in timer queues to hold, in a
 * modifiable form, pending soft-timers that are to be fired using the
 * hardware timer device in question.
 *
 * It holds critical data like: time-range in which it should be fired,
 * the handler to called, and a single argument passed to the handler.
 *
 *
 */
struct Event final
{
	Timestamp triggerRange[2];//!< Range in which the event should be
	 	 	 	  //! be executed by calling the functor given.

	EventCallback handler;//!< The functor to be called at execution time.

	union {
		void *eventObject;
		struct {
			unsigned long live	: 1;
			unsigned long weakPaused: 1;
			unsigned long reserved	: 2;
		};
	};

	Event(Timestamp trigger, Delay shiftAllowed,
			EventCallback handler, void *eventObject) {
		triggerRange[0] = trigger;
		triggerRange[1] = trigger + shiftAllowed;
		this->handler = handler;
		this->eventObject = eventObject;
		this->live = 1;
	}

	inline bool isLive() {
		return (live);
	}

	void execute() {
		handler(eventObject);
	}
};

}// namespace Timer
}// namespace Executable

#endif/* Executable/Timer/EventTrigger.hpp */
