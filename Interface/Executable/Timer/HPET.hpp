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

struct CapabilityAndID
{
	U64 revisionID		: 8;/* Hardware revision ID */
	U64 timerCount		: 5;/* Number of comparators in this block */
	U64 cntSizeCap		: 1;/* Size (32/64-bit) of main-counter */
	U64 reserved00		: 1;
	U64 legacyRouting	: 1;/* Bit telling whether interrupts can be
	 	 	 	 	 	 	   routed through the PIC */
	U64 vendorID		: 16;/* Vendor ID, as if PCI device */
	U64 clockPeriod		: 32;/* Period of main-counter incrementing */
};

struct Configuration
{
	U64 overallEnable	: 1;/* Enables the main-counter & comparators */
	U64 legacyReplacement: 1;/* Allows interrupts to be sent by PIC */
	U64 reservedField0	: 62;
};

struct InterruptStatus
{
	U32 timerSts;/* Bit-field containing whether a timer is delivering
	 	 	 	 	an interrupt. */
	U32 reservedField0;
};

union MainCounter
{
	struct {
		U32 l32;
		U32 u32;
	};
	U64 value;
};

struct ComparatorBlock
{
	/* @union HPET::Timer::ConfigAndCapab
	 *
	 * Contains configuration & capability data for a timer. Reads
	 * and writes to this register should not be done directly. To
	 * read or write this register, store a ConfigAndCapab struct
	 * on the stack. Then equate the value of its val to the val
	 * of cfgAndCap(n).
	 *
	 * @author Shukant Pal
	 * @see IA-PC HPET Specifications 1.0A
	 */
	union ConfigAndCapab
	{
		struct
		{
			U32 reservedField0	: 1;
			U32 intrType		: 1;/* Edge- or level- triggered interrupts */
			U32 intrEnable		: 1;/* Bit enables/disables interrupts */
			U32 timerType		: 1;
			U32 periodicAllowed	: 1;/* Whether timer supports periodic mode */
			U32 timerSize		: 1;/* Width of timer (32/64-bit) */
			U32 valueSet		: 1;
			U32 reservedField1	: 1;
			U32 mode32		: 1;
			U32 intrRoute		: 5;/* IO/APIC redirection entry */
			U32 fsbIntrEnable	: 1;/* Enables/disables FSB interrupt mode */
			U32 fsbIntrAllowed	: 1;/* Whether FSB routing is supported */
			U32 reservedField2	: 16;
			U32 intrRoutingCapable;/* Bit-field containing valid IO/APIC
			 	 	 	 	 	 	  re-direction entries */
		};
		struct
		{
			U32 lower32;
			U32 upper32;
		};
		U64 val;
	} __attribute__((packed)) cfgAndCap;

	union Comparator
	{
		U32 val32;
		U64 val64;
	} cmp;

	struct FSBInterruptRoute
	{
		U32 intrValue;  // written in FSB-intr message
		U32 intrAddress;/* Address of the FSB-interrupt message */
	} __attribute__((packed)) fsb;
};

/**
 * The HPET device can be used through this driver class - which also
 * connects hardware functionality with the HardwareTimer tree. Events of
 * microsecond precision can be fired through this driver.
 *
 * Internally, the HPET maps the register structs above this definition,
 * into physical address space.
 *
 * This driver reveals the following features of HPET devices: no. of
 * comparator blocks present (@code timerCount()), the current main counter
 * (@code mainCounter()), ACPI UID (@code index()), frequency of the HPET
 * main counter (@code frequency()), etc.
 *
 * @author Shukant Pal
 * @see IA-PC HPET Specifications 1.0A
 */
class HPET final : public IRQHandler, public LinkedListNode, public Lockable
{
public:
	class Timer;

	/**
	 * One HPET is reserved during the boot-process and is solely for use by
	 * the kernel environment. It may be used through syscalls but user-mode
	 * applications are not given direct access.
	 */
	static HPET *kernelTimer;

	HPET(int acpiUID, PhysAddr eventBlock);
	~HPET();

	void clearIntrStatus(unsigned char n)
	{ Atomic::oR((unsigned int*) intSts, (1 << n)); }

	U32 routingMap(unsigned char n)
	{
		return (cfgAndCap(n)->upper32);
	}

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
		return ((Timestamp) ctr->value);
	}

	bool disableLegacyRouting();
	bool disable();
	bool enable();
	bool enableLegacyRouting();
	bool intrAction();
	void setMainCounter(unsigned long val);

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
	ArrayList timerTable;/* Table of Timer objects to expose */

	PhysAddr eventBlock;
	int sequenceIndex;
	long mcPeriod;
	unsigned long regBase;

	inline ComparatorBlock volatile *comparatorBlock(unsigned n) {
		return ((ComparatorBlock volatile *)
				(regBase + 0x100 + 0x20 * n));
	}

	inline ComparatorBlock::ConfigAndCapab volatile *cfgAndCap(unsigned n) {
		return ((ComparatorBlock::ConfigAndCapab volatile*)
						(regBase + 0x100 + 0x20 * n));
	}

	inline ComparatorBlock::Comparator volatile *comparator(unsigned n) {
		return ((ComparatorBlock::Comparator volatile*)
						(regBase + 0x108 + 0x20 * n));
	}

	inline ComparatorBlock::FSBInterruptRoute volatile *fsbIntrRoute(unsigned n) {
		return ((ComparatorBlock::FSBInterruptRoute volatile*)
						(regBase + 0x110 + 0x20 * n));
	}

	CapabilityAndID volatile *capAndId;
	Configuration volatile *cfg;
	InterruptStatus volatile *intSts;
	MainCounter volatile *ctr;

	HPET::Timer *getTimer(unsigned int tidx);

	static HPET::Timer *sysWallTimer;
	static ArrayList knownHPETs;

	friend Executable::Timer::HPET::Timer;
};

class HPET::Timer final : public IRQHandler, public HardwareTimer
{
public:
	inline bool isFSBCap() {
		return (Math::bitTest(owner->fsbTimers, index));
	}

	bool hasPeriodicSupport() {
		return (Math::bitTest(owner->periodicTimers, index));
	}

	bool isWide() {
		return (Math::bitTest(owner->timerSizes, index));
	}

	bool isEdgeTriggered() {
		return (block->cfgAndCap.intrType == 0);
	}

	bool isLevelTriggered() {
		return (block->cfgAndCap.intrType == 1);
	}

	bool isEnabled() {
		return (block->cfgAndCap.intrEnable);
	}

	bool isInterruptActive() {
		return ((owner->intSts->timerSts >> index) & 1);
	}

	void clearInterruptActive() {
		owner->intSts->timerSts |= (1 << index);
	}

	Timestamp getTotalTicks() {
		return ((owner->mainCounter() * owner->clockPeriod()) / 10000000);
	}

	inline void enable() {
		block->cfgAndCap.intrEnable = 1;
	}

	inline void disable() {
		block->cfgAndCap.intrEnable = 0;
	}

	inline U32 allRoutes() {
		return (block->cfgAndCap.intrRoutingCapable);
	}

	~Timer();

	bool intrAction();
	EventTrigger *notifyAfter(Timestamp interval, Timestamp delayAllowed,
			EventCallback handler, void *eventObject);
protected:
	bool fireAt(Timestamp fts);
private:
	HPET *owner;
	ComparatorBlock volatile *block;
	unsigned int index;
	Timestamp lastReadCount;

	inline void setComparator(U32 val32) {
		block->cmp.val32 = val32;
	}

#ifdef IA64

	inline void setComparator(U64 val64) {
		block->cmp.val64 = val64;
	}

#endif

	Timer(HPET *owner, ComparatorBlock volatile *block,
			unsigned int index);
public:
	void routeInterrupts();
	void connectTo(unsigned input);

	friend HPET;
};

}
}

#endif/* Executable/Timer/HPET.h */
