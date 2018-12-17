/**
 * File: RunqueueBalancer.cpp
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
#include <Executable/RunqueueBalancer.hpp>
#include <HardwareAbstraction/ProcessorTopology.hpp>
#include <Memory/KObjectManager.h>

using namespace HAL;
using namespace Executable;

static bool inited = false;
static const char *nmrq_Accept = "RunqueueBalancer::Accept";
static const char *nmrq_Renounce = "RunqueueBalancer::Renounce";
ObjectInfo *tRunqueueBalancer_Accept;
ObjectInfo *tRunqueueBalancer_Renounce;

/**
 * Method: RunqueueBalancer::init
 *
 * Summary:
 * Initializes the allocators for the Accept & Renounce request descriptors.
 *
 * Author: Shukant Pal
 */
void RunqueueBalancer::init()
{
	if(!inited)
	{
		tRunqueueBalancer_Accept = KiCreateType(nmrq_Accept, sizeof(RunqueueBalancer::Accept), sizeof(long),
								NULL, NULL);

		tRunqueueBalancer_Renounce = KiCreateType(nmrq_Renounce, sizeof(RunqueueBalancer::Renounce), sizeof(long),
								NULL, NULL);
	}
}

/**
 * Method: RunqueueBalancer::balanceWork
 *
 * Summary:
 * Checks whether any parent domain of the current cpu can be balanced by
 * evaluating the time until which the runqueue-balancer was stopped. If
 * the time has passed then the domain goes through the balancing routine.
 *
 * Args:
 * ScheduleClass cls - the scheduler-class for which the balancer should
 * 			operate on. Tasks from other schedulers will not
 * 			be balanced.
 *
 * Hierarchy:
 * This function is called after each ScheduleRoller::update (by the roller)
 * like as in the RoundRobin class. However, for critical appliations a
 * background kernel-thread can also call this for balancing tasks in realtime
 * classes (not yet implemented).
 *
 * Since: Silcos 2.05
 * Author: Shukant Pal
 */
void RunqueueBalancer::balanceWork(ScheduleClass cls)
{
	Domain *rqholder = GetProcessorById(PROCESSOR_ID)->domlink;

	while(rqholder->parent != NULL)
	{
		if(getSystemTime() > rqholder->taskInfo[cls].balanceDelta)
			balanceWork(cls, rqholder);
		rqholder = rqholder->parent;
	}
}

/**
 * Method: RunqueueBalancer::balanceWork
 *
 * Summary:
 * Balances the given domain tasks of the class given. If any busier domain
 * is found having a load 20% more than this ones, then a renounce request
 * is sent to the most heavily loaded processor in that domain, for transferring
 * tasks to the cpu which is least loaded in this domain.
 *
 * Further idea on this is, if all sibling domains are balanced in a uniform
 * manner, we could try shutting down the cpus in the current domain for
 * power-saving.
 *
 * Args:
 * ScheduleClass cls - schedule-class for which the domain is to get/give tasks
 * Domain *client - the domain undergoing balancing
 *
 * Since: Silcos 2.05
 * Author: Shukant Pal
 */
void RunqueueBalancer::balanceWork(ScheduleClass cls, Domain *client)
{
	Domain *busiest = DomainBinding::findBusiestGroup(cls, client);

	if(!busiest)
	{
		return;// TODO: Implement shutting off the CPUs
	}
	else if(busiest != client)
	{
		// send a renounce request, and later we will get a accept request
		Processor *srcCPU = DomainBinding::getBusiest(cls, busiest);
		Processor *dstCPU = DomainBinding::getIdlest(cls, client);

		Renounce *req = new(tRunqueueBalancer_Renounce) Renounce(cls, *busiest, *client, *srcCPU, *dstCPU);
		CPUDriver::writeRequest(*req, srcCPU);
	}

	SpinLock(&client->queueLock);
	client->taskInfo[cls].balanceDelta += (client->level + 1) * (client->level + 1) * 8;
	SpinUnlock(&client->queueLock);
}
