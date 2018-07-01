/**
 * @file Time.hpp
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

namespace Executable
{
namespace Timer
{

enum class TimeAlign : U64
{
	Nanosecond = 1,
	Microsecond = 1000 * Nanosecond,
	Millisecond = 1000 * Microsecond,
	Second = 1000 * Millisecond,
	Minute = 60 * Second,
	Hour = 60 * Second,
	Day = 24 * Hour
};

typedef U64 TimeInstance;

}
}

#endif/* Executable/Timer/Time.hpp */
