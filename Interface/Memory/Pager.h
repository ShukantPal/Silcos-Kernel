/*=++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * File: Pager.h
 *
 * Summary: This file provides the interface to manipulate page tables, and map physical pageframes
 * to the kernel/user-mode address space. It abstracts the hardware-level paging mechanism and gives
 * a generic interface for creating, mapping, demapping, validating, switching, and deleting address
 * spaces using the CONTEXT type. It also includes the PAGE_ATTRIBUTES type, used for defining the
 * nature of a page, with macros hiding the internal format of the flags.
 *
 * Functions:
 *
 * SwitchContext() - This function is used particularly for switch address spaces.
 *
 * EnsureMapping() - This function ensures that a virtual address is mapped to a physical address.
 *
 * EnsureUsability() - This function ensures that a virtual address is usable and mapped to any valid
 * pageframe.
 *
 * CheckValidity() - This function checks whether a virtual address is mapped or not.
 *
 * Copyright (C) 2017 - Shukant Pal
 *+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=*/
#ifndef MEMORY_PAGING_H
#define MEMORY_PAGING_H

#include <Types.h>
#include <Synch/Spinlock.h>
#include <Util/XArray.h>
#include "KFrameManager.h"

/* Paging attribute macros */
#ifdef x86
	#include <IA32/PageTrans.h>
#endif

/* Unified attributes */
#define KernelData (PagePresent | PageReadWrite)

#ifdef NS_PMFLGS
	#define KERNEL_DATA (PRESENT | READ_WRITE)
#endif

/* Helper macros */
#define OwnerID(pc) (pc -> OwnerId)
#define ContextFlags(pc) (pc -> Flags)
#define HardwarePage(pc) (pc -> HardwarePage)

#define InitCntOperation(C) SpinLock(&C -> ContextLock)
#define CompleteCntOperation(C) SpinUnlock(&C -> ContextLock)

#define CntUsedBy(C) (C -> UsedBy)

/**
 * Type: CONTEXT 
 *
 * Summary:
 * Pager::Context or CONTEXT refers to a virtual address space in which a process
 * or other resource-handlers lives. It is isolated from all other user-mode processes
 * & resource-handlers and contains page-tables to map virtual-to-physical addresses.
 *
 * This type must be partially opaque for cross-platform code, as to manage the disimilarities
 * b/w various architectures in how pagination is done. The Pager subsys gives various
 * functions to manipulate the CONTEXT object.
 *
 * You must know that CONTEXT need not be locked while using the helper functions, as they
 * use seperate platform-dependent locks to synchronize page-table accesses.
 *
 * @Version 2
 * @Since Circuit 2.01
 * @Author Shukant Pal
 */
typedef
struct _CONTEXT {
	unsigned int OwnerId;/* RRM_ID of the owner resource-handler */
	unsigned int Flags;/* Context-handling flags, provided by macros */
	PAGE_TRANSALATOR HardwarePage;/* Hardware-impl of the paging mechanism */
	unsigned int UsedBy;/* Number of resource-handlers referencing this context */
	SPIN_LOCK ContextLock;/* Lock for concurrent accesses*/
} CONTEXT;

extern /* Only addresses > 3gb mapped */
CONTEXT SystemCxt;

/**
 * Function: SwitchContext()
 *
 * Summary:
 * This function is used while switching address spaces. It ensures that the kernel is mapped
 * during transition.
 *
 * Args:
 * pgContext - The page context being used
 *
 * Changes:
 * The current address space
 *
 * @Version 1.1
 * @Since Circuit 2.03
 * @Author Shukant Pal
 */
void SwitchContext(
	CONTEXT *pgContext
);

/**
 * Function: EnsureMapping()
 *
 * Summary:  This function ensures that the address is mapped the location required in physical
 * memory. It directly maps the address, and allocates the page tables (if required), with
 * the given PMA flags. The flags are generally required during early initialization, when
 * some functionalities are not present (such as caching).
 *
 * NOTE: This function is used for mapping only small pages of the default page size. Special
 * features such as 2-MB or 1-GB pages are not supported, in this function.
 *
 * Args:
 * address - Virtual address, being used for mapping
 * pAddress - Physical address, to be mapped to
 * pgContext - Current context (NULL if kernel memory is being mapped)
 * frFlags - PMA flags for allocation of page tables
 * pgAttr - Page attributes to apply on the address
 *
 * Returns: void
 *
 * @Version 1
 * @Since Circuit 2.03
 * @Author Shukant  Pal
 */
void EnsureMapping(ADDRESS address, PADDRESS pAddress, CONTEXT *pgContext,
			unsigned long frFlags, PAGE_ATTRIBUTES pgAttr);

/**
 * Function: EnsureUsability()
 *
 * Summary:  This function maps the address given, to any available physical page. It allocates 
 * it through the PMA, given flags for that. It doesn't support large pages.
 *
 * Args:
 * address - Address to map
 * pgContext - Current context (NULL, if kernel memory is used)
 * frFlags - PMA flags for allocating the frame, and page tables (if any)
 * pgAttr - Page attributes to apply
 *
 * Returns: void
 *
 * @Version 2
 * @Since Circuit 2.03
 * @Author Shukant Pal
 */
void EnsureUsability(ADDRESS address, CONTEXT *pgContext, unsigned long frFlags,
			PAGE_ATTRIBUTES pgAttr);

void EnsureFaulty(ADDRESS address, CONTEXT *c);

/**
 * Function: CheckUsability()
 *
 * Summary: This function just checks whether the given address is usable (is mapped 
 * to a valid address). It does not change any pte(s).
 *
 * Args:
 * address - The address to be tested for mapping
 * pgContext - The context in which mapping is done
 *
 * Returns: Whether the address is mapped
 *
 * @Version 1
 * @Since Circuit 2.03
 * @Author Shukant Pal
 */
bool CheckUsability(ADDRESS address, CONTEXT *pgContext);

void EnsureAllMappings(unsigned long address, PADDRESS pAddress, unsigned long mapSize,
			CONTEXT *pgContext, PAGE_ATTRIBUTES pgAttr);

/* Used to expand heaps by size HEAP_EXTENSION_SIZE (PageTrans) */
ADDRESS RtDynamicMemory(
);

ADDRESS AtStack(
	CONTEXT *
);

void DtStack(
	CONTEXT *,
	unsigned long StackBase
);

#endif /* Memory/Pager.h */
