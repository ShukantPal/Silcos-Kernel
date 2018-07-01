/**
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

#include <Executable/IRQHandler.hpp>
#include <IA32/IO.h>
#include <TYPE.h>

namespace Executable
{
namespace Timer
{

static unsigned short getPITChannelPort(unsigned long counter)
{
	return (0x40 + counter);
}

union ControlWord
{
	struct
	{
		U8 BCD			: 1;
		U8 mode			: 3;
		U8 rwAccess		: 2;
		U8 selectCounter: 2;
	};
	U8 dByte;

	ControlWord() {
		dByte = 0;
	}

	operator U8() const {
		return (dByte);
	}
};

union CounterLatchCommand
{
	struct
	{
		U8 noBits			: 4;
		U8 latchCommand		: 2;
		U8 selectCounter	: 2;
	};
	U8 dByte;

	CounterLatchCommand(U8 counter) {
		noBits = 0;
		latchCommand = 0;
		selectCounter = counter;
	}

	operator U8() const {
		return (dByte);
	}
};

union ReadBackCommand
{
	struct {
		U8 rfield			: 1;//!< Reserved, should be zero
		U8 selectCounters	: 3;//!< A bit-mask of counters that are selected
		U8 latchStatus		: 1;//!< 0 = Latch status of selected counter(s)
		U8 latchCount		: 1;//!< 0 = Latch count of selected counter(s)
		U8 cmdSign			: 2;//!< Command sign that should be 0b11
	};
	U8 dByte;

	ReadBackCommand(bool doStatus, bool doCount, U8 counterMask) {
		rfield = 0;
		selectCounters = counterMask;
		latchStatus = !doStatus;
		latchCount = !doCount;
		cmdSign = 0b11;
	}

	operator U8() const {
		return (dByte);
	}
};

union StatusByte
{
	struct {
		U8 BCD			: 1;
		U8 counterMode	: 3;
		U8 rwAccessMode	: 2;
		U8 nullCount	: 1;
		U8 outputState	: 1;
	};
	U8 status;

	StatusByte(U8 byteRead) {
		status = byteRead;
	}
};

/**
 * The programmable interval timer is used on older system to generate
 * interrupts at varying intervals. It has been replaced by newer technology
 * (i.e. HPET and TSC). Here we control only one PIT in the system.
 *
 * @author Shukant Pal
 */
class PIT final : public IRQHandler
{
public:
	enum {
		CHANNEL_0_DATA	= 0x40,
		CHANNEL_1_DATA	= 0x41,
		CHANNEL_2_DATA	= 0x42,
		CMD_REG		= 0x43
	};

	enum TimerMode {
		BINARY_MODE	= 0,
		BCD_MODE	= 1
	};

	enum OperatingMode {
		INTERRUPT_ON_TERMINAL_COUNT		= 0b000,
		HARDWARE_RETRIGGERABLE_ONE_SHOT	= 0b001,
		RATE_GENERATOR					= 0b010,
		SQUARE_WAVE_MODE				= 0b100,
		SOFTWARE_TRIGGERED_STROBE		= 0b100,
		HARDWARE_TRIGGERED_STROBE		= 0b101,
	};

	enum ReadWrite {
		COUNTER_LATCH_COMMAND	= 0b00,
		LSB						= 0b01,
		MSB						= 0b10,
		BothBytes				= 0b11
	};

	PIT() { // @suppress("Class members should be properly initialized")

	}

	inline StatusByte status(unsigned char ctr)	{
		WritePort(CMD_REG, ReadBackCommand(true, false, 1 << ctr));

		return (StatusByte(ReadPort(getPITChannelPort(ctr))));
	}

	bool intrAction();

	void armTimer(unsigned short initialCount, unsigned char selectCounter);

	void resetTimer(unsigned short initialCount, unsigned char operation,
				unsigned char counter);
private:
	struct Counter {
		int mode;
		unsigned short initialCount;
		unsigned short lastReadCount;
		unsigned int totalTicks;

		unsigned long getPendingTime()
		{
			switch(mode) {
			case INTERRUPT_ON_TERMINAL_COUNT:
				return (lastReadCount);
			default:
				return (0);// not implemented
			}
		}
	};

	Counter progTimers[3];
	U64 lastSoftwareCounter;

	unsigned long calculateJiffies(unsigned short totalCount) {
//		return (totalCount * (838095 / 1000000))
	}

	void flush(unsigned long index);

	unsigned short latch(unsigned long index) {
		WritePort(getPITChannelPort(index), CounterLatchCommand(index));

		unsigned short latchedCounter;
		latchedCounter = ReadPort(getPITChannelPort(index));
		latchedCounter |= ReadPort(getPITChannelPort(index));

		return (latchedCounter);
	}

	inline StatusByte readStatus(unsigned long index) {
		WritePort(getPITChannelPort(index),
				ReadBackCommand(true, false, (1 << index)));

		return (ReadPort(getPITChannelPort(index)));
	}

	void readAll();
	void refresh(unsigned int index);
};

}
}

#endif/* Executable/Timer/PIT.hpp */
