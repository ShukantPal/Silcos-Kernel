///
/// @file ProcessorTopology.cpp
///
/// System-topology is managed in a B-Tree like fashion where each domain-node
/// contains 'n' number of children domains. The topology is forms using the
/// apic-id of each cpu allowing cpus to be "plugged" and "unplugged".
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
#include <HardwareAbstraction/CPUID.h>
#include <HardwareAbstraction/Processor.h>
#include <HardwareAbstraction/ProcessorTopology.hpp>
#include <IA32/APIC.h>
#include <KERNEL.h>

using namespace HAL;

extern bool x2APICModeEnabled;
Domain *ProcessorTopology::systemDomain;
ObjectInfo *ProcessorTopology::tDomain;

import_asm U32 FindMaskWidth(U32);

void UpdateCoreCount(Domain *dom) // @suppress("Name convention for function")
{
	++(dom->cpuCount);
}

///
/// Essentially "plugs" the CPU into the system-topology allowing it to
/// participate in the kernel. Each successive parent domain from top to
/// bottom is inserted into the tree automatically unless they already
/// exist. After calling this, there is no need to use the apic-id to
/// access the topological position of this cpu, as the topological-ids
/// are written into the ArchCpu part of the per-cpu struct.
///
/// @version 1.0
/// @since Silcos 2.05
/// @author Shukant Pal
///
void ProcessorTopology::plug()
{
	Processor *tproc = GetProcessorById(PROCESSOR_ID);
	ArchCpu *tcpu = &tproc->hw;

	if (x2APICModeEnabled) {
		DbgLine("X2APICMode is not supported!");
	} else {
		APIC_ID apicId = tcpu->APICID;

		U32 CPUIDBuffer[4];
		__cpuid(4, 0, CPUIDBuffer);
		U32 coreIdSubfieldSize = FindMaskWidth((CPUIDBuffer[0] >> 26) + 1);
		__cpuid(1, 0, CPUIDBuffer);
		U32 smtIdSubfieldSize = FindMaskWidth((CPUIDBuffer[1] >> 16) & 0xFF)
				- coreIdSubfieldSize;
		tcpu->SMT_ID = apicId & ((1 << smtIdSubfieldSize) - 1);
		tcpu->CoreID = (apicId >> smtIdSubfieldSize)
				& ((1 << coreIdSubfieldSize) - 1);
		tcpu->PackageID = (apicId >> (coreIdSubfieldSize + smtIdSubfieldSize));
		tcpu->ClusterID = 0;

		tcpu->TopologyIdentifiers[0] = tcpu->SMT_ID;
		tcpu->TopologyIdentifiers[1] = tcpu->CoreID;
		tcpu->TopologyIdentifiers[2] = tcpu->PackageID;
		tcpu->TopologyIdentifiers[3] = tcpu->ClusterID;
	}

	Domain *cur = systemDomain, *tmain, *token;

	unsigned long domID;
	CircularList *domList;
	bool domBuiltLastTime = false;

	long domLevel = 3;
	while (domLevel >= 0) {
		if (!domBuiltLastTime) {
			SpinLock(&cur->lock);
		}

		domList = &cur->children;
		tmain = (Domain*) domList->lMain;
		domID = tcpu->TopologyIdentifiers[domLevel];

		if (tmain != NULL) {
			token = tmain;
			do {
				if (token->id == domID)
					goto DomainBuildNotRequired;
				token = token->next;
			} while (tmain != token);
		} else {
			goto DomainBuildRequired;
		}

		DomainBuildRequired: token = new (tDomain) Domain(domID, domLevel, 0,
				cur);

		/* Next search will be in its children only, lock it now! */
		if (domLevel != 0) {
			SpinLock(&token->lock);
		}

		AddCElement((CircularListNode*) token, CFIRST, domList);
		domBuiltLastTime = true;
		goto ContinueBuild;

		DomainBuildNotRequired: if (!domLevel) { // this is a processor-core (lowest) domain
			SpinUnlock(&cur->lock);
			return;
		}
		domBuiltLastTime = false;

		ContinueBuild: SpinUnlock(&cur->lock);
		cur = token;
		--(domLevel);
	}

	cur->type = PROCESSOR_HIERARCHY_LOGICAL_CPU;
	cur->children.lMain = (CircularListNode*) tproc;
	tproc->domlink = cur;
	Iterator::ofEach(tproc, &UpdateCoreCount, 4);
}

///
/// Performs an iteration from the bottom to the top of the topological-tree
/// and toggles the load of each domain by the given magnitude.
///
/// @param initialCPU - the CPU from which iteration starts
/// @param cls - scheduling-class for which the load is being toggled
/// @param mag - magnitude by which load is to change (may be +ve or -ve)
/// @version 1.0
/// @since Silcos 2.05
/// @author Shukant Pal
///
void ProcessorTopology::Iterator::toggleLoad(Processor *initialCPU,
		Executable::ScheduleClass cls, long mag)
{
	if (!initialCPU) {
		initialCPU = GetProcessorById(PROCESSOR_ID);
	}

	Domain *domain = (Domain *) initialCPU->domlink;
	while (domain != NULL) {
		domain->taskInfo[cls].load += mag;
		__mfence
		domain = domain->parent;
	}
}

///
/// Performs an action for each domain upto to the given limit using a
/// call-back functor starting from the cpu-level domain.
///
/// @param initialCPU - cpu from which the action to take place
/// @param handler - call-back function to perform action on behalf of this
/// @param limit - height of domain until which action is to be performed
/// @version 1.0
/// @since Silcos 2.05
/// @author Shukant Pal
///
void ProcessorTopology::Iterator::ofEach(Processor *initialCPU,
		void (*handler)(HAL::Domain*), unsigned long limit)
{
	if (!initialCPU) // Client can pass NULL which means this CPU
	{
		initialCPU = GetProcessorById(PROCESSOR_ID);
	}

	Domain *tdom = (Domain*) initialCPU->domlink;
	while (tdom != NULL && limit) {
		SpinLock(&tdom->lock);
		handler(tdom);
		SpinUnlock(&tdom->lock);
		tdom = tdom->parent;
		--(limit);
	}
}

///
/// Performs an action for each cpu present in the given domain by going
/// through the topological sub-tree.
///
/// @param in - domain in which all cpus on which action is called are
/// @param action - call-back functor which should perform the action
/// @version 1.0
/// @since Silcos 2.05
/// @author Shukant Pal
///
void ProcessorTopology::Iterator::forAll(Domain *in,
		void (*action)(Processor *))
{
	/*
	 * First we go to the bottom of the tree to get the processor on which
	 * we firstly will call action.
	 */

	Domain *cur = in;
	while (cur->type != PROCESSOR_HIERARCHY_LOGICAL_CPU)
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

	enum
	{
		upward, downward
	};

	bool dir = downward; // this the direction of the last jump
	while (cur != in->next) {
		if (cur->type == PROCESSOR_HIERARCHY_LOGICAL_CPU) {
			action((Processor*) cur->children.lMain);

			cur = cur->next;

			if (cur == (Domain*) cur->parent->children.lMain) {
				cur = cur->parent->next;
				dir = upward;
			}
		} else {
			if (cur == (Domain*) cur->parent->children.lMain && dir == upward) {
				cur = cur->parent->next;
				dir = upward;
			} else {
				cur = (Domain*) cur->children.lMain;
				dir = downward;
			}
		}
	}
}

///
/// Finds the cpu for which the load of each successive parent domain is the
/// least locally.
///
/// @param cls - scheduling-class for which load is calculated
/// @param pdom - parent-domain in which the idlest processor is to be searched
/// @return - cpu for which parent-domains have least local load
/// @version 1.0
/// @since Silcos 2.05
/// @author Shukant Pal
///
Processor *DomainBinding::getIdlest(Executable::ScheduleClass cls, Domain *pdom)
{
	Domain *testee, *mdomain, *lloaded = NULL;
	Executable::ScheduleDomain *lrol;

	if (pdom->type == DomainType::LogicalProcessor)
		return ((Processor*) (pdom->children.lMain));

	while (pdom && pdom->type != PROCESSOR_HIERARCHY_LOGICAL_CPU) {
		mdomain = (Domain*) pdom->children.lMain;
		if (mdomain == null)
			return (null);

		testee = mdomain->next;

		lloaded = mdomain; // pre-initialize ldom as it has to be something
		lrol = mdomain->taskInfo + cls; // and prevent null-checks in loop

		while (testee != mdomain) {
			if (testee->taskInfo[cls].load < lrol->load) {
				lloaded = testee;
				lrol = testee->taskInfo + cls;
			}

			testee = testee->next;
		}

		pdom = lloaded;
	}

	if (lloaded)
		return ((Processor *) lloaded->children.lMain);
	else
		return (null);
}

///
/// Finds the cpu for which the load of each successive parent domain is the
/// highest locally.
///
/// @param cls - scheduling class for which the load is calculated
/// @param pdom - parent domain in which highest-load cpu is to be found
/// @return - cpu for which each parent-domain has highest local load
/// @version 1.0
/// @since Silcos 3.02
/// @author Shukant Pal
///
Processor *DomainBinding::getBusiest(Executable::ScheduleClass cls,
		Domain *pdom)
{
	Domain *testee, *mdomain, *hloaded = NULL;
	Executable::ScheduleDomain *hrol;

	if (pdom->type == DomainType::LogicalProcessor)
		return ((Processor*) (pdom->children.lMain));

	while (pdom && pdom->type != PROCESSOR_HIERARCHY_LOGICAL_CPU) {
		mdomain = (Domain*) pdom->children.lMain;
		testee = mdomain->next;

		hloaded = mdomain; //pre-initialize hdom as it has to be something
		hrol = mdomain->taskInfo + cls; // and prevent null-checks in loop

		while (testee != mdomain) {
			if (testee->taskInfo[cls].load > hrol->load) {
				hloaded = testee;
				hrol = testee->taskInfo + cls;
			}

			testee = testee->next;
		}

		pdom = hloaded;
	}

	if (hloaded)
		return ((Processor *) hloaded->children.lMain);
	else
		return (null);
}

///
/// Searches for the domain under the same parent (a.k.a sibling of given
/// donor domain) which has the least load & is willing to participate in
/// load-balancing. The given load diff. is guaranteed to be 20% atleast.
///
/// @param cls - scheduling class for which load is calculated
/// @param donor - given domain by which load is to be compared
/// @return - domain whose load is atleast 20% less than donor, if found.
/// @deprecated (not used in balancing now)
/// @version 1.0
/// @since Silcos 2.05
/// @author Shukant Pal
///
Domain *DomainBinding::findIdlestGroup(Executable::ScheduleClass cls,
		Domain *donor)
{
	Domain *test = donor->next, *bestFound = NULL;

	while (test != donor) {
		if (!bestFound
				&& donor->taskInfo[cls].load
						<= test->taskInfo[cls].load * 6 / 5) {
			test = test->next;
			continue; // not suitable
		}

		if (bestFound
				&& test->taskInfo[cls].load <= bestFound->taskInfo[cls].load) {
			bestFound = test;
		} else if (!bestFound) {
			bestFound = test;
		}

		test = test->next;
	}

	return (bestFound);
}

///
/// Searches for the domain under the same parent (a.ka. sibling of given
/// client) whose load is the greatest and is willing to participate in load
/// balancing. The load diff. is guaranteed to be atleast 20%.
///
/// @param cls - scheduling class for which load is calculated
/// @param client - client which should take the tasks
/// @return - domain with least load and atleast 20% load diff.
/// @version 1.0
/// @since Silcos 2.05
/// @author Shukant Pal
///
Domain *DomainBinding::findBusiestGroup(Executable::ScheduleClass cls,
		Domain *client)
{
	Domain *host = client->parent, *test = client->next, *bestFound = NULL;

	while (test != client) {
		if (!bestFound
				&& client->taskInfo[cls].load
						>= test->taskInfo[cls].load * 4 / 5) {
			test = test->next;
			continue;
		}

		if (bestFound
				&& test->taskInfo[cls].load >= bestFound->taskInfo[cls].load) {
			bestFound = test;
		} else if (!bestFound) {
			bestFound = test;
		}

		test = test->next;
	}

	return (bestFound);
}
