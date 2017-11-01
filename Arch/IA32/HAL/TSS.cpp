/**
 * Copyright (C) 2017 - Shukant Pal
 */

#include <IA32/Processor.h>
#include <Util/CtPrim.h>

TSS SystemTSS;
import_asm void ExecuteLTR(VOID);

decl_c VOID SetupTSS(PROCESSOR_INFO *processorInfo){
	TSS *pTSS = &(processorInfo->TSS);

	UINT tBase = (UINT) pTSS;
	UINT tSize = sizeof(TSS) - 1;

	SetGateOn(5, tBase, tSize, 0x89, 0, &(processorInfo->GDT[0]));
	memsetf(pTSS, 0, sizeof(TSS));

	pTSS->CS = 0x8;
	pTSS->DS = 0x10;
	pTSS->SS_0 = 0x10;
	pTSS->ESP_0 = 0;
	pTSS->IOMAP_BASE = 104;

	ExecuteLTR();
}
