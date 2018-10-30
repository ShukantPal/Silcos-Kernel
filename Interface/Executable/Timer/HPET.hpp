/**
 * @file HPET.h
 *
 * Provides an interface to use the HPET as a wall-timer for the system. The
 * HPET can provide up to 32 comparators and send interrupts at required
 * intervals.
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

#ifndef EXEMGR_TIMER_HPET_H__
#define EXEMGR_TIMER_HPET_H__

#include <Atomic.hpp>
#include "HardwareTimer.hpp"
#include "WallTimer.hpp"
#include "../IRQHandler.hpp"
#include <Memory/KMemorySpace.h>
#include <Synch/Spinlock.h>
#include <TYPE.h>
#include <Utils/AVLTree.hpp>
#include <Math.hpp>

namespace Executable
{
namespace Timer
{

#define CAP_AND_ID		0x00
#define REV_ID			0
#define NUM_TIM_CAP		8
#define COUNT_SIZE_CAP		13
#define LEG_RT_CAP		15
#define VENDOR_ID		16
#define COUNTER_CLK_PERIOD	32

#define CONFIG			0x10
#define ENABLE_CNF		0
#define LEG_RT_CNF		1

#define INT_STS			0x20
#define Tn_INT_STS(n)		n

#define MAIN_COUNT		0xF0

#define TIMER_N_CFG(n) (n * 0x20 + 0x100)
#define TIMER_N_COMPARATOR(n) (n * 0x20 + 0x108)

#define Tn_INT_TYPE_CNF		1
#define Tn_INT_ENB_CNF		2
#define Tn_TYPE_CNF		3
#define Tn_PER_INT_CAP		4
#define Tn_SIZE_CAP		5
#define Tn_VAL_SET_CNF		6
#define Tn_32MODE_CNF		8
#define Tn_INT_ROUTE_CNF	9
#define Tn_FSB_EN_CNF		14
#define Tn_FSB_INT_DEL_CAP	15
#define Tn_INT_ROUTE_CAP	32

/**
 * Driver for HPET hardware, exposing individual comparators by
 * allocating <tt>HPET::Timer</tt> objects. IA32 & IA64 kernels both
 * use 32-bit modes when possible.
 *
 * @author Shukant Pal
 * @see IA-PC HPET Specifications 1.0A
 */
class HPET final : public LinkedListNode, public Lockable
{
public:
	class Timer;

	static HPET *kernelTimer;

	HPET(int acpiUID, PhysAddr eventBlock);
	~HPET();

	inline unsigned clockPeriod() {
		return (mcPeriod);
	}

	inline unsigned index() {
		return (sequenceIndex);
	}

	inline unsigned timerCount() {
		return (blockCapacity);
	}

	inline Timestamp mainCounter() {
#ifdef IA32
		return ((Timestamp) read32(MAIN_COUNT));
#else
		return ((Timestamp) read64(MAIN_COUNT));
#endif
	}

	HPET::Timer *getTimerObject(bool periodic);

	static HPET::Timer *wallTimer() {
		return (sysWallTimer);
	}

private:
	unsigned int blockCapacity;/* Number of timers in this HPET block */
	unsigned int fsbTimers;/* Bit-fielding for fsb-support data */
	unsigned int periodicTimers;/* Bit-field for periodic-mode support data */
	unsigned int timerSizes;/* Bit-fielding containing HPET timer widths */
	unsigned int usageTable;/* Bit-field containing used/free comparators */
	U32 enabledComparators;/*  Bit-field containing enabled timers */

	ArrayList timerTable;/* Table of Timer objects to expose */

	PhysAddr eventBlock;
	int sequenceIndex;
	long mcPeriod;
	unsigned long regBase;

	inline U32 read32(unsigned hpetRegister) {
		return (*(U32 volatile *)(regBase + hpetRegister));
	}

	inline U32 read32(unsigned hpetRegister, unsigned bit) {
		return ((read32(hpetRegister) >> bit) & 1);
	}

	inline U64 read64(unsigned hpetRegister) {
#ifdef IA32
		U64 hpetValue = 0;
		hpetValue |= read32(hpetRegister);
		hpetValue |= ((U64) read32(hpetRegister + 0x04) << 32);
		return (hpetValue);
#else
		return (*(U64 volatile *)(regBase + hpetRegister));
#endif
	}

	inline void write32(U32 value, unsigned targetRegister) {
		*(U32 volatile *)(regBase + targetRegister) = value;
	}

	inline void write64(U64 value, unsigned targetRegister) {
#ifdef IA32
		*(U32 volatile *)(regBase + targetRegister) = (U32) value;
		*(U32 volatile *)(regBase + targetRegister + 0x04) = (U32)(value >> 32);
#else
		*(U64 volatile *)(regBase + targetRegister) = value;
#endif
	}

	HPET::Timer *getTimer(unsigned int tidx);

	static HPET::Timer *sysWallTimer;
	static ArrayList knownHPETs;

	friend Executable::Timer::HPET::Timer;
};

/**
 * Each comparator present in the HPET can be used as an constrained
 * independent timer-device.
 */
class HPET::Timer final : public IRQHandler, public HardwareTimer
{
public:
	inline bool isFSBCap() {
		return (Math::bitTest(owner->fsbTimers, comparatorIndex));
	}

	bool hasPeriodicSupport() {
		return (Math::bitTest(owner->periodicTimers, comparatorIndex));
	}

	bool isWide() {
		return (Math::bitTest(owner->timerSizes, comparatorIndex));
	}

	Timestamp getTotalTicks() {
		return ((owner->mainCounter() * owner->clockPeriod()) / 10000000);
	}

	inline U32 allRoutes() {
		return (owner->read32(TIMER_N_CFG(comparatorIndex) + 0x4));
	}

	~Timer();

	void updateCounter();	
	bool resetCounter();
	bool setCounter(Time newCounter);
	bool stopCounter();
	
	bool intrAction();
	Event *notifyAfter(Time interval, Time delayAllowed,
			EventCallback handler, void *eventObject);
protected:
	bool fireAt(Timestamp fts);
private:
	HPET *owner;
	unsigned int comparatorIndex;
	Timestamp lastReadCount;

	void enableComparator();
	void disableComparator();

#ifdef IA64

	inline void setComparator(U64 val64) {
		block->cmp.val64 = val64;
	}

#endif

	Timer(HPET *owner, unsigned int index);
public:
	void routeInterrupts();
	void connectTo(unsigned input);
	void connectToDefault();

	friend HPET;
};

}
}

#endif/* Executable/Timer/HPET.h */
