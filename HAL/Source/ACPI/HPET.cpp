/**
 * @file HPET.cpp
 *
 * The HAL is responsible only for the initialization of HPET timers. It
 * will create HPET entries for all found timers.
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
#include <HAL/IOAPIC.hpp>
#include <Memory/KObjectManager.h>
#include <KERNEL.h>

#include <Executable/Timer/HPET.hpp>

using namespace HAL;

Executable::Timer::HPET *tm_;

unsigned int fd = 0;

#include <IA32/IO.h>
#include <Heap.hpp>
#include <Executable/Timer/PIT.hpp>
#include <Utils/ArrayList.hpp>

using namespace Executable::Timer;

PIT pit;

import_asm void EOI();
decl_c void hellow_hpet()
{
//	Dbg("hpet-fired or keybord clicked");

	if(pit.status(0).outputState)
	{
		//Dbg("pit");
		pit.resetTimer(0xFFFF, PIT::INTERRUPT_ON_TERMINAL_COUNT, 0);
	}

	EOI();

	//DbgLine("over");
}

ObjectInfo *h;

decl_c void testhpet()
{
	DbgLine("test-hpet !!!!"); DbgInt(sizeof(ArrayList));

//	ACPI::HPET *tb = (ACPI::HPET*) SearchACPITableByName("HPET", null);

//	h = KiCreateType("hpet", sizeof(Executable::Timer::HPET), 4, null, null);
//	tm_ = new(h) Executable::Timer::HPET(tb->baseAddress.addressValue);
//	tm_->enable();
//	tm_->debug_func_hpet_nfp();

	// try setting up io/apic

	IOAPIC *e = IOAPIC::getIterable();
	if(e && 1)// tm_->routingMap(0))
	{

		IOAPIC::RedirectionEntry f = e->getRedirEnt(
					2//HighestBitSet(tm_->routingMap(0))
			);

		f.delvMode = 0;
		f.vector = 0xDD;
		f.destMode = 0;
		f.mask = 0;
		f.destination = 0;
		f.triggerMode = 1;

		pit.resetTimer(0xFFFF, PIT::INTERRUPT_ON_TERMINAL_COUNT, 0);
		e->setRedirEnt(2, &f);
	}
	else
		DbgLine("something is wrong!!@!!");

	Dbg(" ");
	//char volatile *mem = (char volatile*) kcalloc(4096);
	//mem[4095 -11] = 12;
	//DbgInt((unsigned long) mem);

	Dbg("over");
}
