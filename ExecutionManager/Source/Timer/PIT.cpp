/**
 * @file PIT.cpp
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
#include <Executable/Timer/PIT.hpp>
#include <Math.hpp>

#include <Module/Elf/ABI/Implementor.h>

using namespace Executable::Timer;

PIT Executable::Timer::pit;

bool PIT::intrAction()
{
	bool wasFired = status(0).outputState;
	if(!wasFired)
		return (false);



	return (true);
}

void PIT::reset(unsigned short newInitialCount, unsigned char selectCounter)
{
	Counter *newTimer = progTimers + selectCounter;
	newTimer->totalTicks = 0;
	newTimer->initialCount = newInitialCount;
	newTimer->mode = INTERRUPT_ON_TERMINAL_COUNT;
	newTimer->lastReadCount = newInitialCount;
	flush(selectCounter);
}

/**
 * Arms the given timer again so that it will start decrementing from the
 * previously used initial count. This is generally done when the PIT sends
 * an interrupt to a CPU - and it needs to be armed again.
 *
 * @param selectCounter - The counter that is to be re-programmed in the PIT.
 * @version 1.0
 * @since Silcos 3.05
 * @author Shukant Pal
 */
void PIT::arm(unsigned selectCounter)
{
	Counter *oldTimer = progTimers + selectCounter;

	oldTimer->totalTicks += oldTimer->lastReadCount;
	oldTimer->mode = INTERRUPT_ON_TERMINAL_COUNT;
	oldTimer->lastReadCount = oldTimer->initialCount;

	flush(selectCounter);
}

/**
 * Flushes the contents of the counter's programming struct in memory so
 * that it is implemented by the hardware.
 *
 * @param index - The index of the counter to be programmed.
 */
void PIT::flush(unsigned long index)
{
	ControlWord resetCommand;
	resetCommand.mode = progTimers[index].mode;
	resetCommand.selectCounter = index;

	unsigned short initialCount = progTimers[index].initialCount;

	resetCommand.rwAccess = BothBytes;
	WritePort(CMD_REG, resetCommand);
	WritePort(getPITChannelPort(index), initialCount);
	WritePort(getPITChannelPort(index), initialCount >> 8);
}
