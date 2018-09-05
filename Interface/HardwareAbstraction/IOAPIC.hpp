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

enum DestinationMode {
	PHYSICAL,
	LOGICAL
};

class IOAPIC final : public Lockable, public LinkedListNode
{
public:
	enum DeliveryMode
	{
		Fixed		= 0b000,
		LowestPriority	= 0b001,
		SMI		= 0b010,
		INIT		= 0b101,
		ExtINT		= 0b111
	};

	struct RedirectionEntry;
	class Input;

	unsigned char id(){ return (apicID); }
	unsigned char version(){ return (hardwareVersion); }
	unsigned char redirectionEntries(){ return (redirEntries); }
	unsigned char arbitrationId(){ return (arbId); }
	unsigned long intrBase(){ return (globalSystemInterruptBase); }
	unsigned long intrCount(){ return (redirEntries); }

	RedirectionEntry getRedirEnt(unsigned char inputSignal);
	void setRedirEnt(unsigned char inputSignal, RedirectionEntry *data);

	static void registerIOAPIC(MADTEntryIOAPIC *ioaEnt);
	static void mapAllRoutesUniformly();

	static IOAPIC *getIterable()
	{
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

	inline U32 read()
	{
		return (*(U32 volatile*)(virtAddr + 0x10));
	}

	inline U32 read(U8 regOff)
	{
		*(U32 volatile*) virtAddr = regOff;
		return (*(U32 volatile*)(virtAddr + 0x10));
	}

	inline void write(U32 val)
	{
		*(U32 volatile *)(virtAddr + 0x10) = val;
	}

	inline void write(U8 regOff, U32 val)
	{
		*(U32 volatile*) virtAddr = regOff;
		*(U32 volatile*)(virtAddr + 0x10) = val;
	}

#define IOAPICID	0x00
#define IOAPICVER	0x01
#define IOAPICARB	0x02
#define IOREDTBL(n)	0x10 + 2*n

	static ArrayList systemIOAPICInputs;
	static ArrayList systemIOAPICs;
	static unsigned long routesUnderReset;

	/*
	 * In Bochs, it has been found that the IO/APIC id is
	 * 1 and not zero.
	 */
	static unsigned long lowestID;
	friend class IRQ;

	IOAPIC(unsigned long physicalBase, unsigned long intrBase);
};

struct IOAPIC::RedirectionEntry
{
	U64 vector	: 8;
	U64 delvMode	: 3;
	U64 destMode	: 1;
	U64 delvStatus	: 1;
	U64 pinPolarity	: 1;
	U64 remoteIRR	: 1;
	U64 triggerMode	: 1;
	U64 mask	: 1;
	U64 reserved	: 39;
	U64 destination : 8;
};


class IOAPIC::Input : public Executable::IRQ
{
public:
	enum {
		INVALID_INDEX = 0xFF
	};

	inline unsigned globalIndex() {
		return (mGlobalIndex);
	}

	inline bool isLocked() {
		return (locked);
	}

	inline IOAPIC *owner() {
		return (router);
	}

	Input(unsigned globalIndex, IOAPIC *router);

	void connectTo(unsigned lvector, unsigned lapicID);
	int addDev(IRQHandler *handler, bool lock = false);
private:
	bool locked;

	unsigned mGlobalIndex;
	IOAPIC *router;
};

}// namespace HAL

#endif/* HAL/IOAPIC.hpp */
