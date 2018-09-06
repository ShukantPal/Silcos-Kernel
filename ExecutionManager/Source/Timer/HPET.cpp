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
	if(eventBlock % 4096 < 3072) {
		this->regBase = KiPagesAllocate(0, ZONE_KMODULE, ATOMIC) +
				eventBlock % 4096;
		Pager::map(regBase, eventBlock, 0,
				KernelData | PageCacheDisable);
	} else {
		this->regBase = KiPagesAllocate(1, ZONE_KMODULE, ATOMIC) +
				eventBlock % 4096;
		Pager::mapAll(regBase, eventBlock, KB(8), FLG_ATOMIC,
				KernelData | PageCacheDisable);
	}
#endif

	blockCapacity = (read32(CAP_AND_ID) >> NUM_TIM_CAP) & 0b11111;
	fsbTimers = 0;
	periodicTimers = 0;
	timerSizes = 0;
	usageTable = 0;

	this->sequenceIndex = acpiUID;
	this->mcPeriod = read32(CAP_AND_ID + 0x4);

	for(unsigned cidx = 0; cidx < blockCapacity; cidx++) {

		unsigned tnCfg = TIMER_N_CFG(cidx);

		if(read32(tnCfg, Tn_FSB_INT_DEL_CAP))
			fsbTimers |= (1 << cidx);

		if(read32(tnCfg, Tn_PER_INT_CAP))
			periodicTimers |= (1 << cidx);

		if(read32(tnCfg, Tn_SIZE_CAP))
			timerSizes |= (1 << cidx);

		HPET::Timer *timer_n = new HPET::Timer(this, cidx);
		timer_n->disable();
		timerTable.add(timer_n);
	}

	sysWallTimer = getTimer(0);
	knownHPETs.set(static_cast<void*>(this), sequenceIndex);

	U32 cfg = read32(CONFIG);
	cfg |= (1 << LEG_RT_CNF);
	cfg |= (1 << ENABLE_CNF);
	write32(cfg, CONFIG);
}

Executable::Timer::HPET::~HPET()
{
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

/**
 * Constructs a <tt>HPET::Timer</tt> object for the comparator installed
 * in the given HPET block; it is kept in an disabled state.
 *
 * @param owner - owner HPET block
 * @param index - timer index
 */
HPET::Timer::Timer(HPET *owner, unsigned int index) : HardwareTimer()
{
	this->enow = strong_null;
	this->owner = owner;
	this->index = index;
	this->lastReadCount = getTotalTicks();
}

HPET::Timer::~Timer()
{
	owner->usageTable &= ~(1 << index);
}

void HPET::Timer::connectTo(unsigned input)
{
	IOAPIC::inputAt(input)->addDev(this, false);
}

bool HPET::Timer::intrAction()
{
	if(!isInterruptActive()) {
		Dbg("_spu");
		DbgInt(getTotalTicks());
		DbgLine(";");

		return (false);
	}

	clearStatus();

	if(isInterruptActive())
		DbgLine("STILL ACTIVE");

	Dbg("hpet_ ");
	Timestamp fireMoment = getTotalTicks();

	DbgInt(enow[0].overlapRange[0]);
	Dbg(", but ");

	DbgInt(fireMoment);
	DbgLine(" _____ SUCCESS");

	this->lastReadCount = fireMoment;

	/* An important implication of the HPET sometimes losing the interrupts
	   is that it all previously missed events must also be handled. This is
	   done in this loop. */
	while(enow && enow->overlapRange[0] <= fireMoment) {
		retireActiveEvents();
	}

	if(enow != null) {
		Dbg("pending: "); DbgInt(equeue.nodalCount() + 1); DbgLine(" nodes");
		fireAt(enow->overlapRange[0]);
	} else {
		DbgLine("No active triggers");
		disable();
	}

	return (true);
}

Event *HPET::Timer::notifyAfter(Timestamp interval, Timestamp delayAllowed,
		EventCallback hdlr, void *arg)
{
	if(delayAllowed < 10) {
		return (strong_null);
	}

	Event *et;

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
		owner->write32(fts, TIMER_N_COMPARATOR(index));
	} else {
		owner->write32(fts, TIMER_N_COMPARATOR(index));
	}

	U32 cfg = owner->read32(TIMER_N_CFG(index));
	cfg |= (1 << Tn_32MODE_CNF);
	cfg &= ~(1 << Tn_TYPE_CNF);
	cfg |= (1 << Tn_INT_TYPE_CNF);
	cfg |= (1 << Tn_INT_ENB_CNF);
	owner->write32(cfg, TIMER_N_CFG(index));

	return (true);
}
