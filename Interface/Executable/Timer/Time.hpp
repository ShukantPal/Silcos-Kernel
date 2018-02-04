/**
 * File: Time.hpp
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
#ifndef INTERFACE_EXECUTABLE_TIMER_TIME_HPP_
#define INTERFACE_EXECUTABLE_TIMER_TIME_HPP_

namespace Executable
{
namespace Timer
{

enum TimeAlign
{
	Recent = 10,
	Moderate = 100,
	Second = 1000,
	Minute = 60000,
	Hour = 3600000
};

}
}

#endif/* Executable/Timer/Time.hpp */
