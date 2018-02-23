///
/// @file ProcessorTopology.hpp
///
/// Declares how the kernel manages system topology in a generic, neat,
/// and platform-independent manner. Each CPU is part of "domains" which
/// are stored dynamically in a topological tree.
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
#ifndef HAL_PROCESSOR_TOPOLOGY_HPP__
#define HAL_PROCESSOR_TOPOLOGY_HPP__

#include <Executable/Scheduler.h>
#include <Executable/ScheduleRoller.h>
#include <Memory/KObjectManager.h>
#include <Synch/Spinlock.h>
#include <Utils/CircularList.h>
#include <Utils/Memory.h>
#include <Utils/LinkedList.h>

namespace HAL { struct Processor; }

namespace HAL
{

///
/// As physical chips may have different depths of topological domains
/// in the same system, identification of domains is done by their type.
///
/// @version 1.0
/// @since Silcos 2.05
/// @author Shukant Pal
///
enum DomainType
{
	System = 0xFFFFFFFF,//!< System - top domain containing all cpus
	NUMADomain = 5,     //!< NUMADomain - cpu cluster with separate memory
	Chip = 4,           //!< Chip - hardware chip contains cores
	Core = 3,           //!< Core - inside a chip but is independent
	LogicalProcessor = 1//!< LogicalProcessor - logical cpu for kernel
};

///
/// Represents a topological domain present in the system. It contains a list
/// of child domains but only one parent. Each domain has its own state and
/// features.
///
/// The lowest-level domain contains no children as it refers to only one
/// CPU. It has zero children BUT children.clnMain = (struct*) Processor
///
/// @version 1.2
/// @since Circuit 2.03
/// @author Shukant Pal
///
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

///
/// Manages the system-topology tree and provides various utility function to
/// access and manipulate it.
///
/// @version 1.0
/// @since Silcos 2.05
/// @author Shukant Pal
///
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
		static void toggleLoad(Processor *initialCPU,
					Executable::ScheduleClass cls,
					long mag);
		static void ofEach(Processor *initialCPU,
				void (*domainUpdater)(Domain *),
				unsigned long limit);
		static void forAll(Domain *in,
				void (*action)(Processor *proc));
	};
private:
	ProcessorTopology();// No usage as object is statically used
	static Domain *systemDomain kxhide;//!< Top-most domain in the topology
	static ObjectInfo *tDomain kxhide;//!< Allocator for domain objects
};

struct DomainBinding
{
	static Processor *getIdlest(Executable::ScheduleClass cls, Domain *pdom);
	static Processor *getBusiest(Executable::ScheduleClass cls, Domain *pdom);
	static Domain *findIdlestGroup(Executable::ScheduleClass cls, Domain *client);
	static Domain *findBusiestGroup(Executable::ScheduleClass cls, Domain *client);
private:
	DomainBinding();
};

}

#endif/* HAL/ProcessorTopology.hpp */
