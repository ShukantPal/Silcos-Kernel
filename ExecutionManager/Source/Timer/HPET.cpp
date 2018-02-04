/* @file: HPET.cpp
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
#include <Memory/KMemoryManager.h>
#include <Memory/Pager.h>

using namespace Executable;
using namespace Executable::Timer;

/* @constructor HPET::HPET
 *
 * Brings the resources of the HPET into system memory by mapping its registers
 * to a non-cacheable page.
 *
 * @param[in] eventBlock - base physical-address of HPET's event-block
 * @author Shukant Pal
 */
HPET::HPET(PADDRESS eventBlock)
{
	this->eventBlock = eventBlock;

	// We must ensure that the 1024KB event-block must not cover two pages
	// otherwise we'll have to allocate a 8KB block.

// page-size depends on the arch, but event-block remains 1KB only
#ifdef IA32
	if(eventBlock%4096 < 3072)
	{
		this->regBase = KiPagesAllocate(0, ZONE_KMODULE, ATOMIC) + eventBlock%4096;
		EnsureMapping(regBase, eventBlock, NULL, 0, KernelData | PageCacheDisable);
	}
	else
	{
		this->regBase = KiPagesAllocate(1, ZONE_KMODULE, ATOMIC) + eventBlock%4096;
		EnsureAllMappings(regBase, eventBlock, KB(8), NULL, KernelData | PageCacheDisable);
	}
#endif

	this->capAndId = (CapabilityAndID volatile *)(regBase + 0x00);
	this->cfg = (Configuration volatile *)(regBase + 0x10);
	this->intSts = (InterruptStatus volatile *)(regBase + 0x20);
	this->ctr = (MainCounter volatile *)(regBase + 0xF0);
}

Executable::Timer::HPET::~HPET()
{
}

/*
 * Sets the overall-enable bit of the HPET so that the main counter starts
 * incrementing montonically.
 *
 * @author Shukant Pal
 */
bool HPET::enable()
{
	cfg->overallEnable = 1;
	return (true);
}
