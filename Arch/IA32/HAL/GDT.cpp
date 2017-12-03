/** Public Domain (no guarantee provided) */
#include <IA32/Processor.h>
#include <HAL/Processor.h>

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
void SetGateOn(
		unsigned short gateNo,
		unsigned int segBase,
		unsigned int segLimit,
		unsigned char segAccess,
		unsigned char segGranularity,
		GDTEntry *pGDT
){
	pGDT[gateNo].baseLow = (segBase & 0xffff);
	pGDT[gateNo].baseMiddle = (segBase >> 16) & 0xff;
	pGDT[gateNo].baseHigh = (segBase >> 24) & 0xff;
	pGDT[gateNo].limit = (segLimit & 0xffff);
	pGDT[gateNo].granularity = (segLimit >> 16) & 0x0f;
	pGDT[gateNo].granularity |= ((segGranularity << 4) & 0xf0);
	pGDT[gateNo].access = segAccess;
}

/* Part of processor initialization series. */
decl_c void SetupGDT(
		ProcessorInfo *processorInfo
){
	GDTEntry *pGDT = &(processorInfo->GDT[0]);
	GDTPointer *pGDTPointer = &(processorInfo->GDTR);

	pGDTPointer->Limit = (sizeof(GDTEntry) * 6) - 1;
	pGDTPointer->Base = (UINT) pGDT;

	DbgInt((ULONG) pGDT); Dbg(" ");

	SetGateOn(0, 0, 0, 0, 0, pGDT);
	SetGateOn(1, 0, 0xffffffff, 0x9a, 0xcf, pGDT);
	SetGateOn(2, 0, 0xffffffff, 0x92, 0xcf, pGDT);
	SetGateOn(3, 0, 0xffffffff, 0xfa, 0xcf, pGDT);
	SetGateOn(4, 0, 0xffffffff, 0xf2, 0xcf, pGDT);
	/* SetupTSS() will add the 6th entry */

	ExecuteLGDT(pGDTPointer);/* Load.asm */
}
