/**
 * @file IOAPIC.hpp
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
#ifndef HAL_IOAPIC_HPP__
#define HAL_IOAPIC_HPP__

#include <Object.hpp>
#include <ACPI/MADT.h>
#include <Executable/IRQHandler.hpp>
#include <Synch/Spinlock.h>
#include <Utils/ArrayList.hpp>
#include <Utils/LinkedList.h>
#include <TYPE.h>

namespace HAL
{

enum DeliveryMode
{
	Fixed		= 0b000,
	LowestPriority	= 0b001,
	SMI		= 0b010,
	INIT		= 0b101,
	ExtINT		= 0b111
};

enum DestinationMode {
	PHYSICAL,
	LOGICAL
};

/**
 * Driver for IOAPIC hardware, which controls how peripheral device
 * interrupts are handled. Each <tt>IOAPIC</tt> exposes up to 24
 * redirection entries for different hardware input pins, which can
 * be individually modified.
 */
class IOAPIC final : public Lockable, public LinkedListNode
{
public:

	struct RedirectionEntry;
	class Input;

	unsigned char id(){ return (apicID); }
	unsigned char version(){ return (hardwareVersion); }
	unsigned char redirectionEntries(){ return (redirEntries); }
	unsigned char arbitrationId(){ return (arbId); }
	unsigned long gsib(){ return (globalSystemInterruptBase); }
	unsigned long intrCount(){ return (redirEntries); }

	RedirectionEntry readRedirection(unsigned char inputSignal);
	void writeRedirection(unsigned inputSignal,
			RedirectionEntry *data);
	void writeRedirection(unsigned input,
			RedirectionEntry (*callback)(RedirectionEntry));

	static void registerIOAPIC(MADTEntryIOAPIC *ioaEnt);
	static void mapAllRoutesUniformly();

	static IOAPIC *getIterable() {
		return ((IOAPIC*) systemIOAPICs.get(lowestID));
	}

	static inline Input *inputAt(unsigned globalIndex) {
		return ((Input*) systemIOAPICInputs.get(globalIndex));
	}

	static Input *getOptimizedInput(Executable::IRQHandler *dev);
	static Input *getOptimizedInput(Executable::IRQHandler *dev, unsigned mask);
private:
	unsigned char apicID;
	unsigned char hardwareVersion;
	unsigned char redirEntries;
	unsigned char arbId;
	unsigned long physRegs;
	unsigned long virtAddr;
	unsigned long globalSystemInterruptBase;

#define IOAPICID	0x00
#define IOAPICVER	0x01
#define IOAPICARB	0x02
#define IOREDTBL(n)	0x10 + 2*n

	inline U32 read(U8 regOff) {
		*(U32 volatile*) virtAddr = regOff;
		return (*(U32 volatile*)(virtAddr + 0x10));
	}

	inline void write(U8 regOff, U32 val) {
		*(U32 volatile*) virtAddr = regOff;
		*(U32 volatile*)(virtAddr + 0x10) = val;
	}

	static ArrayList systemIOAPICInputs;
	static ArrayList systemIOAPICs;
	static unsigned long routesUnderReset;

	/* In Bochs, it has been found that the IO/APIC id is
	  1 and not zero, just a "jugaad (@Hindi)" */
	static unsigned long lowestID;
	friend class IRQ;

	IOAPIC(unsigned long physicalBase, unsigned long intrBase);
};

/**
 * The IOAPIC uses <tt>RedirectionEntry</tt> objects to configure
 * how the hardware handles interrupts asserted by peripheral devices.
 * Each <tt>RedirectionEntry</tt> can be access given the <tt>IOAPIC
 * </tt> driver by <tt>driver->readRedirection(input)</tt>.
 *
 * These objects are memory-mapped IO registers, and hence should be
 * modified using the read-modify-write process.
 *
 * @see 82093AA I/O Advanced Programmable Interrupt Controller datasheet
 */
struct IOAPIC::RedirectionEntry
{
	U64 localVector	: 8;
	U64 delMod	: 3;
	U64 destMod	: 1;
	U64 delvSts	: 1;
	U64 pinPolarity	: 1;
	U64 remoteIRR	: 1;
	U64 triggerMode	: 1;
	U64 intMask	: 1;
	U64 reserved	: 39;
	U64 destination : 8;
};

/**
 * Driver for IOAPIC input hardware, allowing it to be used in
 * harmony with the systems put in place. It can be accessed
 * using <tt>IOAPIC::inputAt(globalIndex)</tt>.
 *
 * Each IOAPIC input is indexed using two indices - its local
 * & global indices. The local index is just the input pin
 * number on the IOAPIC. The global index is unique and is
 * calculated as <tt>IOAPIC->globalSystemInterruptBase +
 * localIndex</tt>
 *
 * <tt>IRQHandler</tt> drivers can lock IOAPIC input pins, to
 * prevent IRQ sharing. That is not enforced in code (yet!)
 */
class IOAPIC::Input : public Executable::IRQ
{
public:
#define INVALID_INDEX	0xFF

	inline unsigned localIndex() {
		return (mGlobalIndex - hub->globalSystemInterruptBase);
	}

	inline unsigned globalIndex() {
		return (mGlobalIndex);
	}

	inline bool isLocked() {
		return (locked);
	}

	inline IOAPIC *owner() {
		return (hub);
	}

	inline RedirectionEntry redirection() {
		return (hub->readRedirection(localIndex()));
	}

	Input(unsigned globalIndex, IOAPIC *router);
	int addDev(IRQHandler *handler, bool lock = false);
private:
	bool locked;
	unsigned mGlobalIndex;
	IOAPIC *hub;

	void connectTo(unsigned lvector, unsigned lapicID);
};

}// namespace HAL

#endif/* HAL/IOAPIC.hpp */
