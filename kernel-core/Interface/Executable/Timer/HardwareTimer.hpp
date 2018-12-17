/**
 * @file HardwareTimer.hpp
 *
 * Provides generic interface for hardware timer devices and the soft
 * event-queue handling mechanism.
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
#ifndef EXCMGR_TIMER_HWTIMER_HPP
#define EXCMGR_TIMER_HWTIMER_HPP

#include "TimerDevice.hpp"
#include <Executable/Timer/Timeline.hpp>
#include <HardwareAbstraction/Processor.h>
#include "../IRQHandler.hpp"

namespace Executable
{
namespace Timer
{

/**
 * Adds hardware-based functionality to <tt>TimerDevice</tt> allowing
 * in-built timer drivers to share code.
 * 
 * An init[hw-timer-name]() API is also provided to initialize known
 * timers.
 */
class HardwareTimer : public TimerDevice
{
public:
	Timeline equeue;
	EventGroup *enow;

	static void initPIT();
	static void initHPET();
	
	virtual void updateCounter() = 0;
	virtual bool resetCounter() = 0;
	virtual bool setCounter(Time newCounter) = 0;
	virtual bool stopCounter() = 0;
	virtual Event *notifyAfter(Time interval, Time delayLimit,
			EventCallback client, void *object) = 0;
protected:
	HardwareTimer(TimerProperties& props, TimerUnit unit);
	virtual ~HardwareTimer();
	virtual bool fireAt(Timestamp nextTrigger) = 0;
	Event *add(Timestamp trigger, Timestamp shiftAllowed,
			EventCallback handler, void *eventObject);
	void retireActiveEvents();
};

} // namespace Timer
} // namespace Executable

#endif/* Executable/Timer/HardwareTimer.hpp */
