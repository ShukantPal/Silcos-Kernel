///
/// @file HPET.h
///
/// Provides an interface to use the HPET as a wall-timer for the system. The
/// HPET can provide upto 32 comparators and send interrupts at required
/// intervals.
/// -------------------------------------------------------------------
/// This program is free software: you can redistribute it and/or modify
/// it under the terms of the GNU General Public License as published by
/// the Free Software Foundation, either version 3 of the License, or
/// (at your option) any later version.
///
/// This program is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
/// GNU General Public License for more details.
///
/// You should have received a copy of the GNU General Public License
/// along with this program.  If not, see <http://www.gnu.org/licenses/>
///
/// Copyright (C) 2017 - Shukant Pal
///

#ifndef EXEMGR_TIMER_HPET_H__
#define EXEMGR_TIMER_HPET_H__

#include <Atomic.hpp>
#include "WallTimer.hpp"
#include "../IRQHandler.hpp"
#include <Memory/KMemorySpace.h>
#include <Synch/Spinlock.h>
#include <TYPE.h>
#include <Utils/AVLTree.hpp>

namespace Executable
{
namespace Timer
{

/* @class HPET
 *
 * Defines a set of "timers" that can be used by the kernel, or given to
 * user-mode software. Each timer is capable of being configured to deliver
 * interrupts separately. The HPET allows upto 32-timer in one event-block.
 *
 * It also provides a main-counter which increases monotonically. Each
 * timer depends on the main-counter to keep track of time. Each timer can
 * be set to fire interrupts when the main-counter reaches a specific value.
 *
 * @author Shukant Pal
 * @see IA-PC HPET Specifications 1.0A
 */
class HPET : public IRQHandler, public LinkedListNode, public Lockable
{
public:
	HPET(int acpiUID, PhysAddr eventBlock);
	~HPET();

	void clearIntrStatus(unsigned char n)
	{ Atomic::oR((unsigned int*) intSts, (1 << n)); }

	unsigned long mainCounter(){ return ((unsigned long)(ctr->value)); }

	U32 routingMap(unsigned char n)
	{
		return (cfgAndCap(n)->upper32);
	}

	void debug_func_hpet_nfp()// not for production (just testing)
	{

		Timer::ConfigAndCapab c;
		c.val = cfgAndCap(0)->val;

		c.intrRoute = 4;
		c.intrEnable = 1;
		c.intrType = 1;
		c.fsbIntrEnable = 0;
		c.mode32 = 1;
		c.timerType = 0;

		cfgAndCap(0)->val = c.val;
		comparator(0)->comparatorValue = mainCounter() + 100000000;

		c.val = cfgAndCap(0)->val;
		Dbg("intrRoute:" ); DbgInt(c.intrRoutingCapable);
	}

	bool disableLegacyRouting();
	bool disable();
	bool enable();
	bool enableLegacyRouting();
	bool intrAction();
	void setMainCounter(unsigned long val);
private:
	PhysAddr eventBlock;
	int sequenceIndex;
	long clockFrequency;
	unsigned long regBase;

	///
	/// Read-only register which provides general info on the hardware
	/// capabilities of the HPET and its vendor ID. It is located at
	/// offset 0x00 in the event block.
	///
	/// @author Shukant Pal
	///
	struct CapabilityAndID
	{
		U64 revisionID		: 8;//< which revision is implemented
		U64 timerCount		: 4;//< no. of timers in this block
		U64 cntSizeCap		: 1;//< counter is 32-bit/64-bit wide
		U64 reserved00		: 1;
		U64 legacyRouting	: 1;//< if one, legacy-routing allowed
		U64 vendorID		: 16;//<same as if logic was PCI func.
		U64 clockPeriod		: 32;//<main-counter tick period
	};

	///
	/// Read-write register to configure the whole HPET event
	/// block. It is located at offset 0x10 in the event block.
	///
	/// @author Shukant Pal
	///
	struct Configuration
	{
		U64 overallEnable	: 1;//< enables main-counter & timers
		U64 legacyReplacement	: 1;//< if set, legacy routing enabled
		U64 reservedField0	: 62;
	};

	/* @struct HPET::InterruptStatus
	 *
	 * Read-write clear register to check each timer's interrupt
	 * status at a point in time. If any bit is set in the timerSts
	 * field, then the corresponding timer is delivering an interrupt. It
	 * is located at offset 0xF0 in the event block.
	 *
	 * @author Shukant Pal
	 * @see IA-PC HPET Specifications 1.0A
	 */
	struct InterruptStatus
	{
		U32 timerSts;
		U32 reservedField0;
	};

	struct MainCounter
	{
		U64 value;
	};

	/* @struct HPET::Timer
	 *
	 * Represents one comparator in the whole HPET which has registers
	 * placed together in memory. The size of each comparator-block is
	 * 0x20 but registers takeup only 0x18 so 8-bytes reserved reg is
	 * added.
	 *
	 * While using these registers, make sure that
	 *
	 * @author Shukant Pal
	 * @see IA-PC HPET Specifications 1.0A
	 */
	struct Timer
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
				U32 intrType		: 1;
				U32 intrEnable		: 1;
				U32 timerType		: 1;
				U32 periodicAllowed	: 1;
				U32 timerSize		: 1;
				U32 valueSet		: 1;
				U32 reservedField1	: 1;
				U32 mode32		: 1;
				U32 intrRoute		: 5;
				U32 fsbIntrEnable	: 1;
				U32 fsbIntrAllowed	: 1;
				U32 reservedField2	: 16;
				U32 intrRoutingCapable;
			};
			struct
			{
				U32 lower32;
				U32 upper32;
			};
			U64 val;
		} __attribute__((packed));

		struct Comparator
		{
			U32 comparatorValue;
			U32 reservedField;
		} __attribute__((packed));

		struct FSBInterruptRoute
		{
			U32 intrValue;  // written in FSB-intr message
			U32 intrAddress;// FSB-intr message location
		} __attribute__((packed));
	};

	inline Timer::ConfigAndCapab volatile *cfgAndCap(unsigned char n)
	{
		return ((Timer::ConfigAndCapab volatile*)
						(regBase + 0x100 + 0x20 * n));
	}

	inline Timer::Comparator volatile *comparator(unsigned char n)
	{
		return ((Timer::Comparator volatile*)
						(regBase + 0x108 + 0x20 * n));
	}

	inline Timer::FSBInterruptRoute volatile *fsbIntrRoute(unsigned char n)
	{
		return ((Timer::FSBInterruptRoute volatile*)
						(regBase + 0x110 + 0x20 * n));
	}

	CapabilityAndID volatile *capAndId;
	Configuration volatile *cfg;
	InterruptStatus volatile *intSts;
	MainCounter volatile *ctr;

	static HPET *kernelTimer;//!< This HPET device object is for use only
	 	 	 	 //! by the kernel. It is not used by any
	 	 	 	 //! user-mode software.
	static ArrayList knownHPETs;
};

}
}

#endif/* Executable/Timer/HPET.h */
