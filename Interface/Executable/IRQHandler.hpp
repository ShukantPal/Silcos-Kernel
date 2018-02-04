/* @file IRQHandler.hpp
 *
 * Multiple devices can assert interrupts on the same line, and therefore
 * vector. Software is responsible to figure out which device needs servicing,
 * and then handling that device. For this, each vector has a list of
 * irq-handlers, which are all invoked when the CPU is interrupted.
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
#ifndef IRQHANDLER_HPP_
#define IRQHANDLER_HPP_

namespace Executable
{
class IRQHandler
{
public:
	virtual bool isServiceRequired() = 0;
	virtual void handleDevice(unsigned int ioaInput) = 0;
protected:
	IRQHandler();
	~IRQHandler();
};
}

#endif/* Executable/IRQHandler.hpp */
