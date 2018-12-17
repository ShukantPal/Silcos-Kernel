/**
 * @file EventNode.cpp
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
#include <Executable/Timer/Event.hpp>
#include <Executable/Timer/EventGroup.hpp>
#include <Memory/KObjectManager.h>

using namespace Executable::Timer;

ObjectInfo *Executable::Timer::t_EventNode =
		KiCreateType("Executable::Timer::EventNode",
		sizeof(EventGroup), L1_CACHE_ALIGN, null, null);

/**
 * Initializes the event node with one trigger object, which can
 * be accessed using (@code evn->getTriggerByIndex(0)) before it
 * is deleted.
 *
 * @param trigger - the timestamp after the event can be executed
 * @param shiftAllowed - the delay after trigger which is allowed
 * 		before which the event should be executed
 * @param handler - the callback which should be called to execute the
 * 		event
 * @param eventObject - optional parameter to pass to the callback
 */
EventGroup::EventGroup(Timestamp trigger, Delay shiftAllowed,
		EventCallback handler, void *eventObject)
{
	etrigArray = static_cast<Event*>(kmalloc(sizeof(Event) * 2));
	etrigCount = 1;
	holeCount = 0;
	bufferSize = 2;

	leftChild = rightChild = weak_null;
	parent = weak_null;
	color = kRed;

	new(etrigArray) Event(trigger,
			shiftAllowed, handler, eventObject);

	overlapRange[0] = trigger;
	overlapRange[1] = trigger + shiftAllowed;
}

/**
 * Constructs a trigger object into this node so that it can also be
 * executed along with other triggers in the node.
 *
 * @param trigger - timestamp for the trigger
 * @param shiftAllowed - delay after timestamp before which it is to
 * 		be executed
 * @param handler - callback which will execute the event
 * @param eventObject - optional parameter to the callback
 * @return - the construct EventTrigger object, shouldn't be modified
 */
Event* EventGroup::add(Timestamp trigger, Delay shiftAllowed,
		EventCallback handler, void *eventObject)
{
	Timestamp newRange[2] = { trigger, trigger + shiftAllowed };
	if(rematchRange(newRange) == -1)
		return (null);

	Event *freeSlot = findFreeSlot();

	DbgInt(freeSlot - etrigArray);

	new(freeSlot) Event(trigger,
			shiftAllowed, handler, eventObject);

	return (freeSlot);
}

/**
 * Invalidates the given trigger, assuming it is held in this node, and
 * clears its (@code live) state. The new-range of this node is again
 * calculated, and it is returned (not set as nodal-range), to check
 * against unwanted expansion of the nodal range, as it could be
 * participating in a queue.
 *
 * The owner queue of this sorter should then set the new range of this
 * node accordingly.
 *
 * @param trig - trigger object to delete
 */
void EventGroup::del(Event *trig, Timestamp (&impendingRange)[2])
{
	trig->live = 0;

	if(getIndexOf(trig) != etrigCount - 1)
		++(holeCount);
	else
		--(etrigCount);

	cleanCalculateRange(impendingRange);
}

/**
 * Checks whether the given range overlaps with the nodal range with a
 * 5 tick buffer.
 *
 * @param rangeStart
 * @param rangeEnd
 * @return
 */
bool EventGroup::isHoldable(Timestamp rangeStart, Timestamp rangeEnd)
{
	if(rangeEnd > overlapRange[0] &&
			rangeStart < overlapRange[1]) {
		if(rangeStart <= overlapRange[0]) {
			return (rangeEnd - overlapRange[0] > 5);
		} else if(rangeEnd <= overlapRange[1]){
			return (rangeEnd - rangeStart > 5);
		} else {
			return (overlapRange[1] - rangeStart > 5);
		}
	}

	return (false);
}

/**
 * Re-calculates the range for execution of this node by transversing
 * each trigger. This method should be called only when removing any
 * trigger or doing any operation that does not insert more
 * condition's on the node's range.
 */
void EventGroup::cleanCalculateRange(Timestamp (&range)[2])
{
	range[0] = etrigArray->triggerRange[0];
	range[1] = etrigArray->triggerRange[1];

	Event *eobj = etrigArray + 1;

	for(unsigned int index = 1; index < etrigCount; index++) {
		if(eobj->live) {
			if(range[0] < eobj->triggerRange[0])
				range[0] = eobj->triggerRange[0];

			if(range[1] > eobj->triggerRange[1])
				range[1] = eobj->triggerRange[1];
		}

		++(eobj);
	}
}

/**
 * Ensures that the range given overlaps with the node's overlapRange such
 * that the delay allowed is at least 5 ticks.
 *
 * @param newRange - the range of a trigger that may be executed along
 * 		with the other triggers in this node.
 * @return - 0 - if the range given was merged into the node's range;
 * 		-1 - if the range doesn't overlap with the nodes' range
 */
int EventGroup::rematchRange(Timestamp newRange[2])
{
	if(newRange[0] >= overlapRange[1] ||
			newRange[1] <= overlapRange[0])
		return (-1);

	if(newRange[0] <= overlapRange[0])
		newRange[0] = overlapRange[0];

	if(newRange[1] >= overlapRange[1])
		newRange[1] = overlapRange[1];

	if(newRange[1] - newRange[0] <= 5)
		return (-1);

	overlapRange[0] = newRange[0];
	overlapRange[1] = newRange[1];

	return (0);
}

void EventGroup::ensureBuffer(unsigned long requiredCapacity)
{
	if(requiredCapacity <= bufferSize)
		return;

	Event *oldArray = etrigArray;
	Event *newArray = (Event*)
			kralloc(etrigArray, requiredCapacity * sizeof(Event));

	if(oldArray == newArray)
		goto WriteChanges;

	Arrays::copyFast(oldArray, newArray, etrigCount * sizeof(Event));

	WriteChanges:
	etrigArray = newArray;
	bufferSize = requiredCapacity;
}

/**
 * Finds a free slot (.i.e. a hole in the array) to store a new
 * trigger object.
 *
 * @return - the slot which is free
 */
Event *EventGroup::findFreeSlot()
{
	if(holeCount > 0)
	{
		unsigned int index = 0;
		Event *possibleHole = etrigArray;

		while(index < etrigCount)
		{
			if(!possibleHole->live)
				return (possibleHole);
		}
	}

	ensureBuffer(etrigCount + 1);
	return (etrigArray + (etrigCount++));
}

