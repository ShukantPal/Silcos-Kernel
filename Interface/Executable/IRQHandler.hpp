///
/// @file IrqHandler.hpp
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

#include <Atomic.hpp>
#include <Object.hpp>
#include <Utils/ArrayList.hpp>

namespace Executable
{
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
class IRQHandler
{
public:
	virtual bool intrAction() = 0;
protected:
	IRQHandler(){}
	virtual ~IRQHandler();
};

/**
 * Holds different irq-handlers that shared the same hardware line
 * physically. Whenever an irq is triggered, each handler is called
 * until the source is found.
 */
class IRQ : public IRQHandler
{
public:
	virtual bool intrAction();
protected:
	IRQ();
	IRQ(IRQHandler *primHdlr);
	bool fire(unsigned long& hdlrId);
	bool fireOnly(unsigned long hdlrId);

	ArrayList lineHdlrs;
	friend struct Processor;
};


}// namespace Executable

#endif/* Executable/IRQHandler.hpp */
