/**
 * @file RSDP.cpp
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
#include <Memory/Pager.h>
#include <KERNEL.h>
#include "../../../Interface/Utils/Memory.h"

RSDP *SystemRsdp = NULL;
RSDP2 *SystemRsdp2 = NULL;

RSDP *ScanRsdp()
{
	Pager::mapHuge(stcConfigBlock << 12, 0,
			KF_NOINTR | FLG_NOCACHE, KernelData);

	unsigned char *scanAddress = (unsigned char *)(stcConfigBlock << 12);
	unsigned char *endSearch = scanAddress + MB(2);
	char *RsdpString = RsdpSignature;

	DbgLine("Scanning Configuration...");

	while(scanAddress < endSearch) {
		if(memcmp(scanAddress, RsdpString, 8)) {
			if(VerifyRsdpChecksum((RSDP *) scanAddress)) {
				SystemRsdp = (RSDP *) scanAddress;
				break;
			}
		}

		scanAddress += 16;
	}

	if(!SystemRsdp) {
		DbgLine("System Hardware Failure : ");
		DbgLine("Details : Root System Description Pointer not found. ");
		DbgLine("Warning - Severe.");
		asm volatile("hlt;");
	}

	stcConfigBlock += 512;

	return (SystemRsdp);
}
