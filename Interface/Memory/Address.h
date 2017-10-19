/**
 * Copyright (C) 2017 - Shukant Pal
 */

#ifndef MEMORY_ADDRESS_H
#define MEMORY_ADDRESS_H

#include <Types.h>
#include <Util/LinkedList.h>

#ifdef x86
	#include <IA32/PageTrans.h>

	#define ObjectInit (VIRTUAL_T) (GB(3) + MB(200))
	#define MaxObjectPages (VIRTUAL_T) (MB(512) / KB(4))
	#define MinObjectPages (VIRTUAL_T) 128

	#define DYNAMIC_MEMORY (VIRTUAL_T) (GB(3) + MB(200))
	#define DYNAMIC_MEMORY_SIZE (KB(128))

	#define MX_DYNAMIC_MEMORY MB(512)
	#define MN_DYNAMIC_MEMORY KB(512)
#endif

/* TLB management helper functions */
#ifdef x86

static inline void FlushTLB(ULONG addr) {
	asm volatile("invlpg (%0)" :: "r" (addr) : "memory");
}

#endif

/* Address conversion */
#define GB(n) ((ULONG) n*1024*1024*1024)
#define MB(n) ((ULONG) n*1024*1024)
#define KB(n) ((ULONG) n*1024)

/* Pointer conversion */
#define PtrMB(n) ((ULONG*) ((ULONG) n*1024*1024))
#define PtrKB(n) ((ULONG*) ((ULONG n*1024))
#define PtrMx(n,m) ((ULONG*) (MB(n) + KB(m)))
#define PtrBlk(n) ((ULONG*) n*4096)

#endif /* Memory/Address.h */
