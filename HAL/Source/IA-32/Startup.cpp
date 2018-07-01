///
/// @file Startup.cpp
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

#include <ACPI/RSDP.h>
#include <ACPI/RSDT.h>
#include <ACPI/MADT.h>
#include <ACPI/HPET.h>
#include <IA32/APIC.h>
#include <Executable/RunqueueBalancer.hpp>
#include <HardwareAbstraction/IOAPIC.hpp>
#include <HardwareAbstraction/Processor.h>
#include <HardwareAbstraction/ProcessorTopology.hpp>
#include <KERNEL.h>
#include <Math.hpp>

using namespace HAL;
using namespace Executable;

decl_c void ArchMain()
{
	ScanRsdp();
	SetupRSDTHolder();
	MapAPIC();
	SetupBSP();

	InitTTable();
	RunqueueBalancer::init();

	SetupAPs();
	APIC::setupScheduleTicks();
}
