/**
 * @file HardwareTimer.cpp
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
#include <Executable/Timer/HardwareTimer.hpp>

using namespace Executable::Timer;

HardwareTimer::HardwareTimer()
{
	activeTriggers = strong_null;
}

HardwareTimer::~HardwareTimer()
{
	// umm, could you rip out a timer device at runtime
	// - something nice to implement :)
}

/**
 * Adds another event to be triggered by this timer into the queue. It may
 * also be added to the cached <code>activeTriggers</code> node. The resulting
 * <code>EventTrigger</code> object is then returned, on success, otherwise if
 * the timer couldn't fire this event, <code>null</code> is returned.
 *
 * This internal method should be exposed by the implementation of the device
 * driver by <code>notifyAfter</code> methods. Take should be taken of
 * preventing the <code>intrAction</code> method to execute while new events
 * are being added to prevent data corruption.
 *
 * @param trigger - Time at which the event should be triggered
 * @param shiftAllowed - Amount of delay possible for this event
 * @param handler - A callback function that will handle the timed event
 * @param eventObject - An optional argument to the callback handler
 * @return - An <code>EventTrigger</code> object pointer on success; otherwise,
 * 			<code>null</code> on an internal failure.
 */
EventTrigger *HardwareTimer::add(Timestamp trigger,
		Timestamp shiftAllowed, EventCallback handler,
		void *eventObject)
{
	EventTrigger *softTimer;

	if(activeTriggers == null) {
		activeTriggers = new EventNode(trigger, shiftAllowed,
				handler, eventObject);
		softTimer = activeTriggers->etrigArray;
		fireAt(activeTriggers->overlapRange[0]);

		return (softTimer);
	} else if(activeTriggers->isHoldable(trigger,
					trigger + shiftAllowed)) {
		softTimer = activeTriggers->add(trigger, shiftAllowed,
				handler, eventObject);
		return (softTimer);
	}

	if(trigger < activeTriggers->overlapRange[0]) {
		pendingTriggers.addAll(activeTriggers);

		activeTriggers = new EventNode(trigger, shiftAllowed,
				handler, eventObject);

		if(!fireAt(activeTriggers->overlapRange[0]))
			DbgLine("Serious unhandled error - timer set failure");

		DbgLine("___REPLACING");
		return (activeTriggers->etrigArray);
	} else {
		DbgLine(" _____ HERE_____");
		return (pendingTriggers.add(trigger, shiftAllowed,
				handler, eventObject));
	}
}

/**
 * Executes all the currently active events and then recycles
 * them, i.e. fetches the next set of active events. The implementation
 * of the device driver should then set the timer to fire at the next
 * active node, or otherwise sleep if <code>activeTriggers</code> is
 * set to <code>null</code> (no more events are live).
 */
void HardwareTimer::retireActiveEvents()
{
	EventTrigger *activeEvent = activeTriggers->etrigArray;
	int activeEventCount = activeTriggers->etrigCount;

	for(int eidx = 0; eidx < activeEventCount; eidx++) {
		if(activeEvent->isLive()) {
			activeEvent->execute();
		}
	}

	activeTriggers = pendingTriggers.get();
}
