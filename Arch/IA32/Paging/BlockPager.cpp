/**
 * Copyright(C) 2017 - Shukant Pal
 */

#define NS_KFRAMEMANAGER
#define NAMESPACE_PHYSICAL_MEMORY

#include <Memory/Address.h>
#include <Memory/Pager.h>
#include <Memory/KMemorySpace.h>
#include <Multiboot.h>
#include <Util/Memory.h>
#include <KERNEL.h>

extern PADDRESS mmTotal;

/* x86-specific paging structures */
import_asm U64 PDPT[4];
import_asm U64 GlobalDirectory[512];
import_asm U64 IdentityDirectory[512];
import_asm U64 GlobalTable[512];

extern ULONG memFrameTableSize;

VOID MTMap2mb(PHYSICAL_T Address, VIRTUAL_T Vad){
	GlobalDirectory[(Vad % GB(1)) / MB(2)] = Address | (1 << 7) | KernelData;
	FlushTLB(Vad);
}

PADDRESS KiMapTables(){
	PADDRESS frameMapper = MB(16);
	ULONG framePtr = KFRAMEMAP;
	ULONG frameTableEnd = KFRAMEMAP + sizeof(MMFRAME) * (mmTotal >> 12);
	while(framePtr < frameTableEnd) {
		MTMap2mb(frameMapper, framePtr);
		frameMapper += MB(2);
		framePtr += MB(2);
	}
	memFrameTableSize = frameTableEnd - KFRAMEMAP;
	memsetf((VOID*) KFRAMEMAP, 0, memFrameTableSize);
	return (MB(16));
}

