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
#include "HardwareTimer.hpp"
#include <IA32/IO.h>
#include <TYPE.h>

namespace Executable
{
namespace Timer
{

static inline unsigned short getPITChannelPort(unsigned long counter) {
	return (0x40 + counter);
}

static inline unsigned short getPITCommandPort() {
	return (0x43);
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
		U8 rfield			: 1;
		U8 selectCounters	: 3;
		U8 latchStatus		: 1;
		U8 latchCount		: 1;
		U8 cmdSign			: 2;
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

class PIT final : public IRQHandler, public HardwareTimer
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

	PIT();
	virtual ~PIT();

	inline StatusByte status(unsigned char ctr)	{
		WritePort(CMD_REG, ReadBackCommand(true, false, 1 << ctr));

		return (StatusByte(ReadPort(getPITChannelPort(ctr))));
	}

	bool intrAction();
	Event *notifyAfter(Timestamp npitTicks, Timestamp delayFeasible,
			EventCallback handler, void *eventObject);
	Event *notifyAgain(Timestamp npitTicks, Timestamp delayFeasible,
			EventCallback handler, void *eventObject);

	void reset(unsigned short newInitialCount, unsigned char selectCounter);
protected:
	bool fireAt(Timestamp pitTicks);
private:
	struct Counter {
		int mode;
		U16 initialCount;
		U16 lastReadCount;
		U64 totalTicks;

		unsigned long getPendingTime() {
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

	U16 getCounter(unsigned index) {
		return (latch(index));
	}

	U16 getLastReadCounter(unsigned index) {
		return (progTimers[index].lastReadCount);
	}

	U64 getTotalTicks(unsigned index) {
		U16 updatedCounter = getCounter(index);
		progTimers[index].totalTicks +=
				getLastReadCounter(index) - updatedCounter;
		setLastReadCounter(index, updatedCounter);
		return (progTimers[index].totalTicks);
	}

	Counter *getTimerBlock(unsigned index) {
		return (progTimers + index);
	}

	void setLastReadCounter(unsigned index, U16 value) {
		progTimers[index].lastReadCount = value;
	}

	unsigned short latch(unsigned long index) {
		WritePort(getPITCommandPort(), CounterLatchCommand(index));

		unsigned short latchedCounter;
		latchedCounter = ReadPort(getPITChannelPort(index));
		latchedCounter |= ReadPort(getPITChannelPort(index)) << 8;

		return (latchedCounter);
	}

	inline StatusByte readStatus(unsigned long index) {
		WritePort(getPITChannelPort(index),
				ReadBackCommand(true, false, (1 << index)));

		return (ReadPort(getPITChannelPort(index)));
	}

	void arm(unsigned selectCounter);
	void armAt(Timestamp nextFire);
	void readAll();
	void refresh(unsigned int index);
};

extern PIT pit;

}
}

#endif/* Executable/Timer/PIT.hpp */
