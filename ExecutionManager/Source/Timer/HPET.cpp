/**
 * @file HPET.cpp
 *
 * Implements the HPET driver, allowing high-precision event
 * timing, in order of tens of microseconds (and possible even more
 * precision of hardware supports). This implementation assumes
 * that the HPET is used <b>only on IA-PC systems</b>.
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
#include <Executable/Timer/HPET.hpp>
#include <HardwareAbstraction/IOAPIC.hpp>
#include <Memory/KMemoryManager.h>
#include <Memory/Pager.h>

using namespace HAL;
using namespace Executable;
using namespace Executable::Timer;

HPET::Timer *HPET::sysWallTimer = null;
HPET *HPET::kernelTimer = null;

ArrayList HPET::knownHPETs;

/**
 * Builds a new HPET timer object whose chipset is mapped at the given
 * physical address. The ACPI-UID is kept cached with this driver.
 */
HPET::HPET(int acpiUID, PhysAddr eventBlock) : timerTable(3)
{
	this->eventBlock = eventBlock;

	// We must ensure that the 1024KB event-block must not cover two pages
	// otherwise we'll have to allocate a 8KB block.

// page-size depends on the arch, but event-block remains 1KB only
#ifdef IA32
	if(eventBlock%4096 < 3072)
	{
		this->regBase = KiPagesAllocate(0, ZONE_KMODULE, ATOMIC) +
				eventBlock % 4096;
		Pager::map(regBase, eventBlock, 0,
				KernelData | PageCacheDisable);
	}
	else
	{
		this->regBase = KiPagesAllocate(1, ZONE_KMODULE, ATOMIC) +
				eventBlock % 4096;
		Pager::mapAll(regBase, eventBlock, KB(8), FLG_ATOMIC,
				KernelData | PageCacheDisable);
	}
#endif

	this->capAndId = (CapabilityAndID volatile *)(regBase + 0x00);
	this->cfg = (Configuration volatile *)(regBase + 0x10);
	this->intSts = (InterruptStatus volatile *)(regBase + 0x20);
	this->ctr = (MainCounter volatile *)(regBase + 0xF0);

	blockCapacity = capAndId->timerCount;
	fsbTimers = 0;/* We'll set this later on! */
	periodicTimers = 0;/* We'll set this later on! */
	timerSizes = 0;/* We'll set this later on! */
	usageTable = 0;/* All comparators are free right now! */

	this->sequenceIndex = acpiUID;
	this->mcPeriod = capAndId->clockPeriod;

	ComparatorBlock volatile *cmp;
	for(unsigned cidx = 0; cidx < blockCapacity; cidx++) {
		cmp = comparatorBlock(cidx);

		if(cmp->cfgAndCap.fsbIntrAllowed)
			fsbTimers |= (1 << cidx);

		if(cmp->cfgAndCap.periodicAllowed)
			periodicTimers |= (1 << cidx);

		if(cmp->cfgAndCap.timerSize == 1)
			timerSizes |= (1 << cidx);

		HPET::Timer *timer_n = new HPET::Timer(this, cmp, cidx);
		timer_n->disable();
		timerTable.add(timer_n);
	}

	sysWallTimer = getTimer(0);
	knownHPETs.set(static_cast<void*>(this), sequenceIndex);

	cfg->legacyReplacement = 1;
	enable();
}

Executable::Timer::HPET::~HPET()
{
}

HPET::Timer::Timer(HPET *owner, ComparatorBlock volatile *block,
		unsigned int index) : HardwareTimer()
{
	this->activeTriggers = strong_null;
	this->owner = owner;
	this->block = block;
	this->index = index;
	this->lastReadCount = getTotalTicks();
}

HPET::Timer::~Timer()
{
	owner->usageTable &= ~(1 << index);
}

void HPET::Timer::connectTo(unsigned input)
{
	disable();
	block->cfgAndCap.intrType =1;
	block->cfgAndCap.intrRoute = input;

	IOAPIC::inputAt(input)->addDev(this, false);
}

bool HPET::enable()
{
	cfg->overallEnable = 1;
	return (true);
}



bool HPET::intrAction()
{
	DbgLine("ihpet; ");
	return (true);
}

bool HPET::Timer::intrAction()
{
	if(!isInterruptActive()) {
		Dbg("_spu");
		DbgInt(getTotalTicks());
		DbgLine(";");

		return (false);
	}

	clearInterruptActive();

	Dbg("hpet_");
	Timestamp fireMoment = getTotalTicks();

	DbgInt(fireMoment);
	DbgLine(" _____ SUCCESS");

	this->lastReadCount = fireMoment;

	/* An important implication of the HPET sometimes losing the interrupts
	   is that it all previously missed events must also be handled. This is
	   done in this loop. */
	while(activeTriggers && activeTriggers->overlapRange[0] <= fireMoment) {
		retireActiveEvents();
	}

	if(activeTriggers != null) {
		fireAt(activeTriggers->overlapRange[0]);
	} else {
		DbgLine("No active triggers");
		disable();
	}

	return (true);
}

EventTrigger *HPET::Timer::notifyAfter(Timestamp interval, Timestamp delayAllowed,
		EventCallback hdlr, void *arg)
{
	if(delayAllowed < 10) {
		return (strong_null);
	}

	EventTrigger *et;
	block->cfgAndCap.mode32 = 0;

	int tt = getTotalTicks();
	__cli
	et = add(interval + tt, delayAllowed, hdlr, arg);
	__sti

	return (et);
}

bool HPET::Timer::fireAt(Timestamp fts)
{
	Timestamp ttp = getTotalTicks();

	if(fts < ttp + 2) {
		DbgLine("here");
		return (false);
	} else if(isWide()) {
		owner->comparator(index)->val32 = fts;
	} else {
		owner->comparator(index)->val32 = fts;
	}

	block->cfgAndCap.intrType = 1;
	enable();

	return (true);
}

/**
 * Allows the caller to access the timer whose comparator is at the given
 * index. If that timer has already been allocated, <code>null</code>
 * is returned.
 *
 * @param tidx - The index of the timer required in this HPET.
 * @return - An driver object that can be used to operate the given timer.
 */
HPET::Timer *HPET::getTimer(unsigned int tidx)
{
		return (HPET::Timer*)(timerTable.get(tidx));
}
