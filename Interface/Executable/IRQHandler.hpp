///
/// @file IOAPIC.hpp
///
/// Multiple devices can assert interrupts on the same line, and therefore
/// vector. Software is responsible to figure out which device needs servicing,
/// and then handling that device. For this, each vector has a list of
/// irq-handlers, which are all invoked when the CPU is interrupted.
///
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
#ifndef EXEMGR_IRQHANDLER_HPP__
#define EXEMGR_IRQHANDLER_HPP__

#include <Object.hpp>
#include <Utils/ArrayList.hpp>

namespace Executable
{
///
/// @class IRQHandler
///
/// Device-drivers which require control whenever an interrupt occurs to
/// service their hardware should inherit from this class. They can then
/// later register to an IRQ handler-queue.
///
/// But if a device uses multiple irq-lines, it would be advisable to
/// create a seperate "irq" class for that device and then register the
/// objects separately.
///
/// @version 1.0
/// @since Silcos 3.02
/// @author Shukant Pal
///
class IRQHandler : virtual public Object
{
public:
	virtual bool intrAction() = 0;
protected:
	IRQHandler(){}
	~IRQHandler(){}
};

}// namespace Executable

#endif/* Executable/IRQHandler.hpp */
