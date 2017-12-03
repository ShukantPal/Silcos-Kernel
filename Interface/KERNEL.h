/**
 * File: KERNEL.h
 *
 * Summary:
 * Kernel provides a programmatic environment for other object-files. All of
 * the related functions/objects/struct/classes are provided here to allow
 * including all utilities at once.
 *
 * Functions:
 * NextPowerOf2 - Used for getting the least power-of-2 greater than 'x'
 * HighestBitSet - Used for getting the index of the set most significant bit
 *
 * Copyright (C) 2017 - Shukant Pal 
 */
#ifndef KERNEL_H__
#define KERNEL_H__

#include <Debugging.h>
#include <TYPE.h>
#include <Util/Memory.h>

#define __ADM_CONFIG__

#ifdef NS_ADM
	#ifdef NS_ADM_MULTIBOOT
		import_asm ULONG admMultibootTableStart;
		import_asm ULONG admMultibootTableEnd;
	#endif
#endif

#define FLIP_BIT(N, B) (~((N) & (1 << B)) & ((N) | (1 << B))) // Flip Bth bit in N
#define FLAG_SET(flgContainer, flgOffset) ((flgContainer >> flgOffset) & 1)

#define ALIGN_MAX(no, align) (no % align) ? ((no / align) * (align + 1)) : no
#define ALIGN_MIN(no, align) (no % align) ? ((no / align) * align) : no

#define VADDR_TO_PADDR(virtualAddress) ((ULONG) virtualAddress - 0xC0000000)
#define PADDR_TO_VADDR(physicalAddress) ((ULONG) physicalAddress + 0xC0000000)

#define LESS(ctvName, vLimit) ((ctvName < vLimit) ? ctvName : vLimit)

#define KLOCAL_END while(TRUE){ asm volatile("nop;"); }

#ifdef ARCH_32

static inline unsigned long NextPowerOf2(
		unsigned long x
){
	--(x);
	x |= (x >> 1);
	x |= (x >> 2);
	x |= (x >> 4);
	x |= (x >> 8);
	x |= (x >> 16);
	return (++x);
}

static inline unsigned long HighestBitSet(
		unsigned long x
){
	ULONG highestBit = 0;
	while(x >>= 1)
		++highestBit;
	return (highestBit);
}

#else /* ARCH_64 */

static inline
ULONG NextPowerOf2(ULONG x){
	--(x);
	x |= (x >> 1);
	x |= (x >> 2);
	x |= (x >> 4);
	x |= (x >> 8);
	x |= (x >> 16);
	x |= (x >> 32);
	return (++x);
}

static inline
ULONG HighestBitSet(ULONG x){
	ULONG highestBit = 0;
	while(x >>= 1)
		++highestBit;
	return (highestBit);
}

#endif

void ASSERT(
	BOOL boolCondition,
	CHAR *errorMsg
);

#endif/* KERNEL.h */
