/**
 * File: ProcessorTopology.hpp
 *
 * Summary:
 * 
 * Functions:
 *
 * Origin:
 *
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
#ifndef INTERFACE_HAL_PROCESSORTOPOLOGY_HPP_
#define INTERFACE_HAL_PROCESSORTOPOLOGY_HPP_

#include <Executable/Scheduler.h>
#include <Executable/ScheduleRoller.h>
#include <Memory/KObjectManager.h>
#include <Synch/Spinlock.h>
#include <Util/LinkedList.h>
#include <Util/CircularList.h>
#include <Util/Memory.h>

struct Processor;

namespace HAL
{

/**
 * Enum: DomainType
 *
 * Summary:
 * Specifies the type of domain for recognization of the physical significance
 * in their topology. Better than domain->level, as the level for the same
 * domain type in different chips --may be different--.
 *
 * Author: Shukant Pal
 */
enum DomainType
{
	System = 0xFFFFFFFF,
	NUMADomain = 5,
	Chip = 4,
	Core = 3,
	LogicalProcessor = 1
};

/**
 * Struct: Domain
 *
 * Summary:
 * Represents a hardware-topology and a collection of virtual-processors. They
 * are maintained in a B-tree-like fashion to represent the resources at each
 * hierarchy in the topology by the ProcessorTopology class.
 *
 * Changes:
 * #BUG: Moved children after liLinker
 */
struct Domain
{
	union
	{
		LinkedListNode liLinker;
		CircularListNode clnLinker;
		struct
		{
			Domain *next;
			Domain *last;
		};
	};
	CircularList children;
	Domain *parent;
	unsigned int id;
	unsigned int level;
	unsigned int type;
	unsigned int cpuCount;
	Executable::ScheduleDomain taskInfo[3];
	Spinlock queueLock;// Lock for executing balance-routine with this domain
	Spinlock searchLock;// Serializing searches for direct child-domains for balancing
	Spinlock lock;// for changes to domain data

	Domain()
	{
		this->parent = 0;
		this->id = id;
		this->level = 0;
		this->type = 0;
		this->cpuCount = 0;
		SpinUnlock(&queueLock);
		SpinUnlock(&searchLock);
		SpinUnlock(&lock);
	}

	Domain(unsigned int id, unsigned int level, unsigned int type,
			Domain *parent)
	{
		this->parent = parent;
		this->id = id;
		this->level = level;
		this->type = type;
		this->cpuCount = 0;
		SpinUnlock(&queueLock);
		SpinUnlock(&searchLock);
		SpinUnlock(&lock);
	}
};

/**
 * Class: ProcessorTopology
 *
 * Summary:
 * Manages the topological relationships of hardware-processors and provides
 * lookups based on various criteria.
 *
 * Function:
 * init - setup the basic functionality for starting up the static-class
 * plug - register a cpu in the topology
 * unplug - remove a cpu from the topology
 * del - (not impl)
 *
 * Sub-classes:
 * Iterator - used for iterating over the topological tree (vertical/horizontal fashion)
 * DomainBinding - search for various things in a specific domain
 *
 * Author: Shukant Pal
 */
class ProcessorTopology final
{
public:
	static inline void init()
	{
		tDomain = KiCreateType("::HAL::Domain", sizeof(Domain),
				sizeof(long), 0, 0);
		systemDomain = new(tDomain) Domain();
	}

	static void plug();
	static void unplug();
	static void del(unsigned long id);

	struct Iterator
	{
		static void toggleLoad(Processor *initialCPU, Executable::ScheduleClass cls, long mag);

		static void ofEach(Processor *initialCPU, void (*domainUpdater)(Domain *),
					unsigned long limit);

		static void forAll(Domain *in, void (*action)(Processor *proc));
	};

private:
	ProcessorTopology();// No usage as object is statically used
	static Domain *systemDomain kxhide;
	static ObjectInfo *tDomain kxhide;
};

struct DomainBinding
{
	static Processor *getIdlest(Executable::ScheduleClass cls, Domain *pdom);
	static Processor *getBusiest(Executable::ScheduleClass cls, Domain *pdom);
	static Domain *findIdlestGroup(Executable::ScheduleClass cls, Domain *client);
	static Domain *findBusiestGroup(Executable::ScheduleClass cls, Domain *client);
};

}

#endif/* HAL/ProcessorTopology.hpp */
