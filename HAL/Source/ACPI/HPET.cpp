///
/// @file HPET.cpp
/// @module HAL
///
/// THIS FILE IS THE "WORKING" FILE. NOTHING HERE IS FINALIZED. HERE
/// DEBUGGING OF EXISTING KERNEL FEATURES WAS GOING ON.
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

#include <IA32/IO.h>
#include <HardwareAbstraction/Processor.h>
#include <Heap.hpp>
#include <Executable/Timer/PIT.hpp>
#include <Utils/ArrayList.hpp>

using namespace Executable::Timer;

PIT *pit;
ObjectInfo *h;

ArrayList hl(24);

decl_c void do_action(Executable::IRQHandler *h)
{
	DbgInt((unsigned long)h ); Dbg(" ");
	h->intrAction();
}

decl_c void testhpet()
{
	DbgLine("test-hpet !!!!");

	pit = new PIT();

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

		pit->resetTimer(0xFFFF, PIT::INTERRUPT_ON_TERMINAL_COUNT, 0);
		GetIRQTableById(0)[190 - 32].addHandler(pit);
		e->setRedirEnt(2, &f);
	}
	else
	{
		extern unsigned long ctorsStart, ctorsEnd;
		DbgLine("something is wrong!!@!!");

		DbgInt((U32)&ctorsEnd -(U32)&ctorsStart);
	}
}
