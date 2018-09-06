/**
 * @file HPET.cpp
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
#include <ACPI/RSDT.h>
#include <ACPI/HPET.h>
#include <Executable/Timer/HPET.hpp>

using namespace Executable;
using namespace Executable::Timer;

/**
 * Constructs a kernel-only HPET driver object, that can also be used
 * during the booting process. If the ACPI table "HPET" doesn't exist,
 * then the HPET cannot be used during boot.
 */
decl_c void InitKernelHPET()
{
	ACPI::HPET *ahdt = (ACPI::HPET *) SearchACPITableByName("HPET", null);

	if(ahdt == null) {
		DbgLine("Warning: No HPET found on IA-PC system!");
		return;
	}

	HPET::kernelTimer = new HPET(
			ahdt->timerNumber, ahdt->baseAddress.addressValue);

	HPET::Timer *intern = HPET::wallTimer();
	int input = Math::bitScanReverse(intern->allRoutes());

	intern->connectTo(input);
}
