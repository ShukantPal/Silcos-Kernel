/* @file PIT.hpp
 *
 * The legacy programmable interval timer is used in particularly old
 * systems. Software must know how to operate it, and if modern timers are
 * available as an alternative, how to turn it off. This is due to the fact
 * that the state of the PIT is undefined during startup.
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
#ifndef EXEMGR_TIMER_PIT_HPP__
#define EXEMGR_TIMER_PIT_HPP__

#include <IA32/IO.h>
#include <Executable/IRQHandler.hpp>
#include <TYPE.h>

namespace Executable
{
namespace Timer
{

#define CHANNEL_N_DATA(n) (0x40 + n)

/* @class PIT
 *
 * The programmable interval timer is used on older system to generate
 * interrupts at varying intervals. It has been replaced by newer technology
 * (i.e. HPET and TSC). Here we control only one PIT in the system.
 *
 * @author Shukant Pal
 */
class PIT final
{
public:
	enum
	{
		CHANNEL_0_DATA	= 0x40,
		CHANNEL_1_DATA	= 0x41,
		CHANNEL_2_DATA	= 0x42,
		CMD_REG		= 0x43
	};

	enum TimerMode
	{
		BINARY_MODE	= 0,
		BCD_MODE	= 1
	};

	enum OperatingMode
	{
		INTERRUPT_ON_TERMINAL_COUNT	= 0b000,
		HARDWARE_RETRIGGERABLE_ONE_SHOT	= 0b001,
		RATE_GENERATOR			= 0b010,
		SQUARE_WAVE_MODE		= 0b100,
		SOFTWARE_TRIGGERED_STROBE	= 0b100,
		HARDWARE_TRIGGERED_STROBE	= 0b101,
	};

	enum AccessMode
	{
		LATCH_COUNT_VALUE_CMD	= 0b00,
		LOW_BYTE_ONLY		= 0b01,
		HIGH_BYTE_ONLY		= 0b10,
		LOW_OR_HIGH_BYTE	= 0b11
	};

	union StatusByte
	{
		struct
		{
			U8 mode		: 1;
			U8 operation	: 3;
			U8 access	: 2;
			U8 nullCount	: 1;
			U8 outputState	: 1;
		};
		U8 status;
	};

	PIT()
	{

	}

	/*
	 * Uses the read-back command to check the current state of the given
	 * counter. Latching multiple counters is not supported in this
	 * method.
	 *
	 * @param ctr - counter no. whose status is to be tested, valid values
	 * 			are 0 to 2, inclusive
	 * @return latched status byte of the counter selected
	 * @author Shukant Pal
	 */
	inline StatusByte status(unsigned char ctr)
	{
		ReadBackCmd cmd;
		cmd.value = (1 << (ctr + 1));
		cmd.latchCount = 1;// turned off
		cmd.cmdSign = (1 << 0) | (1 << 1);
		WritePort(CMD_REG, cmd.value);

		StatusByte sts;
		sts.status = ReadPort(CHANNEL_N_DATA(ctr));
		return (sts);
	}

	bool intrAction();
	void resetTimer(unsigned short initialCount, unsigned char operation,
				unsigned char counter);
private:
	union ControlWord
	{
		struct
		{
			U8 mode		: 1;
			U8 operation	: 3;
			U8 access	: 2;
			U8 channel	: 2;
		};
		U8 format;
	};

	union ReadBackCmd
	{
		struct
		{
			U8 rfield	: 1;
			U8 ctr0		: 1;
			U8 ctr1		: 1;
			U8 ctr2		: 1;
			U8 latchStatus	: 1;
			U8 latchCount	: 1;
			U8 cmdSign	: 2;
		};
		U8 value;
	};
};

}
}


#endif /* PIT_HPP_ */
