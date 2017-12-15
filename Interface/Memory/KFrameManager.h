/*=++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * File: KFrameManager.h
 *
 * Summary: This file provides the interface for allocating and deallocating pageframes from the
 * system. It provides multiple zones to allocate pageframes - ZONE_DMA, ZONE_NORMAL, ZONE_CODE,
 * ZONE_DATA, ZONE_KERNEL. Each zone has a particular purpose to allocate from, and thus, they
 * decrease synchronization overhead on SMP systems.
 *
 * The file also provides system memory information, and also some debugging features for the
 * interface.
 *
 * Functions:
 * 
 * KeFrameAllocate() - This function is used to allocate a pageframe of a given order.
 *
 * KeFrameFree() - This function returns the pageframe to the system
 *
 * KiFrameAllocate(), KiFrameFree() - Used for atomic, system memory allocated
 *
 * KiFrameEntrap() - Used for non-caching allocation of pageframes (to bypass the
 * PROCESSOR table)
 *
 * Copyright (C) 2017 - Shukant Pal
 *+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=*/
#ifndef MEMORY_KFRAME_MANAGER_H
#define MEMORY_KFRAME_MANAGER_H

#include "BuddyManager.h"
#include "KMemorySpace.h"
#include "ZoneManager.h"

#ifdef x86
	#include <IA32/PageTrans.h>
#endif

extern PADDRESS mmLow;
extern PADDRESS mmHigh;
extern PADDRESS mmTotal;
extern PADDRESS mmUsable;
extern unsigned long pgTotal;
extern unsigned long pgUsable;

#ifdef NS_KFRAMEMANAGER
#include "BuddyManager.h"

#define FRSIZE_ORDER 12

typedef struct Memory::Internal::BuddyBlock MMFRAME;

#define FRAME_AT(pAddress) (MMFRAME*) (KFRAMEMAP + (sizeof(MMFRAME) * ((unsigned long) (pAddress) >> 12)))
#define FROPAGE(pgOffset) (KFRAMEMAP + pgOffset * sizeof(MMFRAME))
#define FRADDRESS(fAddress) (PADDRESS) (((unsigned long)fAddress - KFRAMEMAP) / sizeof(MMFRAME)) * KB(4)

/**
 * Function: TypifyMRegion()
 *
 * Summary:  This function is used to declare the types of frames in a memory, by aligning
 * the address with the following rules -
 *
 * 1. For a free region (typeValue equ 1), region boundaries are contracted, meaning unaligned
 * start is increased and ending address is decreased to align on a KPGSIZE block.
 *
 * 2. For a non-free region (typeValue !equ 1), region boundaries are expanded, meaning unaligned
 * start is decreased and ending address is increased to align on a KPGSIZe block.
 *
 * This is because of non-free regions cannot be used at all and thus, they cannot be contracted.
 *
 * It is used in the initialization sequence, to mark modules as allocated, because they aren't
 * marked allocated in the memory map.
 *
 * Args:
 * typeValue - Value to put in the BdType field (BDINFO)
 * regionAddress - Region address
 * regionSize - Size of the region
 *
 * @Version 1
 * @Since Circuit 2.03
 */
void TypifyMRegion(
	unsigned long typeValue,
	unsigned long regionAddress,
	unsigned long regionSize
);

#endif

#ifdef NAMESPACE_MAIN

/**
 * Function: SetupKFrameManager() 
 *
 * Summary:
 * This function is used once, to setup the KFrameManager, after setting up the ADM and
 * multiboot interface. It then parses the MMAP and reserves the MODULES and then calls
 * internal allocator, to free all the usable page frames.
 *
 * This function relies on the SearchMultibootTag(), and after calling it, the KFrameManager
 * is fully setup and can be used by the functions below.
 *
 * @Version 2 (Multiboot 1.6 Update)
 * @Since Circuit 2.03
 * @Author Shukant Pal
 * @See Multiboot 1.6
 */
void SetupKFrameManager(
	void
);

#endif

#define ZONE_DMA 0
#define ZONE_DRIVER 1
#define ZONE_CODE 2
#define ZONE_DATA 3
#define ZONE_KERNEL 4

#define OFFSET_NOINTR 31
#define KF_NOINTR (1 << 31) /* Do not turn interrupts on */

PADDRESS KeFrameAllocate(
	unsigned long fOrder,
	unsigned long prefZone,
	unsigned long fFlags
);

/******************************************************************************
 * KeFrameFree() - 
 *
 * Summary: This function just deallocates the physical pageframe. Clients mustn't use the
 * pageframe after calling this.
 *
 * Args:
 * frameAddress - Address given during allocation
 *
 * Returns: 0
 *
 * @Version 1
 * @Since Circuit 2.03
 * @Author Shukant Pal
 * @See ZNSYS, ZNINFO, ZnFreeBlock() - "ZoneManager.h"
 ******************************************************************************/
unsigned long KeFrameFree(
	PADDRESS frameAddress
);

#define KiFrameAllocate() KeFrameAllocate(0, ZONE_KERNEL, FLG_ATOMIC) /* Atomic allocation */
#define KiFrameFree(fAddress) KeFrameFree(fAddress) /* Just the pair provided for KiFrameAllocate() */
#define KiFrameEntrap(frFlags) KeFrameAllocate(0, ZONE_KERNEL, frFlags | FLG_ATOMIC | FLG_NOCACHE) /* Bypass the PROCESSOR table (non-caching) */

#endif /* Memory/KFrameManager.h */
