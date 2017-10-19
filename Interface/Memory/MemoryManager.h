/**
 * Copyright (C) 2017 - Shukant Pal
 *
 * MemoryManager is a generic interface for manipulating process address spaces, so
 * that further extensions are easy to build. It uses the MM_MANAGER, which contains
 * functions (ptr) to do logical operations on address spaces. Usage of regions is defined
 * by the structure MM_REGION, which contains flags and nodes to control the region.
 *
 * Both things are extendable, and thus further development is required to build the
 * functionality. Default functionality is built in the PMemoryManager, which is the default
 * code for managing process memory.
 */
#ifndef MEMORY_MEMORYMANAGER_H
#define MEMORY_MEMORYMANAGER_H

#include "Pager.h"
#include <Util/AVLTree.h>
#include <Util/LinkedList.h>

/**
 * MM_REGION - 
 *
 * Summary:
 * This type is used for referring to a memory region, which may or may not be
 * mapped in virtual memory. All pages in a region have the same page attributes, and
 * are of the same type.
 *
 * Memory regions are particularly used for checking whether a unmapped address
 * is valid for the process or not.
 *
 * They can be inserted, removed, extended and shrunk in the process address space.
 *
 * Fields:
 * Node - AVLNode for usage in tree
 * LeftLinker - Left-child of the region (in AVL-tree)
 * RightLinker - Right-child of the region (in AVL-tree)
 * NodeHeight - Height of the region's AVLNode
 * Address - Address of the region
 * LiLinker - Participating in lists
 * Size - Size of the region
 * Flags - 
 * TypeNo - Type Id for the region
 * ControlFlags - Flags for logical control of the region's behavior
 * PagerFlags - Flags for mapping the pages
 *
 * @Version 1
 * @Since Circuit 2.03
 */
typedef
struct _MM_REGION {
	union {
		AVLNODE Node;
		struct {
			struct _MM_REGION *LeftLinker;
			struct _MM_REGION *RightLinker;
			ULONG NodeHeight;
			ULONG Address;
		};
	};
	union {
		LINODE LiLinker;
		struct {
			struct _MM_REGION *PreviousLinker;
			struct _MM_REGION *NextLinker;
		};
	};
	ULONG Size;
	union {
		ULONG Flags;
		struct {
			USHORT TypeNo;
			USHORT ControlFlags;
		};
	};
	PAGE_ATTRIBUTES PagerFlags;
} MM_REGION;

#define PSMM_TP 0xFFFFFFFE

/**
 * MM_MANAGER - 
 *
 * Summary:
 * This type is for managing process address spaces. It is a data header for interfacing
 * with the underlying memory manager. The memory manager should implement this
 * interface to work with the kernel.
 *
 * Fields:
 * MgrName - Name of the memory manager
 * MgrTp - Its typeID
 * CODE - Region representing code
 * DATA - " " data
 * BSS - " " BSS
 * HEAP - " " heap
 * STACK - " " stack(s)
 * RegionList - Linked list of regions (must be up-to-date)
 * RegionMap - Tree of regions (optional & internal)
 * MmInsertRegion - (@Documented)
 * MmRemoveRegion - (@Documented)
 * MmExtendRegion - (@Documented)
 *
 * @Version 1
 * @Since Circuit 2.03
 */
typedef
struct _MM_MANAGER {
	CHAR *MgrName;
	ULONG MgrTp;
	ULONG ReferCount;
	MM_REGION *CODE; /* Set by client */
	MM_REGION *DATA; /* Set by client */
	MM_REGION *BSS; /* Set by client */
	MM_REGION *HEAP; /* Set by client */
	MM_REGION *STACK; /* Set by client */
	LINKED_LIST RegionList;
	AVLTREE RegionMap;

	#define RG_INSERT 0xFFFF /* Successfully added */
	#define RG_EXISTS 0xFFFE /* Overlapping region exists */
	#define RG_FAILURE 0xFFEE /* Failure (unknown) */
	/******************************************************************************
	 * MM_MANAGER.MmInsertRegion() - 
	 *
	 * Summary:
	 * This generic function will map a region in the process address space given its
	 * limits, flags and page-attributes. It can give the above values in return to show
	 * status of the operation.
	 *
	 * Args:
	 * rgAddress - Region's address
	 * rgSize - Region's size
	 * rgFlags - TypeID (16-bits) + Control flags (16-bits)
	 * pgFlags - Page attributes
 	 * mgr -
	 *
	 * @Version 1
	 * @Since Circuit 2.03
 	 ******************************************************************************/
	ULONG (*MmInsertRegion)(
		ULONG rgAddress,
		ULONG rgSize,
		ULONG rgFlags,
		PAGE_ATTRIBUTES pgFlags,
		struct _MM_MANAGER *mgr
	);

	#define RG_REMOVE 0xEEEE
	#define RG_UNDERLAP 0xEEEF
	/* OR RG_FAILURE */
	/******************************************************************************
	 * MM_MANAGER.MmRemoveRegion() -
	 *
	 * Summary: This function is used to remove a region containing the address
	 * given. This is means that only one region will be shrunk or carved even iff the size
	 * given covers multiple regions. Thus, use this only to remove a particular region
	 * partially or completely, but don't use it for remove multiple regions at once.
	 *
	 * Args:
	 * rgAddress - Region's address
	 * rgSize - Region's size
	 * rgType - Type of the region (for cross-checking, or TP_UNKNOWN)
	 * ctlFlags - Flags for controlling this operation
	 * mgr - 
	 *
	 * @Version 1
 	 * @Since Circuit 2.03
	 ******************************************************************************/
	ULONG (*MmRemoveRegion)(
		ULONG rgAddress,
		ULONG rgSize,
		USHORT rgType,
		struct _MM_MANAGER *mgr
	);

	#define RG_EXTEND 0xFEEF

	/**
	 * MM_MANAGER.MmExtendRegion() -
	 *
	 * Summary:
	 * This function is used for extending memory regions, especially stack regions.
	 *
	 * Args:
	 * rgAddress - Address of the region
	 * exAddress - Address, which should come in the region after extension
	 * mgr -
	 *
	 * @Version 1
	 * @Since Circuit 2.03
 	 */
	ULONG (*MmExtendRegion)(
		ULONG rgAddress,
		ULONG exAddress,
		struct _MM_MANAGER *mgr
	);

	/**
	 * MM_MANAGER.MmValidateAddress() -
	 *
	 * Summary:
	 * This function will return a memory region which contains the given address.
	 *
	 * Args:
	 * address - Required address
	 *
	 * @Version 1
	 * @Since Circuit 2.03
	 */
	MM_REGION *(*MmValidateAddress)(
		ULONG address,
		struct _MM_MANAGER *pmgr
	);
} MM_MANAGER;

#endif /* Memory/MemoryManager.h */
