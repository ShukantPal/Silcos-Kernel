/**
 * @file TimerDevice.hpp
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
#ifndef EXECMGR_TIMER_DEVICE_HPP
#define EXECMGR_TIMER_DEVICE_HPP

#include "Time.hpp"
#include "Event.hpp"
#include <HardwareAbstraction/Processor.h>
#include <TYPE.h>

include_kobject(ExecMgr_TimerOperation) // rather stored in HardwareTimer.cpp

namespace Executable
{ 
namespace Timer
{

/**
 * Holds the set of properties possessed by a kernel-timer in a bitmap.
 */
struct TimerProperties
{
#define IS_MONOTONOUS	0x0000
#define IS_ADJUSTABLE	0x0001
#define IS_RESETABLE	0x0002
#define IS_STOPPABLE	0x0003
	int wiredProps;
	
	bool isMonotonous() const { return (wiredProps >> IS_MONOTONOUS & 1); }
	bool isAdjustable() const { return (wiredProps >> IS_ADJUSTABLE & 1); } 
	bool isResetable() const { return (wiredProps >> IS_RESETABLE & 1); }
	bool isStoppable() const { return (wiredProps >> IS_STOPPABLE & 1); }
};

/**
 * Holds an operation due to be processed by the timer-object, that can
 * possibly sent to another CPU.
 * 
 * Two types of operations are supported:
 * 1. INVOKE_AFTER - invoke the given callback after a given time, although
 *			the exact timestamp is given instead.
 * 
 * 2. CANCEL_EVENT - cancel the referred event
 */
struct TimerOperation : HAL::IPIRequest
{
public:
	int callGate;
#define INVOKE_AFTER	1
#define CANCEL_EVENT	2
	
	class TimerDevice& targetDevice;
	
	union {
		struct {
			Timestamp absoluteTrigger;
			Timestamp delayLimit;
			EventCallback eventClient;
			void *clientArg;
		};
		
		struct {
			Event *referredEvent;
		};
	};
	
	static TimerOperation *packageInvokeAfter(Timestamp absoluteTrigger,
			Timestamp delayLimit, EventCallback eventClient,
			void *clientArg, TimerDevice &targetDevice) {
		TimerOperation *package = new(::ExecMgr_TimerOperation)
				TimerOperation(INVOKE_AFTER, targetDevice);

		package->absoluteTrigger = absoluteTrigger;
		package->delayLimit = delayLimit;
		package->eventClient = eventClient;
		package->clientArg = clientArg;
		
		package->callbackThis = &handleOperation;
		return (package);
	}

	static TimerOperation *packageCancelEvent(Event *targetClient,
			TimerDevice &targetDevice) {
		TimerOperation *package = new(::ExecMgr_TimerOperation)
			TimerOperation(CANCEL_EVENT, targetDevice);
		
		package->referredEvent = targetClient;
		return (package);
	}
private:
	TimerOperation(int callGate, TimerDevice &target) :
	IPIRequest(HAL::INVOKE_OPERATION_THIS, sizeof(TimerOperation),
			ExecMgr_TimerOperation), targetDevice(target) {
		this->callGate = callGate;
	}
	
	static void handleOperation(IPIRequest *arg);// HardwareTimer.cpp
};

/**
 * Exposes a kernel-timer device with a generic counter and event-callback
 * mechanism.
 */
class TimerDevice
{
public:

	TimerUnit unit() { return (gCounter.unit); }
	Time getCounter() { return (gCounter.value); }
	const TimerProperties& getProperties() { return (props); }
	
	virtual void updateCounter() = 0;
	virtual bool resetCounter() = 0;
	virtual bool setCounter(Time newCounter) = 0;
	virtual bool stopCounter() = 0;
	
	virtual Event *notifyAfter(Time interval, Time delayLimit,
				EventCallback client, void *object) = 0;
protected:
	GenericTime gCounter;
	const TimerProperties& props;
	
	/* Timers can set this internal property to communicate the current
	   state of the object. */
	unsigned int invocationMode;
	unsigned int intId;
	Lockable externalInvocationLock;
	
/* Possible invokation modes */
#define TDIM_EXTERNAL		0x00A	// Object is idle, callers are external
#define TDIM_INTERRUPT		0x00B	// Object is invoking callbacks	
#define TDIM_LOCKED		0x00C	// Object is processing a request	
	
	TimerDevice(TimerProperties &implProps)
	: gCounter(), props(implProps), externalInvocationLock() {
		invocationMode = TDIM_EXTERNAL;
	}
	
	TimerDevice(TimerProperties &implProps, TimerUnit implUnit) 
	: gCounter(implUnit), props(implProps), externalInvocationLock() {
		invocationMode = TDIM_EXTERNAL;
	}
	
	unsigned int getInvocationMode() {
		return (invocationMode);
	}
	
	void setInvocationMode(int newMode) {
		if(newMode == TDIM_LOCKED)
			externalInvocationLock.lock();

		Atomic::xchg(newMode, &invocationMode);

		if(newMode == TDIM_EXTERNAL)
			externalInvocationLock.unlock();
	}
};

}// Timer
}// Executable

#endif/* Executable/Timer/TimerDevice.hpp */

