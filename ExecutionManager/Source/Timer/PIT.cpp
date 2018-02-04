/* @file PIT.cpp
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

using namespace Executable::Timer;

/*
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
	ctl.mode = 0;
	ctl.operation = operation;
	ctl.access = LOW_OR_HIGH_BYTE;
	ctl.channel = counter;
	WritePort(CMD_REG, ctl.format);

	WritePort(CHANNEL_N_DATA(counter), (unsigned char) initialCount);
	WritePort(CHANNEL_N_DATA(counter), (unsigned char)(initialCount >> 8));
}
