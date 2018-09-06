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

/**
 * Constructs the <tt>HardwareTimer</tt> with no pending events.
 */
HardwareTimer::HardwareTimer()
{
	enow = strong_null;
}

/**
 * Not implemented: Hot rip for timers whohoho (well, since timers
 * can't be removed in hardware why worry about destroying this)
 */
HardwareTimer::~HardwareTimer()
{
	// umm, could you rip out a timer device at runtime
	// - something nice to implement :)
}

void HardwareTimer::initPIT()
{
}

/**
 * Puts another event in-queue, to be executed. It may be added into
 * the <tt>activeEvents</tt> group, or to the pending
 * <tt>Timeline</tt>.
 *
 * @param trigger - Time at which the event should be triggered
 * @param shiftAllowed - Amount of delay possible for this event
 * @param handler - A callback function that will handle the timed event
 * @param eventObject - An optional argument to the callback handler
 * @return - An <code>EventTrigger</code> object pointer on success; otherwise,
 * 			<code>null</code> on an internal failure.
 */
Event *HardwareTimer::add(Timestamp trigger,
		Timestamp shiftAllowed, EventCallback handler,
		void *eventObject)
{
	if(enow == null) {
		enow = new EventGroup(trigger, shiftAllowed,
				handler, eventObject);
		fireAt(enow->overlapRange[0]);

		return (enow->etrigArray);
	} else if(enow->isHoldable(trigger,
					trigger + shiftAllowed)) {
		return enow->add(trigger, shiftAllowed,
				handler, eventObject);
	}

	if(trigger < enow->overlapRange[0]) {
		equeue.addAll(enow);

		enow = new EventGroup(trigger, shiftAllowed,
				handler, eventObject);

		if(!fireAt(enow->overlapRange[0]))
			DbgLine("Serious unhandled error - timer set failure");

		DbgLine("___REPLACING");
		return (enow->etrigArray);
	} else {
		return (equeue.add(trigger, shiftAllowed,
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
	Event *activeEvent = enow->etrigArray;
	int activeEventCount = enow->etrigCount;

	for(int eidx = 0; eidx < activeEventCount; eidx++) {
		if(activeEvent->isLive()) {
			activeEvent->execute();
		}
	}

	delete enow;
	enow = equeue.get();
}
