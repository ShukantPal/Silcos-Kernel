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

#include "EventQueue.hpp"
#include "../IRQHandler.hpp"

namespace Executable
{
namespace Timer
{

/**
 * System timers that can serve soft-timers should inherit
 * from this class. It doesn't provide any public interface
 * but setups required underlying functionality.
 *
 * The add() function here is not public - the device must
 * actually provide a function to fire a timer "n" ticks
 * later - and calculate the time stamp using its internal
 * counter.
 */
class HardwareTimer
{
public:
	EventQueue pendingTriggers;
	EventNode *activeTriggers;
protected:
	HardwareTimer();
	virtual ~HardwareTimer();
	virtual bool fireAt(Timestamp nextTrigger) = 0;
	EventTrigger *add(Timestamp trigger, Timestamp shiftAllowed,
			EventCallback handler, void *eventObject);
	void retireActiveEvents();
};

} // namespace Timer
} // namespace Executable

#endif/* Executable/Timer/HardwareTimer.hpp */
