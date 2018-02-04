/**
 * @file WallTimer.hpp
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

/* @class WallTimer
 *
 * Abstracts the services that a wall-timer will provide to the system. It
 * provides a monotonically incrementing counter and an interrupt providing
 * service.
 *
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
