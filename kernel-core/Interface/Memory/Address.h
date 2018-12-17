/**
 * Copyright (C) 2017 - Shukant Pal
 */

#ifndef MEMORY_ADDRESS_H
#define MEMORY_ADDRESS_H

#include <Types.h>
#include "../Utils/LinkedList.h"

#ifdef x86
	#include <IA32/PageTrans.h>

	#define ObjectInit (ADDRESS) (GB(3) + MB(200))
	#define MaxObjectPages (ADDRESS) (MB(512) / KB(4))
	#define MinObjectPages (ADDRESS) 128

	#define DYNAMIC_MEMORY (ADDRESS) (GB(3) + MB(200))
	#define DYNAMIC_MEMORY_SIZE (KB(128))

	#define MX_DYNAMIC_MEMORY MB(512)
	#define MN_DYNAMIC_MEMORY KB(512)
#endif

/* TLB management helper functions */
#ifdef x86

static inline void FlushTLB(unsigned long addr) {
	asm volatile("invlpg (%0)" :: "r" (addr) : "memory");
}

#endif

/* Address conversion */
#define GB(n) ((unsigned long) n*1024*1024*1024)
#define MB(n) ((unsigned long) n*1024*1024)
#define KB(n) ((unsigned long) n*1024)

/* Pointer conversion */
#define PtrMB(n) ((unsigned long*) ((unsigned long) n*1024*1024))
#define PtrKB(n) ((unsigned long*) ((unsigned long n*1024))
#define PtrMx(n,m) ((unsigned long*) (MB(n) + KB(m)))
#define PtrBlk(n) ((unsigned long*) n*4096)

#endif /* Memory/Address.h */
