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
#include <Synch/Spinlock.h>
#include <Util/Memory.h>

#define __ADM_CONFIG__

#ifdef NS_ADM
	#ifdef NS_ADM_MULTIBOOT
		import_asm unsigned long admMultibootTableStart;
		import_asm unsigned long admMultibootTableEnd;
	#endif
#endif

#define FLIP_BIT(N, B) (~((N) & (1 << B)) & ((N) | (1 << B))) // Flip Bth bit in N
#define FLAG_SET(flgContainer, flgOffset) ((flgContainer >> flgOffset) & 1)

#define ALIGN_MAX(no, align) (no % align) ? ((no / align) * (align + 1)) : no
#define ALIGN_MIN(no, align) (no % align) ? ((no / align) * align) : no

#define VADDR_TO_PADDR(virtualAddress) ((unsigned long) virtualAddress - 0xC0000000)
#define PADDR_TO_VADDR(physicalAddress) ((unsigned long) physicalAddress + 0xC0000000)

#define LESS(ctvName, vLimit) ((ctvName < vLimit) ? ctvName : vLimit)

#define KLOCAL_END while(TRUE){ asm volatile("nop;"); }

#define Max(a, b) ((a > b) ? a : b)
#define Min(a, b) ((a < b) ? a : b)

#ifdef IA32
	#define __cli 		asm volatile("cli");
	#define __mfence 	asm volatile("mfence");
	#define __lfence 	asm volatile("lfence");
	#define __sfence 	asm volatile("sfence");
	#define __sti 		asm volatile("sti");

	#define no_intr_ret 	asm volatile("sti"); return

#endif

#ifdef ARCH_32

static inline unsigned long NextPowerOf2(unsigned long x)
{
	--(x);
	x |= (x >> 1);
	x |= (x >> 2);
	x |= (x >> 4);
	x |= (x >> 8);
	x |= (x >> 16);
	return (++x);
}

static inline unsigned long HighestBitSet(unsigned long x)
{
	unsigned long highestBit = 0;
	while(x >>= 1)
		++highestBit;
	return (highestBit);
}

#else /* ARCH_64 */

static inline
unsigned long NextPowerOf2(unsigned long x){
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
unsigned long HighestBitSet(unsigned long x){
	unsigned long highestBit = 0;
	while(x >>= 1)
		++highestBit;
	return (highestBit);
}

#endif

/****************************** Code with no interrupts intervening ********************************************/

#define __no_interrupts(fcode)	\
	__cli			\
	fcode 			\
	__sti			\

/*
 * This macro should be used only on the function which don't return
 * anything. If the function does then __sti won't execute -> bug.
 */
#define __no_interrupt_func(fcode)		\
{						\
	__cli					\
	fcode					\
	__sti					\
}

/*
 * This macro should be used on the function which return a named
 * variable or any constant. This ensures __sti is called before
 * returning.
 */
#define __ret_interrupt_func(fcode, ret)	\
{						\
	__cli					\
	fcode					\
	__sti					\
	return (ret);				\
}

/*
 * Use this macro when using __no_interrupt_return at the end. More semantic
 * than useful...
 */
#define __no_ret__interrupt_func(fcode)		\
{						\
	__cli					\
	fcode					\
}

/**
 * Use this macro for returning from __no_interrupt_func
 * or __ret_interrupt_func functions
 */
#define __no_interrupt_return(ret)		\
	__sti					\
	return (ret);

#define _no_interrupt_open (
#define _no_interrupt_close )

/****************************** elf visiblity of symbols ******************************************/

#define kxport __attribute__((visibility("default")))
#define kxhide __attribute__((visibility("hidden")))

void ASSERT(
	bool boolCondition,
	char *errorMsg
);

#endif/* KERNEL.h */
