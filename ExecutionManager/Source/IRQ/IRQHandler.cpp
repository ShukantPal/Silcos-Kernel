///
/// @file IRQHandler.cpp
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
#include <Executable/IRQHandler.hpp>
#include <HardwareAbstraction/Processor.h>
#include <Executable/Timer/PIT.hpp>

using namespace Executable;

IRQHandler::~IRQHandler()
{

}

IRQ::IRQ() : lineHdlrs(4)
{

}

IRQ::IRQ(IRQHandler *primHdlr) : lineHdlrs(4)
{
	this->lineHdlrs.add(primHdlr);
}

///
/// Fast method for firing an interrupt, without getting the id of the
/// irq-handler that executed successfully.
///
/// @return - if an IRQ handler executed succesfully
/// @version 1.0
/// @since Silcos 3.02
/// @author Shukant Pal
///
bool IRQ::intrAction()
{
	if(!lineHdlrs.count())
		return (false);

	ArrayList::Iterator hItr(lineHdlrs);

	do {
		if(((IRQHandler*) hItr.get())->intrAction())
			return (true);
	} while(hItr.fastNext() != null);

	return (false);
}

///
/// Invokes each interrupt handler using the IRQHandler.intrAction() callback
/// and returns the status of the interrupt
///
/// @param[out] hdlrId - holds the id of the handler for which the interrupt
/// 			occured. Handlers after this one do not get a chance to
/// 			execute as this handler recognized the origin of the
/// 			interrupt.
/// @return - if any handler executed successfully returning a TRUE status. If
/// 		false is returned, then hdlrId is not even filled and may have
/// 		junk. Thus, the caller should first check the returned boolean
/// 		state before reading hdlrId.
/// @version 1.0
/// @since Silcos 3.02
/// @author Shukant Pal
///
bool IRQ::fire(unsigned long &hdlrId)
{
	if(!lineHdlrs.count())
		return (false);

	ArrayList::Iterator hItr(lineHdlrs);
	bool completed = false;

	do
	{
		completed = ((IRQHandler*) hItr.get())->intrAction();

		if(completed)
		{
			hdlrId = hItr.index();
			return (true);
		}
	} while(hItr.fastNext());

	return (false);
}

///
/// Invokes only the handler given by its id. This id should be given while
/// registering the irq-handler.
///
/// @param id - handler-id for the call-back functor
/// @return - the returned status of the call-back functor; if an invalid id
/// 		was given, then false.
///
bool IRQ::fireOnly(unsigned long id)
{
	IRQHandler *callback = (IRQHandler*) lineHdlrs.get(id);

	if(callback)
		return (callback->intrAction());
	else
		return (false);
}

import_asm void EOI();

///
/// This is the generic CPU interrupt request handler.
///
/// @param[in] vector - the IRQ-no. in the local APIC
/// @version 1.0
/// @since Silcos 3.02
/// @author Shukant Pal
///
export_asm void HandleDeviceIRQ(unsigned long vector)
{
	(GetIRQTableById(PROCESSOR_ID) + (vector - 32))->intrAction();

	EOI();
}
