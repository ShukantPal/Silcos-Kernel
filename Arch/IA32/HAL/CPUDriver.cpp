/**
 * File: CPUDriver.cpp
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
#include <HAL/Processor.h>
#include <KERNEL.h>

using namespace HAL;

/**
 * Method: HAL::CPUDriver::readRequest
 *
 * Summary:
 * Pulls the first request in the queue for the given processor, subsequently
 * removing it.
 *
 * Args:
 * Processor *proc - cpu for which the request is to be given
 *
 * Returns:
 * first request in queue, for 'proc'; NULL, if all were handled.
 *
 * Author: Shukant Pal
 */
IPIRequest *CPUDriver::readRequest(Processor *proc)
{
	SpinLock(&proc->migrlock);

	CircularList *arl = &proc->actionRequests;
	IPIRequest *req;

	if(arl->lMain != null)
	{
		CircularListNode *node = arl->lMain;
		node->last->next = node->next;
		node->next->last = node->last;
		--(arl->count);

		if(arl->count == 0)
			arl->lMain = null;

		req = (IPIRequest *) node;
	}
	else
	{
		req = NULL;
	}

	SpinUnlock(&proc->migrlock);
	return (req);
}

/**
 * Method: HAL::CPUDriver::writeRequest
 *
 * Summary:
 * Writes the request into the queue for the specific processor.
 *
 * Args:
 * IPIRequest& req - the request to write in the cpu's actionRequests
 * Processor *proc - the cpu to send the request to
 *
 * Author: Shukant Pal
 */
void CPUDriver::writeRequest(IPIRequest &req, Processor *proc)
{
	Dbg("Wat");
	SpinLock(&proc->migrlock);
	Dbg("D");

	CircularList *arl = &proc->actionRequests;

	if(arl->lMain != null)
	{
		CircularListNode *main = arl->lMain;

		req.reqlink.next = main;
		req.reqlink.last = main->last;

		main->last->next = (CircularListNode*) &req;
		main->last = (CircularListNode*) &req;

		arl->lMain = (CircularListNode*) &req;
	}
	else
	{
		arl->lMain = (CircularListNode*) &req;
		req.reqlink.next = (CircularListNode*) &req;
		req.reqlink.last = (CircularListNode*) &req;
	}

	++(arl->count);

	SpinUnlock(&proc->migrlock);
	APIC::triggerIPI(proc->Hardware.APICID, 0xFD);
}
