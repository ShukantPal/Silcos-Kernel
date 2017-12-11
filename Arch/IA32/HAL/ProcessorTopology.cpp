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

using namespace HAL;

extern BOOL x2APICModeEnabled;
Domain *ProcessorTopology::systemDomain;
ObjectInfo *ProcessorTopology::tDomain;

import_asm U32 FindMaskWidth(U32);

void UpdateCoreCount(Domain *dom)
{
	++(dom->cpuCount);
}

/**
 * Function: ::HAL::ProcessorTopology::plug
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

	if(x2APICModeEnabled){
		DbgLine("X2APICMode is not supported!");
	} else {
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
	while(domLevel >= 0){
		if(!domBuiltLastTime)
			SpinLock(&cur->lock);

		domList = &cur->children;
		tmain = (Domain*) domList->ClnMain;
		domID = tcpu->TopologyIdentifiers[domLevel];

		if(tmain != NULL){
			token = tmain;
			do {
				if(token->id == domID)
					goto DomainBuildNotRequired;
				token = token->nextDomain;
			} while(tmain != token);
		} else
			goto DomainBuildRequired;

		DomainBuildRequired:
		token = new(tDomain) Domain(domID, domLevel, 0, cur);

		/* Next search will be in its children only, lock it now! */
		if(domLevel != 0)
			SpinLock(&token->lock);

		ClnInsert((CircularListNode*) token, CLN_FIRST, domList);
		domBuiltLastTime = true;
		goto ContinueBuild;

		DomainBuildNotRequired:
		if(!domLevel){// this is a processor-core (lowest) domain
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
	cur->children.ClnMain = (CircularListNode*) cur;
	tproc->DomainInfo = cur;
	Iterator::ofEach(tproc, &UpdateCoreCount, 4);
}

/**
 * Function: ::HAL::ProcessorTopology::Iterator::ofEach
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
void ProcessorTopology::Iterator::ofEach(
		Processor *initialCPU,
		void (*handler)(HAL::Domain*),
		unsigned long limit
){
	if(!initialCPU) // Client can pass NULL which means this CPU
		initialCPU = GetProcessorById(PROCESSOR_ID);

	Domain *tdom = (Domain*) initialCPU->DomainInfo;
	while(tdom != NULL && limit){
		SpinLock(&tdom->lock);
		handler(tdom);
		SpinUnlock(&tdom->lock);
		tdom = tdom->parent;
		--(limit);
	}
}
