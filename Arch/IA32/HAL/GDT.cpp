/*=++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * File: GDT.c
 *
 * Summary:  This file contains the code for setting up the GDT, and is part of the CPU-initialization
 * series. It also contains the defaultBootGDTPointer and defaultBootGDT, which is used for early GDT
 * to enter protected mode for the APs.
 *
 * Copyright (C) 2017 - Shukant Pal
 *+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=*/

#include <IA32/Processor.h>

import_asm GDT defaultBootGDT[3]; /* This should contain a flat memory-model, and no TSS entry. (APBoot.asm) */
import_asm GDT_POINTER defaultBootGDTPointer;/* APBoot.asm */
import_asm void ExecuteLGDT(GDT_POINTER *);

VOID SetGateOn(USHORT gateNo, UINT segBase, UINT segLimit, UCHAR segAccess, UCHAR segGranularity, GDT *pGDT){
	pGDT[gateNo].BaseLow = (segBase & 0xffff);
	pGDT[gateNo].BaseMiddle = (segBase >> 16) & 0xff;
	pGDT[gateNo].BaseHigh = (segBase >> 24) & 0xff;
	pGDT[gateNo].Limit = (segLimit & 0xffff);
	pGDT[gateNo].Granularity = (segLimit >> 16) & 0x0f;
	pGDT[gateNo].Granularity |= ((segGranularity << 4) & 0xf0);
	pGDT[gateNo].Access = segAccess;
}

/* Part of processor initialization series. */
decl_c void SetupGDT(PROCESSOR_INFO *processorInfo){
	GDT *pGDT = &(processorInfo->GDT[0]);
	GDT_POINTER *pGDTPointer = &(processorInfo->GDTPointer);

	pGDTPointer->Limit = (sizeof(GDT_ENTRY) * 6) - 1;
	pGDTPointer->Base = (UINT) pGDT;

	SetGateOn(0, 0, 0, 0, 0, pGDT);
	SetGateOn(1, 0, 0xffffffff, 0x9a, 0xcf, pGDT);
	SetGateOn(2, 0, 0xffffffff, 0x92, 0xcf, pGDT);
	SetGateOn(3, 0, 0xffffffff, 0xfa, 0xcf, pGDT);
	SetGateOn(4, 0, 0xffffffff, 0xf2, 0xcf, pGDT);
	/* SetupTSS() will add the 6th entry */

	ExecuteLGDT(pGDTPointer);/* Load.asm */
}
