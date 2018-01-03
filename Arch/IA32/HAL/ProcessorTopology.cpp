/**
 * File: ProcessorTopology.cpp
 *
 * Summary:
 * Hardware-topology is maintained in a in-optimized B-tree fashion allowing
 * CPUs to be plugged & unplugged. A CPU is a part of a hierarchy of CPU
 * domains, in which it is at the lowest-level.  Each domain has a set of
 * children (unless it is the lowest-level domain) and a parent-domain unless
 * it is the top-most one. This allows iterating over the topology and maintain
 * resources in a organized manner.
 * 
 * Functions:
 * RegisterProcessorCount - (++) the record of no. of processors in a domain
 * ::HAL::ProcessorTopology::plug - plug-in a new-processor in the topology
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
#include <IA32/APIC.h>
#include <HAL/CPUID.h>
#include <HAL/Processor.h>
#include <HAL/ProcessorTopology.hpp>
#include <KERNEL.h>

using namespace HAL;

extern bool x2APICModeEnabled;
Domain *ProcessorTopology::systemDomain;
ObjectInfo *ProcessorTopology::tDomain;

import_asm U32 FindMaskWidth(U32);

void UpdateCoreCount(Domain *dom)
{
	++(dom->cpuCount);
}

/**
 * Method: HAL::ProcessorTopology::plug
 *
 * Summary:
 * Plugs the running processor into the CPU-topology tree and registers all its
 * parent domains. It gathers all topological-IDs and writes them to the
 * per-CPU TopologyIdentifiers. From there, each successive domain is added
 * into the tree (unless it already exists) till the processor is inserted at
 * the end.
 *
 * The domain representing the single CPU (logical core) is reverse mapped and
 * the relevant CPU can be found by Domain::children::ClnMain (but the core is
 * not inserted as a element & ClnCount is still zero).
 *
 * Changes:
 * # When the child domain is allocated, it is immediately locked so that this
 * CPU can continuously go to the lower-topological levels.
 *
 * Author: Shukant Pal
 */
void ProcessorTopology::plug()
{
	Processor *tproc = GetProcessorById(PROCESSOR_ID);
	ProcessorInfo *tcpu = &tproc->Hardware;

	if(x2APICModeEnabled)
	{
		DbgLine("X2APICMode is not supported!");
	}
	else
	{
		APIC_ID apicId = tcpu->APICID;

		U32 CPUIDBuffer[4];
		CPUID(4, 0, CPUIDBuffer);
		U32 coreIdSubfieldSize = FindMaskWidth((CPUIDBuffer[0] >> 26) + 1);
		CPUID(1, 0, CPUIDBuffer);
		U32 smtIdSubfieldSize = FindMaskWidth((CPUIDBuffer[1] >> 16) & 0xFF) - coreIdSubfieldSize;
		tcpu->SMT_ID = apicId & ((1 << smtIdSubfieldSize) - 1);
		tcpu->CoreID = (apicId >> smtIdSubfieldSize) & ((1 << coreIdSubfieldSize) - 1);
		tcpu->PackageID = (apicId >> (coreIdSubfieldSize + smtIdSubfieldSize));
		tcpu->ClusterID = 0;

		tcpu->TopologyIdentifiers[0] = tcpu->SMT_ID;
		tcpu->TopologyIdentifiers[1] = tcpu->CoreID;
		tcpu->TopologyIdentifiers[2] = tcpu->PackageID;
		tcpu->TopologyIdentifiers[3] = tcpu->ClusterID;
	}

	Domain *cur = systemDomain, *tmain, *token;

	/* Id for this domain */
	unsigned long domID;

	/* Search for 'new' domain already existing occurs in this list. */
	CircularList *domList;

	/* true, if parent-domain of the 'new' one was allocated here only. */
	bool domBuiltLastTime = false;

	long domLevel = 3;
	while(domLevel >= 0)
	{
		if(!domBuiltLastTime)
		{
			SpinLock(&cur->lock);
		}

		domList = &cur->children;
		tmain = (Domain*) domList->lMain;
		domID = tcpu->TopologyIdentifiers[domLevel];

		if(tmain != NULL)
		{
			token = tmain;
			do
			{
				if(token->id == domID)
					goto DomainBuildNotRequired;
				token = token->next;
			} while(tmain != token);
		}
		else
		{
			goto DomainBuildRequired;
		}

		DomainBuildRequired:
		token = new(tDomain) Domain(domID, domLevel, 0, cur);

		/* Next search will be in its children only, lock it now! */
		if(domLevel != 0)
		{
			SpinLock(&token->lock);
		}

		ClnInsert((CircularListNode*) token, CLN_FIRST, domList);
		domBuiltLastTime = true;
		goto ContinueBuild;

		DomainBuildNotRequired:
		if(!domLevel)
		{// this is a processor-core (lowest) domain
			SpinUnlock(&cur->lock);
			return;
		}
		domBuiltLastTime = false;

		ContinueBuild:
		SpinUnlock(&cur->lock);
		cur = token;
		--(domLevel);
	}

	cur->type = PROCESSOR_HIERARCHY_LOGICAL_CPU;
	cur->children.lMain = (CircularListNode*) cur;
	tproc->domlink = cur;
	Iterator::ofEach(tproc, &UpdateCoreCount, 4);
}

/**
 * Method: ::HAL::ProcessorTopology::Iterator::toggleLoad
 *
 * Summary:
 * Iterates all the way to the top toggling each domain's load (for a specific
 * sched-class), by the given magnitude.
 *
 * Args:
 * Processor *initialCPU - the cpu whose load is to be toggled
 * Executable::ScheduleClass cls - index of the scheduler's class
 * long magnitude - amount by which to change the load (+ve or -ve)
 *
 * Author: Shukant Pal
 */
void ProcessorTopology::Iterator::toggleLoad(Processor *initialCPU, Executable::ScheduleClass cls, long mag)
{
	if(!initialCPU)
	{
		initialCPU = GetProcessorById(PROCESSOR_ID);
	}

	Domain *domain = (Domain *) initialCPU->domlink;
	while(domain != NULL)
	{
		domain->taskInfo[cls].load += mag;
		__mfence
		domain = domain->parent;
	}
}

/**
 * Method: HAL::ProcessorTopology::Iterator::ofEach
 *
 * Summary:
 * Iterates to the top of the topology starting from a initial-processor. It
 * calls a callback-handler which should handle the domain (without locking)
 * in a synchronized manner.
 *
 * Args:
 * Processor* initialCPU - CPU of the initial-domain; NULL, if this CPU is used
 * void (*handle)(Domain*) - call-back handler for all domains
 * unsigned long limit - limit for the topology-level
 *
 * Author: Shukant Pal
 */
void ProcessorTopology::Iterator::ofEach(Processor *initialCPU, void (*handler)(HAL::Domain*),
						unsigned long limit)
{
	if(!initialCPU) // Client can pass NULL which means this CPU
	{
		initialCPU = GetProcessorById(PROCESSOR_ID);
	}

	Domain *tdom = (Domain*) initialCPU->domlink;
	while(tdom != NULL && limit)
	{
		SpinLock(&tdom->lock);
		handler(tdom);
		SpinUnlock(&tdom->lock);
		tdom = tdom->parent;
		--(limit);
	}
}

/**
 * Method: ProcessorTopology::Iterator::forAll
 *
 * Summary:
 * Executes a action for all the processor in a domain, going through each
 * nested domain serially. This means from the lowest APIC-id to the highest
 * if the processor are sorted by order in each domain.
 *
 * Args:
 * Domain *in - parent domain in which all processor required exist
 * void (*action)(Processor *) - action to take on each cpu
 *
 * Author: Shukant Pal
 */
void ProcessorTopology::Iterator::forAll(Domain *in, void (*action)(Processor *))
{
	/*
	 * First we go to the bottom of the tree to get the processor on which
	 * we firstly will call action.
	 */

	Domain *cur = in;
	while(cur->type != PROCESSOR_HIERARCHY_LOGICAL_CPU)
		cur = (Domain*) cur->children.lMain;

	/*
	 * Now, we will keep going to the right until we reach the end of the
	 * list of domains (in cur->parent). After that, go up and do the same
	 * until we reach the top.
	 *
	 * On finishing all domains we go the parent's next domain and thus
	 * wait until the domain becomes in->next (not in, as we'll always skip
	 * the parent).
	 */

	enum { upward, downward };

	bool dir = downward;// this the direction of the last jump
	while(cur != in->next)
	{
		if(cur->type == PROCESSOR_HIERARCHY_LOGICAL_CPU)
		{
			action((Processor*) cur->children.lMain);

			cur = cur->next;

			if(cur == (Domain*) cur->parent->children.lMain)
			{
				cur = cur->parent->next;
				dir = upward;
			}
		}
		else
		{
			if(cur == (Domain*) cur->parent->children.lMain && dir == upward)
			{
				cur = cur->parent->next;
				dir = upward;
			}
			else
			{
				cur = (Domain*) cur->children.lMain;
				dir = downward;
			}
		}
	}
}

/**
 * Method: HAL::ProcessorTopology::DomainBinding::getIdlest
 *
 * Summary:
 * Find the processor which has the least load in the given domain. In an
 * ideal situation, the returned cpu would really have the least load in the
 * given domain, but practically this function always goes the sub-domains in
 * which the load is minimum. It assumes the subdomains are internally balanced
 * and thus will give almost relevant results.
 *
 * This technique of searching for least-loaded subdomains speedens lookup as
 * not all cpus are tested for.
 *
 * Args:
 * Executable::ScheduleClass cls - scheduling class for which load is to be
 * 					the least
 * Domain *pdom - parent domain in which the cpu is to be searched
 *
 * Return:
 * the processor whose successive parent domains are least-loaded relatively to
 * their sibling (.i.e almost idlest processor)
 *
 * Author: Shukant Pal
 */
Processor *DomainBinding::getIdlest(Executable::ScheduleClass cls, Domain *pdom)
{
	Domain *testee, *mdomain, *lloaded = NULL;
	Executable::ScheduleDomain *lrol;

	while(pdom && pdom->type != PROCESSOR_HIERARCHY_LOGICAL_CPU)
	{
		mdomain = (Domain*) pdom->children.lMain;
		testee = mdomain->next;

		lloaded = mdomain;// pre-initialize ldom as it has to be something
		lrol = mdomain->taskInfo + cls;// and prevent null-checks in loop

		do
		{
			if(testee->taskInfo[cls].load < lrol->load)
			{
				lloaded = testee;
				lrol = testee->taskInfo + cls;
			}

			testee = testee->next;
		} while(testee != mdomain);

		pdom = lloaded;
	}

	if(lloaded)
		return (Processor *) lloaded->children.lMain;
	else
		return (NULL);
}

/**
 * Method: ::HAL::ProcessorTopology::DomainBinding::getBusiest
 *
 * Summary:
 * Find the processor which has the highest load in the given domain. In an
 * ideal situation, the returned cpu would really have the highest load in the
 * given domain, but practically this function always goes the sub-domains in
 * which the load is the highest. It assumes the subdomains are internally balanced
 * and thus will give almost relevant results.
 *
 * This technique of searching for highest-loaded subdomains speedens lookup as
 * not all cpus are tested for.
 *
 * Args:
 * Executable::ScheduleClass cls - scheduling class for which load is to be
 * 					the highest
 * Domain *pdom - parent domain in which the cpu is to be searched
 *
 * Return:
 * the processor whose successive parent domains are most-loaded relatively to
 * their sibling (.i.e most heavily loaded processor)
 *
 * Author: Shukant Pal
 */
Processor *DomainBinding::getBusiest(Executable::ScheduleClass cls, Domain *pdom)
{
	Domain *testee, *mdomain, *hloaded = NULL;
	Executable::ScheduleDomain *hrol;

	while(pdom && pdom->type != PROCESSOR_HIERARCHY_LOGICAL_CPU)
	{
		mdomain = (Domain*) pdom->children.lMain;
		testee = mdomain->next;

		hloaded = mdomain;//pre-initialize hdom as it has to be something
		hrol = mdomain->taskInfo + cls;// and prevent null-checks in loop

		do
		{
			if(testee->taskInfo[cls].load > hrol->load)
			{
				hloaded = testee;
				hrol = testee->taskInfo + cls;
			}

			testee = testee->next;
		} while(testee != mdomain);

		pdom = hloaded;
	}

	if(hloaded)
		return (Processor *) hloaded->children.lMain;
	else
		return (NULL);
}

/**
 * Method: HAL::ProcessorTopology::DomainBinding::findDonee
 *
 * Summary:
 * Searches for a domain under the same parent, which has a load atleast 20%
 * less than the donor. It locks the search-lock for the parent, and for each
 * sibling it tests for load-balancing.
 *
 * After selecting the donee, it will subtract the given 'delta' from the donor
 * load and add it to the donee load.
 *
 * Args:
 * Executable::ScheduleClass cls - the scheduling class whose runqueue is to balance
 * Domain *client - domain that is to be balanced (by pushing tasks away)
 *
 * Note:
 * Donor must have a parent, thus, it cannot be the 'system' domain, honestly.
 *
 * Author: Shukant Pal
 */
Domain *DomainBinding::findIdlestGroup(Executable::ScheduleClass cls, Domain *donor)
{
	Domain *host = donor->parent, *test = donor->next, *bestFound = NULL;

	while(test != donor)
	{
		if(!bestFound && donor->taskInfo[cls].load <= test->taskInfo[cls].load * 6 / 5)
		{
			test = test->next;
			continue;// not suitable
		}

		if(bestFound && test->taskInfo[cls].load <= bestFound->taskInfo[cls].load)
		{
			bestFound = test;
		}
		else if(!bestFound)
		{
			bestFound = test;
		}

		test = test->next;
	}

	return (bestFound);
}

/**
 * Method: HAL::ProcessorTopology::DomainBinding::findBusiestGroup
 *
 * Summary:
 * Searches for a domain under the same parent, which is the busiest.
 *
 * Args:
 * ScheduleClass cls - schedule class for which load is to be maximum
 * Domain *client - one which is going to transfer with client
 *
 * Author: Shukant Pal
 */
Domain *DomainBinding::findBusiestGroup(Executable::ScheduleClass cls, Domain *client)
{
	Domain *host = client->parent, *test = client->next, *bestFound = NULL;

	while(test != client)
	{
		if(!bestFound && client->taskInfo[cls].load >= test->taskInfo[cls].load * 4 / 5)
		{
			test = test->next;
			continue;
		}

		if(bestFound && test->taskInfo[cls].load >= bestFound->taskInfo[cls].load)
		{
			bestFound = test;
		}
		else if(!bestFound)
		{
			bestFound = test;
		}

		test = test->next;
	}

	return (bestFound);
}
