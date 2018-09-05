/**
 * @file PIT.cpp
 *
 * Implements the PIT driver, allowing low-accuracy event timing, in
 * order of milliseconds. The HAL can disable this, by turning off
 * the IOAPIC input sending PIT signals to the CPU. But if it shares
 * the same IRQ with another device, it cannot (possibly) be turned
 * off in a standard manner.
 *
 * This implementation provides precision to 5 milliseconds. The
 * errors in PIT timing are <b>random</b>, hence time-sensitive events
 * should not be given to this timer.
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
#include <Executable/Timer/PIT.hpp>
#include <Math.hpp>

#include <Module/Elf/ABI/Implementor.h>

using namespace Executable::Timer;

PIT Executable::Timer::pit;

PIT::PIT() // @suppress("Class members should be properly initialized")
{
}

PIT::~PIT()
{

}

/**
 * Checks for any PIT interrupts sent in its status register. If an
 * interrupt was sent, the PIT gets re-armed with the next event,
 * otherwise it is set to fire as far as possible.
 *
 * @return whether the PIT caused an interrupt
 */
bool PIT::intrAction()
{
	bool wasFired = status(0).outputState;
	if(!wasFired)
		return (false);

	unsigned long updatedTicks = getTimerBlock(0)->totalTicks
			+ getTimerBlock(0)->lastReadCount;

	if(activeTriggers == null ||
			activeTriggers->overlapRange[0] - updatedTicks > 65535) {
		arm(0);// Sadly, no soft-timers dependent here
		return (true);
	} else if(activeTriggers->overlapRange[0] > updatedTicks + 4) {
		/*
		 * If we have more than 4 microseconds left, we'll have to wait,
		 * but otherwise, the events are unlucky, they'll be called a few
		 * microseconds earlier - why, because the "ancient" PIT needs
		 * upto 3-4 microseconds to get its counter read or written.
		 */
		armAt(activeTriggers->overlapRange[0]);
		return (true);
	}

	retireActiveEvents();

	if(activeTriggers == null) {
		arm(0);
	} else {
		armAt(activeTriggers->overlapRange[0]);
	}

	return (true);
}

/**
 * Holds an event-trigger that will notify the caller after the given
 * amount of "ticks" pass by. The frequency of this timer is fixed, and
 * hence, other units of time can be converted using the (@code getTicks())
 * method.
 *
 * This method can induce +-1 tick error, as the event will be fired on
 * the closest tick after the given amount of time.
 *
 * @param npitTicks - time in pit-ticks, when the event will fire
 * @param delayFeasible - delay tolerable in firing event; at least 5
 * 				ticks are required
 * @param handler - callback to handle the event
 * @param eventObject - optional argument for the callback
 * @return - object holding data regarding the newly created event;
 * 			null, if the request failed.
 */
EventTrigger *PIT::notifyAfter(Timestamp npitTicks,
		Timestamp delayFeasible, EventCallback handler,
		void *eventObject)
{
	if(delayFeasible < 5)
		return (strong_null);

	EventTrigger *et;

	__cli // TODO: Instead of __cli check whether enough time is
		// available until next "tick". If not, then do __cli

	et = add(npitTicks + getTotalTicks(0), delayFeasible, handler,
			eventObject);

	__sti

	return (et);
}

/**
 * While the driver is calling event-handlers, the PIT is an undefined state,
 * as the counter is rolled over and is not reliable, hence, to renew the
 * event-triggers the handler must use (@code notifyAgain()) to access the
 * event-queue and add an event-trigger.
 *
 * If (@code notifyAfter()) is used in an event-handler, the actually trigger
 * will occur up to 60-70 milliseconds after the expected time. Hence, it is
 * the handler's responsibility to use the correct API.
 *
 * @param npitTicks - The no. of ticks after which the renewed event should
 * 					occur.
 * @param delayFeasible - The amount of delay allowed for the renewed event,
 * 					which should be at least 5-6 microseconds.
 * @param handler - The new-handler for the event - or it may be same as the
 * 					current handler.
 * @param eventObject - The (optional) argument to the handler for the new
 * 					event.
 * @return - An event-trigger object representing the soft-timer/event just
 * 			created; null, if the event couldn't be created for some unknown
 * 			reason.
 */
EventTrigger *PIT::notifyAgain(Timestamp npitTicks,
		Timestamp delayFeasible, EventCallback handler,
		void *eventObject)
{
	if(delayFeasible < 5)
		return (strong_null);

	return (pendingTriggers.add(npitTicks + getTimerBlock(0)->totalTicks +
			getTimerBlock(0)->lastReadCount,
			delayFeasible, handler, eventObject));
}

void PIT::reset(unsigned short newInitialCount, unsigned char selectCounter)
{

	Counter *newTimer = progTimers + selectCounter;
	newTimer->totalTicks = 0;
	newTimer->initialCount = newInitialCount;
	newTimer->mode = INTERRUPT_ON_TERMINAL_COUNT;
	newTimer->lastReadCount = newInitialCount;
	flush(selectCounter);
}

bool PIT::fireAt(Timestamp fts)
{
	Timestamp ttp = getTotalTicks(0);

	if(fts < ttp) {
		return (false);
	} else {
		if(fts - ttp > 0xFFFF)
			reset(0xFFFF, 0);
		else
			reset(fts - ttp, 0);
	}

	return (true);
}

/**
 * Arms the counter with 65535 count.
 *
 * @param selectCounter - The counter that is to be re-programmed in the PIT.
 * @version 1.0
 * @since Silcos 3.05
 * @author Shukant Pal
 */
void PIT::arm(unsigned selectCounter)
{
	Counter *oldTimer = progTimers + selectCounter;

	oldTimer->totalTicks += oldTimer->lastReadCount;
	oldTimer->mode = INTERRUPT_ON_TERMINAL_COUNT;
	oldTimer->initialCount = 65535;
	oldTimer->lastReadCount = oldTimer->initialCount;

	flush(selectCounter);
}

/**
 * Arms channel 0 to fire at the given timestamp, assuming that it is
 * less than 0xFFFF ticks later than now, and, in addition, that at least
 * more than 4-5 ticks are remaining. This is due to the latency induced
 * while arming the timer, which causes considerable delay.
 *
 * @param nextFire
 */
void PIT::armAt(Timestamp nextFire)
{
	Counter *ch0 = getTimerBlock(0);
	ch0->totalTicks += ch0->lastReadCount;
	ch0->mode = INTERRUPT_ON_TERMINAL_COUNT;
	ch0->initialCount = nextFire - ch0->totalTicks;
	ch0->lastReadCount = ch0->initialCount;
	flush(0);
}

/**
 * Flushes the contents of the counter's programming struct in memory so
 * that it is implemented by the hardware.
 *
 * @param index - The index of the counter to be programmed.
 */
void PIT::flush(unsigned long index)
{
	ControlWord resetCommand;
	resetCommand.mode = progTimers[index].mode;
	resetCommand.selectCounter = index;

	unsigned short initialCount = progTimers[index].initialCount;

	resetCommand.rwAccess = BothBytes;
	WritePort(CMD_REG, resetCommand);
	WritePort(getPITChannelPort(index), initialCount);
	WritePort(getPITChannelPort(index), initialCount >> 8);
}
