/**
 * @file PIT.h
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

PIT systemPIT;

bool PIT::intrAction()
{
	bool servReq = status(0).outputState;

	if(servReq)
	{
		armTimer(0xFFFF, 0);
		//Dbg("p ");
	}

	return (servReq);
}

/**
 * Arms the timer with the initial count in the mode that it decrements it
 * at a fixed frequency throughout time. An interrupt is generated when the
 * count value drops to zero, and the timer will have to be programmed again.
 *
 * @param initialCount - The initial count that should be put into the count
 * 				register. This value is decremented at the fixed frequency
 * 				of the PIT.
 * @param selectCounter - The counter that is to be re-programmed in the PIT.
 * @version 1.0
 * @since Silcos 3.05
 * @author Shukant Pal
 */
void PIT::armTimer(unsigned short initialCount, unsigned char selectCounter)
{
	Counter *oldTimer = progTimers + selectCounter;

	oldTimer->totalTicks += oldTimer->getPendingTime();
	oldTimer->initialCount = initialCount;
	oldTimer->mode = INTERRUPT_ON_TERMINAL_COUNT;
	oldTimer->lastReadCount = initialCount;

	flush(selectCounter);
}

/**
 * Resets the PIT by writing to the control word and then to the counter, so
 * that the next interrupt occurs at initialCount cycles later. Even if the
 * PIT isn't used as the system wall-timer, it must be reset regularly due to
 * its <foolish> inability to turn itself off.
 *
 * @param initialCount - no. of CLK pulses delay for next interrupt
 * @param operation - operating mode for the timer
 * @param counter - counter no. of the channel being programmed
 * @author Shukant Pal
 */
void PIT::resetTimer(unsigned short initialCount, unsigned char operation,
			unsigned char counter)
{
	ControlWord ctl;
	ctl.BCD = 0;
	ctl.mode = operation;
	ctl.rwAccess = ReadWrite::BothBytes;
	ctl.selectCounter = counter;
	WritePort(CMD_REG, ctl);

	WritePort(getPITChannelPort(counter), (unsigned char) initialCount);
	WritePort(getPITChannelPort(counter), (unsigned char)(initialCount >> 8));
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

	if(initialCount & 0xFF == 0) {
		resetCommand.rwAccess = MSB;
	} else if(initialCount & 0xFF == initialCount) {
		resetCommand.rwAccess = LSB;
	} else {
		resetCommand.rwAccess = BothBytes;
	}

	WritePort(getPITChannelPort(index), resetCommand);

	if(initialCount & 0xFF)
		WritePort(getPITChannelPort(index), initialCount);

	if(initialCount >> 8)
		WritePort(getPITChannelPort(index), initialCount);
}
