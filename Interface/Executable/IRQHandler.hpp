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

///
/// Carries all interrupt-handlers specific to one "IRQ" line. One can fire
/// an interrupt in this IRQ object, but that should be only done by the asm
/// interrupt-handler registered in the IDT.
///
/// Child classes may add and remove handlers by directly accessing the
/// lineHdlrs `ArrayList`.
///
/// Note that a IRQ is itself an IRQHandler - local irq (on LAPICs) triggers
/// and may have IOAPIC handlers too. But those device-interrupt handlers are
/// specific to IOAPIC so the IOAPIC irq is will behave as an irq-handler and
/// execute the device irqs. (ToDo: clear this paragraph)
///
/// @version 1.0
/// @since Silcos 3.02
/// @author Shukant Pal
///
class IRQ : public IRQHandler
{
public:
	virtual bool intrAction();
protected:
	IRQ();
	IRQ(IRQHandler *primHdlr);
	bool fire(unsigned long& hdlrId);
	bool fireOnly(unsigned long hdlrId);

	ArrayList lineHdlrs;//!< Array of irq-handler for this line
	friend struct Processor;
};


}// namespace Executable

#endif/* Executable/IRQHandler.hpp */
