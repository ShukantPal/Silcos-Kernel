/**
 * @file IOAPIC.cpp
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
#include <HAL/IOAPIC.hpp>
#include <Memory/KMemoryManager.h>
#include <Memory/KObjectManager.h>
#include <Memory/Pager.h>

ObjectInfo *tp_IOAPIC;

using namespace HAL;

/* @field ioApicChain
 *
 * Sorted linked-list of registered I/O apics. Automated interrupt mapping
 * can be implemented using this (experimental).
 */
LinkedList IOAPIC::ioApicChain;

/* @constructor IOAPIC::IOAPIC
 *
 * Allocates a register mapping in kernel-space to use the IOAPIC. It also
 * copies read-only data from the IOAPIC registers locally to save access
 * time.
 *
 * @version 1.0
 * @since Silcos 3.02
 * @author Shukant Pal
 */
IOAPIC::IOAPIC(unsigned long regBase)
{
	this->physRegs = regBase;

	unsigned long mmioPage = KiPagesAllocate(0, ZONE_KMODULE, ATOMIC);
	EnsureMapping(mmioPage, physRegs & 0xFFFFF000, null,
					0, KernelData | PageCacheDisable);

	this->virtAddr = mmioPage + (physRegs % KPGSIZE);
	this->apicID = read(IOAPICID) >> 4;
	this->hardwareVersion = read(IOAPICVER);
	this->redirEntries = 1 + (read(IOAPICVER) >> 16);
	this->arbId = read(IOAPICARB) >> 4;
}

/*
 * Copies the redirection-entry locally for the given input-signal. Care should
 * be taken to avoid requesting entries that don't exist using -
 *
 * 			inputSignal < redirectionEntries()
 *
 * @param inputSignal - signal for which the redirection entry is requested
 * @version 1.0
 * @since Silcos 3.02
 * @author Shukant Pal
 */
IOAPIC::RedirectionEntry IOAPIC::getRedirEnt(unsigned char inputSignal)
{
	union { U32 rout[2]; RedirectionEntry re; };

	if(inputSignal < redirectionEntries())
	{
		rout[0] = read(IOREDTBL(inputSignal));
		rout[1] = read(IOREDTBL(inputSignal) + 1);
	}
	else
	{
		rout[0] = rout[1] = 0;
	}

	return (re);
}

/*
 * Writes the redirection-entry given for the given input signal. It should be
 * done after copying the redirection entry - performing a read-modify-write
 * operation. Care should be taken to avoid requesting writes for entries that
 * don't exist by using -
 *
 * 			inputSignal < redirectionEntries()
 *
 * @param inputSignal - signal for which entry is being modified
 * @param ent - modified redirection entry to write
 * @version 1.0
 * @since Silcos 3.02
 * @author Shukant Pal
 */
void IOAPIC::setRedirEnt(unsigned char inputSignal,
				IOAPIC::RedirectionEntry *ent)
{
	if(inputSignal < redirectionEntries())
	{
		write(IOREDTBL(inputSignal), *(unsigned int*) ent);
		write(IOREDTBL(inputSignal) + 1, *((unsigned int*) ent + 1));
	}
	else
		DbgLine("EXC");
}

/*
 * Registers the I/O APIC into a system-chain from an I/O apic entry in the
 * APIC 2.0 multiple apic descriptor table. It will link it into a system-wide
 * chain in a sorted order.
 *
 * @param ioaEnt - i/o apic entry of acpi 2.0 MADT
 * @version 1.0
 * @since Silcos 3.02
 * @author Shukant Pal
 */
void IOAPIC::registerIOAPIC(MADTEntryIOAPIC *ioaEnt)
{
	IOAPIC *ioa = new(tp_IOAPIC) IOAPIC(ioaEnt->apicBase);

	if(ioa->id() != ioaEnt->apicID)
	{
		DbgLine("Invalid IO/APIC entry found in ACPI 2.0 tables!");
		ioa->~IOAPIC();
		kobj_free((kobj*) ioa, tp_IOAPIC);
	}
	else
	{
		AddElement(ioa, &ioApicChain);
	}
}
