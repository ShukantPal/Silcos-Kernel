/**
 * Copyright(C) 2017 - Shukant Pal
 */

#define NS_KFRAMEMANAGER
#define NAMESPACE_PHYSICAL_MEMORY

#include <Memory/Address.h>
#include <Memory/Pager.h>
#include <Memory/KMemorySpace.h>
#include <Util/Memory.h>
#include <KERNEL.h>

extern PADDRESS mmTotal;

/* x86-specific paging structures */
extern "C" U64 PDPT[4];
extern "C" U64 GlobalDirectory[512];
extern "C" U64 IdentityDirectory[512];
extern "C" U64 GlobalTable[512];
extern unsigned long memFrameTableSize;

void MTMap2mb(PADDRESS paddr, ADDRESS vaddr)
{
	GlobalDirectory[(vaddr % GB(1)) / MB(2)] = paddr | (1 << 7) | KernelData;
	FlushTLB(vaddr);
}

PADDRESS KiMapTables()
{
	PADDRESS frameMapper = MB(16);
	unsigned long framePtr = KFRAMEMAP;
	unsigned long frameTableEnd = KFRAMEMAP + sizeof(MMFRAME) * (mmTotal >> 12);

	while(framePtr < frameTableEnd)
	{
		MTMap2mb(frameMapper, framePtr);
		frameMapper += MB(2);
		framePtr += MB(2);
	}

	memFrameTableSize = frameTableEnd - KFRAMEMAP;
	memsetf((void*) KFRAMEMAP, 0, memFrameTableSize);

	return (MB(16));
}

