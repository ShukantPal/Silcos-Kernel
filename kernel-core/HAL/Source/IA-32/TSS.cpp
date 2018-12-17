/**
 * @file TSS.cpp
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
#include <IA32/Processor.h>
#include "../../../Interface/Utils/CtPrim.h"

using namespace HAL;

TSS SystemTSS;
import_asm void ExecuteLTR(void);

extern "C" void SetupTSS(ArchCpu *pinfo)
{
	TSS *pTSS = &(pinfo->kTSS);

	unsigned int tBase = (unsigned int) pTSS;
	unsigned int tSize = sizeof(TSS) - 1;

	SetGateOn(5, tBase, tSize, 0x89, 0, &(pinfo->GDT[0]));
	memsetf(pTSS, 0, sizeof(TSS));

	pTSS->CS = 0x8;
	pTSS->DS = 0x10;
	pTSS->SS_0 = 0x10;
	pTSS->ESP_0 = 0;
	pTSS->IOMAP_BASE = 104;

	ExecuteLTR();
}
