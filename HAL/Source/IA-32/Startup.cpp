/**
 * File: Startup.cpp
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
#include <ACPI/RSDP.h>
#include <ACPI/RSDT.h>
#include <ACPI/MADT.h>
#include <IA32/APIC.h>
#include <HAL/Processor.h>
#include <HAL/ProcessorTopology.hpp>
#include <Executable/RunqueueBalancer.hpp>
#include <KERNEL.h>

using namespace HAL;
using namespace Executable;

import_asm void BSPGrantPermit();

/**
 * Function: __init()
 *
 * Summary:
 * The (hardware abstraction layer) function initializes the system and sets
 * the new oballocNormaleUse to true (SetupBSP) for normal object allocation.
 *
 * Author: Shukant Pal
 */
decl_c void __init()
{
	ScanRsdp();
	SetupRsdt();
	MapAPIC();
	SetupBSP();

	InitTTable();
	RunqueueBalancer::init();

	SetupAPs();
	SetupTick();

	BSPGrantPermit();
}
