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
#include <HardwareAbstraction/IOAPIC.hpp>
#include <HardwareAbstraction/ProcessorTopology.hpp>
#include <Memory/KMemoryManager.h>
#include <Memory/KObjectManager.h>
#include <Memory/Pager.h>

using namespace HAL;

/**
 * All IOAPIC <tt>Input</tt> objects are kept in a static
 * <tt>ArrayList</tt> so that they can be accessed using
 * their global input index.
 */
ArrayList IOAPIC::systemIOAPICInputs(24);

ArrayList IOAPIC::systemIOAPICs;

unsigned long IOAPIC::routesUnderReset;

unsigned long IOAPIC::lowestID = 0xF;

/**
 * Constructs a new <tt>IOAPIC</tt> driver, that controls the chipset
 * mapped at <tt>regBase</tt> physical address. The <tt>IOAPIC::Input
 * </tt> objects are not added in this process.
 *
 * @version 1.0
 * @since Silcos 3.02
 * @author Shukant Pal
 */
IOAPIC::IOAPIC(unsigned long regBase, unsigned long intrBase)
{
	this->physRegs = regBase;
	unsigned long mmioPage = KiPagesAllocate(0, ZONE_KMODULE, ATOMIC);
	Pager::map(mmioPage, physRegs & 0xFFFFF000, 0,
			KernelData | PageCacheDisable);

	this->virtAddr = mmioPage + (physRegs % KPGSIZE);
	this->apicID = (read(IOAPICID) >> 24);
	this->hardwareVersion = read(IOAPICVER);
	this->redirEntries = 1 + (read(IOAPICVER) >> 16);
	this->arbId = read(IOAPICARB) >> 4;
	this->globalSystemInterruptBase = intrBase;
}

/**
 * Returns the redirection entry for the given input in this
 * IOAPIC, so that the user can modify-write it later.
 *
 * @param input - index of the redirection entry; must be less than
 * 				<tt>redirectionEntries()</tt>
 */
IOAPIC::RedirectionEntry IOAPIC::readRedirection(unsigned char input)
{
	union { U32 rout[2]; RedirectionEntry re; };

	if(input < redirectionEntries()) {
		rout[0] = read(IOREDTBL(input));
		rout[1] = read(IOREDTBL(input) + 1);
	} else {
		rout[0] = rout[1] = 0;
	}

	return (re);
}

/**
 * Finalizes the redirection entry for the given input, making it
 * effectively implemented. The target local-irq is automatically
 * updated on changing it.
 *
 * @param input - signal for which entry is being modified
 * @param ent - modified redirection entry to write
 */
void IOAPIC::writeRedirection(unsigned inputSignal,
				IOAPIC::RedirectionEntry *ent)
{
	if(inputSignal < redirectionEntries()) {
		write(IOREDTBL(inputSignal), *(unsigned int*) ent);
		write(IOREDTBL(inputSignal) + 1, *((unsigned int*) ent + 1));
	}

	inputAt(inputSignal)->connectTo(ent->localVector, ent->destination);
}

/**
 * Convenience method that allows the user to do a read-modify-write
 * operation using his callback method, which inputs & then outputs
 * a <tt>IOAPIC::RedirectionEntry</tt>.
 *
 * @param input - local index of the IOAPIC input pin
 * @param callback - read-modify-write functor
 */
void IOAPIC::writeRedirection(unsigned input,
		RedirectionEntry (*callback)(RedirectionEntry))
{
	RedirectionEntry target = readRedirection(input);
	target = callback(target);
	writeRedirection(input, &target);
}

/**
 * Constructs a new <tt>IOAPIC</tt> driver, for the given IOAPIC
 * found in ACPI tables; initializes the <tt>IOAPIC::Input</tt>
 * drivers as well.
 *
 * @param ioaEnt - IOAPIC entry in the ACPI MADT table
 */
void IOAPIC::registerIOAPIC(MADTEntryIOAPIC *ioaEnt)
{
	IOAPIC *ioa = new IOAPIC(ioaEnt->apicBase, ioaEnt->GSIB);

	if(ioa->id() != ioaEnt->apicID) {
		DbgLine("Invalid IO/APIC entry found in ACPI 2.0 tables!");
		delete ioa;
	} else {
		if(ioa->id() < lowestID)
			lowestID = ioa->id();

		systemIOAPICs.set(ioa, ioaEnt->apicID);

		for(unsigned long globlIdx = ioaEnt->GSIB;
				globlIdx < (ioaEnt->GSIB + ioa->intrCount());
				globlIdx++) {
			systemIOAPICInputs.set(new IOAPIC::Input(globlIdx, ioa),
					globlIdx);
		}
	}
}

IOAPIC::Input *IOAPIC::getOptimizedInput(Executable::IRQHandler *dev)
{
	return (null);// not implemented yet!
}

/**
 * Constructs a new IOAPIC input with no device linked for
 * interrupts.
 */
IOAPIC::Input::Input(unsigned globalIndex, IOAPIC *router)
{
	this->mGlobalIndex = globalIndex;
	this->locked = false;
	this->hub = router;
}

/**
 * Registers the irq-handler connected to this IOAPIC input. The irq
 * handler may execute on any processor, which is connected to this
 * IOAPIC input.
 *
 * It is expected that the device will be programmed to generate
 * interrupts after the irq-handler is registered, so no interrupts
 * go unhandled.
 *
 * @param handler - the irq-handler for the device
 * @param lock - whether sharing this irq is feasible for this device
 * @return - an index used to perform operations with the handler w.r.t this
 * 		input; INVALID_INDEX, if the device wasn't added to this input
 */
int IOAPIC::Input::addDev(IRQHandler *handler, bool lock)
{
	if(this->locked ||
			(lock && lineHdlrs.count() > 0)) {
		return (INVALID_INDEX);
	}

	return (lineHdlrs.add(handler));
}

/**
 * Connects this IOAPIC input to the local-irq, at the given vector
 * & local APIC.
 *
 * @param lvector - the IDT vector at which the IOAPIC handler should
 * 					execute, when an interrupt is recieved
 * @param lapicId - the id of the local APIC of the CPU which should
 * 					handle interrupts from this IOAPIC input
 */
void IOAPIC::Input::connectTo(unsigned lvector, unsigned lapicId)
{
	// TODO: Remove already registered handler

	LocalIRQ *lirq = GetIRQTableById(lapicId) + lvector - 32;
	lirq->addHandler(this);
}
