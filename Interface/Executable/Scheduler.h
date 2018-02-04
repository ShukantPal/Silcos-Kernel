/**
 * File: Scheduler.h
 * Module: ExecutionManager (@kernel.silcos.excmgr)
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
#ifndef EXEC_SCHEDULER_H
#define EXEC_SCHEDULER_H

#include <HAL/Processor.h>
#include <Executable/Task.hpp>
#include <Executable/Thread.h>
#include <Synch/Spinlock.h>
#include <KERNEL.h>

extern Time XMilliTime;

export_asm void WakeupExpiredWaiters(HAL::Processor *cpu) kxhide;
export_asm void Schedule(HAL::Processor *cpu);

static inline Time getSystemTime()
{
	return (XMilliTime);
}

#endif
