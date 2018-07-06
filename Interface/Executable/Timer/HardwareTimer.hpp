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

namespace Executable
{
namespace Timer
{

class HardwareTimer
{
public:
	EventQueue trigsLeft;
protected:
	HardwareTimer(){}
	void fireAll();
};

} // namespace Timer
} // namespace Executable

#endif/* Executable/Timer/HardwareTimer.hpp */
