/**
 * @file GDT.cpp
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
#include <HardwareAbstraction/Processor.h>
#include <IA32/Processor.h>

using namespace HAL;

import_asm GDTEntry defaultBootGDT[3];/* This should contain a flat memory-model, and no TSS entry. (APBoot.asm) */
import_asm GDTPointer defaultBootGDTPointer;/* APBoot.asm */
import_asm int ExecuteLGDT(GDTPointer *);

/**
 * Function: SetGateOn
 *
 * Summary:
 * Writes the GDT entry at the given index and parameters.
 *
 * Args:
 * unsigned short gateNo - index of the GDT-entry
 * unsigned int base - base address of the segment
 * unsigned int limit - limiting address of the segment
 * unsigned char access - access-levels/security-parameter
 * unsigned char granularity - segment-addressing granularity
 * GDTEntry *pGDT - start of the GDT-table
 *
 * Author: Shukant Pal
 */
void SetGateOn(unsigned short gateNo, unsigned int segBase,
		unsigned int segLimit, unsigned char segAccess,
		unsigned char segGranularity, GDTEntry *pGDT)
{
	pGDT[gateNo].baseLow = (segBase & 0xffff);
	pGDT[gateNo].baseMiddle = (segBase >> 16) & 0xff;
	pGDT[gateNo].baseHigh = (segBase >> 24) & 0xff;
	pGDT[gateNo].limit = (segLimit & 0xffff);
	pGDT[gateNo].granularity = (segLimit >> 16) & 0x0f;
	pGDT[gateNo].granularity |= ((segGranularity << 4) & 0xf0);
	pGDT[gateNo].access = segAccess;
}

/* Part of processor initialization series. */
extern "C" void SetupGDT(ArchCpu *processorInfo)
{
	GDTEntry *pGDT = &(processorInfo->GDT[0]);
	GDTPointer *pGDTPointer = &(processorInfo->GDTR);

	pGDTPointer->Limit = (sizeof(GDTEntry) * 6) - 1;
	pGDTPointer->Base = (unsigned int) pGDT;

	SetGateOn(0, 0, 0, 0, 0, pGDT);
	SetGateOn(1, 0, 0xFFFFFFFF, 0x9A, 0xCF, pGDT);
	SetGateOn(2, 0, 0xFFFFFFFF, 0x92, 0xCF, pGDT);
	SetGateOn(3, 0, 0xFFFFFFFF, 0xFA, 0xCF, pGDT);
	SetGateOn(4, 0, 0xFFFFFFFF, 0xF2, 0xCF, pGDT);
	/* SetupTSS() will add the 6th entry */

	ExecuteLGDT(pGDTPointer);/* Load.asm */
}
