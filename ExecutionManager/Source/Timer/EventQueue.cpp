/**
 * @file EventQueue.cpp
 *
 * The event queue internally uses an engine-object (NodeSorter) to
 * keep nodes sorted and for queries. This flexible model requires it
 * to only do operations directly with timer/trigger objects.
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
 * Copyright (C) 2017, 2018 - Shukant Pal
 */
#include <Executable/Timer/EventQueue.hpp>

using namespace Executable::Timer;

EventQueue::EventQueue()
{

}

EventTrigger *EventQueue::add(Timestamp trigger, Timestamp shiftAllowed,
		EventCallback handler, void *eventObject)
{
	EventNode *open = ndsEngine.findFor(trigger, trigger + shiftAllowed);

	if(open != null) {
		return (open->add(trigger, shiftAllowed,
				handler, eventObject));
	} else {
		open = new EventNode(trigger, shiftAllowed, handler, eventObject);
		ndsEngine.put(open);
		return (open->etrigArray[0]);
	}
}

bool EventQueue::rem(EventTrigger *timer)
{
	EventNode *owner = ndsEngine.findFor(timer->triggerRange[0],
			timer->triggerRange[1]);

	if(owner == strong_null)
		return (false);

	Timestamp impendingRange[2];
	owner->del(timer, impendingRange);

	EventNode *iop, *ins;
	iop = ndsEngine.inorderPredecessor(owner);
	ins = ndsEngine.inorderSuccessor(owner);

	if(impendingRange[0] < iop->overlapRange[1])
		impendingRange[0] = iop->overlapRange[1];

	if(impendingRange[1] > ins->overlapRange[0])
		impendingRange[1] = ins->overlapRange[0];

	owner->overlapRange[0] = impendingRange[0];
	owner->overlapRange[1] = impendingRange[1];

	return (true);
}

EventNode *EventQueue::get()
{
	EventNode *mr = ndsEngine.getMostRecent();

	return (mr != ndsEngine.nil) ? mr : strong_null;
}
