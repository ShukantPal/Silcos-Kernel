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

#include <Exec/Scheduler.h>
#include <Exec/ScheduleClass.h>
#include <Memory/KObjectManager.h>
#include <Synch/Spinlock.h>
#include <Util/LinkedList.h>
#include <Util/CircularList.h>
#include <Util/Memory.h>

struct _PROCESSOR;
typedef _PROCESSOR Processor;

namespace HAL
{

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
		CLNODE clnLinker;
		struct
		{
			Domain *nextDomain;
			Domain *previousDomain;
		};
	};
	CircularList children;
	Domain *parent;
	unsigned int id;
	unsigned int level;
	unsigned int type;
	unsigned int cpuCount;
	KSCHED_ROLLER_DOMAIN rolDom[3];
	SPIN_LOCK lock;

	Domain()
	{
		this->parent = 0;
		this->id = id;
		this->level = 0;
		this->type = 0;
		this->cpuCount = 0;
		this->lock = 0;
	}

	Domain(unsigned int id, unsigned int level, unsigned int type,
			Domain *parent)
	{
		this->parent = parent;
		this->id = id;
		this->level = level;
		this->type = type;
		this->cpuCount = 0;
		memsetf((void*) &rolDom[0], 0,
				sizeof(KSCHED_ROLLER_DOMAIN) * 3);
		this->lock = 0;
	}
};

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
	static void ofEach(Processor *initialCPU,
			void (*domainUpdater)(Domain *), unsigned long limit);
};

private:
	ProcessorTopology();// No usage as object is statically used
	static Domain *systemDomain;
	static ObjectInfo *tDomain;
};

}

#endif/* HAL/ProcessorTopology.hpp */
