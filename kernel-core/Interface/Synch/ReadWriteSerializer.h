///
/// @file ReadWriteSerializer.h
/// -------------------------------------------------------------------
/// This program is free software: you can redistribute it and/or modify
/// it under the terms of the GNU General Public License as published by
/// the Free Software Foundation, either version 3 of the License, or
/// (at your option) any later version.
///
/// This program is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
/// GNU General Public License for more details.
///
/// You should have received a copy of the GNU General Public License
/// along with this program.  If not, see <http://www.gnu.org/licenses/>
///
/// Copyright (C) 2017 - Shukant Pal
///

#ifndef SYNCH__READ_WRITE_SERIALIZER_H__
#define SYNCH__READ_WRITE_SERIALIZER_H__

#include "Spinlock.h"
#include <Atomic.hpp>
#include <KERNEL.h>

///
/// Allows multiple read-accesses concurrently, while exclusively
/// locking only when software is modifying it. Until all readers exit
/// the critical section, no writer will be allowed to enter.
///
/// This lock serializes all reads/writes too - if a reader comes after
/// a writer is waiting for its turn, that reader itself will wait. That
/// means it won't miss anything :-)
///
/// This type of lock is available in non-preemptive environments and is
/// optimized when writers are minimal.
///
/// @version 1.0
/// @since Silcos 3.02
/// @author Shukant Pal
///
struct ReadWriteSerializer
{
	unsigned long onlineReaders;
	Spinlock criticalSection;

	ReadWriteSerializer()
	{
		(void) Atomic::xchg(0, &onlineReaders);
		(void) Atomic::xchg(0, (unsigned long*) &criticalSection);
	}

	~ReadWriteSerializer()
	{
		while(onlineReaders)
			asm volatile("nop");
	}

	void enterAsReader()
	{
		SpinLock(&criticalSection);
		Atomic::inc(&onlineReaders);
		SpinUnlock(&criticalSection);
	}

	void enterAsWriter()
	{
		SpinLock(&criticalSection);
		while(onlineReaders)
			asm volatile("nop");
	}

	void exitAsReader()
	{
		Atomic::dec(&onlineReaders);
	}

	void exitAsWriter()
	{
		SpinUnlock(&criticalSection);
	}
};

#endif/* Synch/ReadWriteSerializer.h */
