/**
 * @file HPET.cpp
 *
 * THIS FILE IS THE "WORKING" FILE. NOTHING HERE IS FINALIZED. HERE
 * DEBUGGING OF EXISTING KERNEL FEATURES WAS GOING ON.
 *
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
#include <Object.hpp>
#include <ACPI/RSDT.h>
#include <ACPI/HPET.h>
#include <Executable/IRQHandler.hpp>
#include <Memory/KObjectManager.h>
#include <KERNEL.h>

#include <Executable/Timer/HPET.hpp>
#include <HardwareAbstraction/IOAPIC.hpp>

using namespace HAL;

Executable::Timer::HPET *tm_;

unsigned int fd = 0;

#include <Module/Elf/ABI/Implementor.h>

#include <ACPI/RSDT.h>
#include <IA32/IO.h>
#include <HardwareAbstraction/Processor.h>
#include <Heap.hpp>
#include <Executable/Timer/PIT.hpp>
#include <Utils/ArrayList.hpp>

#include <Executable/Timer/EventQueue.hpp>
#include <Executable/Timer/EventNode.hpp>
#include <Executable/Timer/NodeSorter.hpp>

using namespace Executable;
using namespace Executable::Timer;


ArrayList hl(24);

decl_c void do_action(Executable::IRQHandler *h)
{
	DbgInt((unsigned long)h ); Dbg(" ");
	h->intrAction();
}

decl_c void InitKernelHPET()
{
//	ACPI::HPET *ahdt = (ACPI::HPET *) SearchACPITableByName("HPET", null);

//	HPET *kernelTimer = new HPET(ahdt->timerNumber,
//			ahdt->baseAddress.addressValue);

}

#include <Module/SymbolLookup.hpp>

using namespace Module;

void hdlr(void *nilObj)
{
	DbgLine("Tested timr");
}

decl_c void testhpet()
{
	DbgLine("test-hpet !!!!");

	// try setting up io/apic

	IOAPIC *e = IOAPIC::getIterable();
	if(e)// tm_->routingMap(0))
	{
		IOAPIC::RedirectionEntry f = e->getRedirEnt(
					2//HighestBitSet(tm_->routingMap(0))
			);

		f.delvMode = 0;
		f.vector = 190;
		f.destMode = 0;
		f.mask = 0;
		f.destination = 0;
		f.triggerMode = 1;

		pit.reset(0xFFFF, 0);
		GetIRQTableById(0)[190 - 32].addHandler(static_cast<IRQHandler*>(&pit));
		e->setRedirEnt(2, &f);
	}

	EventNode::init();
}
