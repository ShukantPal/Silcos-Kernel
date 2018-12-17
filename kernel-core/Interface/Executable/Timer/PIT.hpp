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

/* PIT data & control port addresses */
#define CHANNEL_0_DATA	0x40
#define CHANNEL_1_DATA	0x41
#define CHANNEL_2_DATA	0x42
#define CMD_REG		0x43

static inline
unsigned short getPITChannelPort(unsigned long counter) {
	return (0x40 + counter);
}

/* PIT modes */
#define BINARY_MODE	0
#define BCD_MODE	1

/* PIT operating modes */
#define INT_ON_TERM_CNT		0b000
#define HW_RETRIG_ONESHOT	0b001
#define RATE_GEN		0b010
#define SQUAREWAVE_GEN		0b100
#define SW_TRIG_STROBE		0b100
#define HW_TRIG_STROBE		0b101

/* Access modes */
#define CNTR_LATCH_CMD		0b00
#define LSB_MODE		0b01
#define MSB_MODE		0b10
#define BOTH_MODE		0b11

#define LATCH_STATUS		0
#define LATCH_COUNT		0
#define NO_LATCH		1

/* Creates a CONTROL WORD */
#define CTRL_WORD(numMode, opMode, accessMode, counterSelect) \
((numMode) | ((opMode) << 0x01) | \
	((accessMode) << 0x04) | ((1 << counterSelect) << 0x06))

#define LATCH_CMD(selectCounter) ((selectCounter) << 0x06)

/* Creates a READBACK COMMAND */
#define READBACK_CMD(selectCounter, latchStatus, latchCount) \
(((1 << selectCounter) << 0x01) | (latchStatus << 0x04) | \
	(latchCount << 0x05) | (0b11 << 0x06))

#define IS_READBACK_CMD(readBackCommand) ((readBackCommand) >> 0x06 == 0b11)

/* Access fields in the STATUS BYTE */
#define BCD_MODE_(statusByte) ((statusByte) & 1)
#define OP_MODE(statusByte) ((statusByte) >> 0x01 && 0b111)
#define ACCESS_MODE(statusByte) ((statusByte) >> 0x04 & 0b11)
#define NULL_COUNT(statusByte) ((statusByte) >> 0x06 & 1)
#define OUT_PIN(statusByte) ((statusByte) >> 0x07 & 1)

/**
 * Performs a read-back command and then read the PIT's status for
 * the given channel/counter.
 * 
 * @param selectCounter - Counter whose status is returned
 */
static inline U8 readStatusByte(int selectCounter) {
	WritePort(CMD_REG, READBACK_CMD(selectCounter,
		LATCH_STATUS, NO_LATCH));
	
	return (ReadPort(getPITChannelPort(selectCounter)));
}

/**
 * Driver for the PIT.
 *
 * @author Shukant Pal
 */
class PIT final : public IRQHandler, public HardwareTimer
{
public:
	PIT();
	virtual ~PIT();

	void updateCounter();
	bool resetCounter();
	bool setCounter(Time newCounter);
	bool stopCounter();
	
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
			case INT_ON_TERM_CNT:
				return (lastReadCount);
			default:
				return (0);// not implemented
			}
		}
	};

	Counter progTimers[3];
	U64 lastSoftwareCounter;

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
		WritePort(CMD_REG, LATCH_CMD(index));

		unsigned short latchedCounter;
		latchedCounter = ReadPort(getPITChannelPort(index));
		latchedCounter |= ReadPort(getPITChannelPort(index)) << 8;

		return (latchedCounter);
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
