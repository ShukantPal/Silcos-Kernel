/**
 * @file WallTimer.hpp
 *
 * THIS FILE IS THE "WORKING" FILE. NOTHING HERE IS FINALIZED. HERE
 * DEBUGGING OF EXISTING KERNEL FEATURES WAS GOING ON.
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
#ifndef EXEMGR_TIMER_WALLTIMER_HPP__
#define EXEMGR_TIMER_WALLTIMER_HPP__

#include <TYPE.h>

/**
 * Any hardware time that can support multi-cpu synchronized services can
 * inherit WallTimer. The HAL must choose which WallTimer to use as the
 * default timer for the system.
 *
 * The following order of preference is there -
 * 'i) synchronized TSC'
 * 'ii) high-precision event timer'
 * 'iii) programmable-interval-timer'
 * 'iv) ACPI wall timer'
 * 'v) System Failure'
 *
 * @version 1.0
 * @since Silcos 3.02
 * @author Shukant Pal
 */
class WallTimer
{
public:
	U64 getCounter()
	{
		return 0;
	}
protected:
	WallTimer();
	~WallTimer();
	U64 volatile *counter;
};

#endif /* INTERFACE_EXECUTABLE_TIMER_WALLTIMER_HPP_ */
