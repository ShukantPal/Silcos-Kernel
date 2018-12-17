/**
 * @file Time.hpp
 * 
 * Unifies the measurement of time and its units in the Silcos kernel.
 * -------------------------------------------------------------------
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See theAft
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 *
 * Copyright (C) 2017 - Shukant Pal
 */
#ifndef INTERFACE_EXECUTABLE_TIMER_TIME_HPP_
#define INTERFACE_EXECUTABLE_TIMER_TIME_HPP_

#include <TYPE.h>

namespace Executable
{
namespace Timer
{

typedef unsigned long TimerUnit;

/* Timer units */
#define TU_SEC		1000000000
#define TU_MILLISEC	1000000
#define TU_MICROSEC	1000
#define TU_TENANOSEC	10
#define TU_NANOSEC	1

static inline
Time TransformTime(Time t, TimerUnit oldUnit, TimerUnit newUnit) {
	return (t * (oldUnit / newUnit));
}

struct GenericTime
{
	Time value;
	TimerUnit unit;
	
	GenericTime() {
		value = 0;
		unit = TU_NANOSEC;
	}
	
	GenericTime(TimerUnit u) {
		value = 0;
		unit = u;
	}
	
	GenericTime(Time t, TimerUnit u) {
		value = t;
		unit = u;
	}
	
	Time valueFor(TimerUnit u) {
		return (TransformTime(value, unit, u));
	}
};

} // Time
} // Executable

#endif/* Executable/Timer/Time.hpp */
