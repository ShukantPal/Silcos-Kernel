/**
 * File: RunqueueBalancer.cpp
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

#include <HAL/ProcessorTopology.hpp>
#include <Exec/RunqueueBalancer.hpp>
#include <Memory/KObjectManager.h>

using namespace HAL;
using namespace Executable;

static bool inited = false;
static char *nmrq_Accept = "RunqueueBalancer::Accept";
static char *nmrq_Renounce = "RunqueueBalancer::Renounce";
ObjectInfo *tRunqueueBalancer_Accept;
ObjectInfo *tRunqueueBalancer_Renounce;

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

void RunqueueBalancer::balanceWork(ScheduleClass cls)
{
	Domain *rqholder = GetProcessorById(PROCESSOR_ID)->domlink;

	while(rqholder->parent != NULL)
	{
		if(getSystemTime() > rqholder->taskInfo[cls].balanceDelta)
			balanceWork(cls, rqholder);
		else
			Dbg("called");
		rqholder = rqholder->parent;
	}
}

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
	client->taskInfo[cls].balanceDelta += client->level * client->level * 64;
	SpinUnlock(&client->queueLock);
}
