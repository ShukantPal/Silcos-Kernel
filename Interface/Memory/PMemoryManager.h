/*=++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * File: PMemoryManager.h
 *
 * Summary: This file contains the interface for memory management of user-space processes which
 * use the system-provided memory manager. It does so by providing the PSMM_MANAGER, which is
 * a superset of the MM_MANAGER, along with a few memory region types, boundary-specifiers, and
 * export of the implementor functions.
 *
 * Functions:
 *
 * PMgrCreate() - This function is used for creating a PSMM_MANAGER with a few initializing fields to
 * specify the program's core.
 *
 * PMgrDispose() -  This function is required to dispose the manager, by reducing its reference
 * count. After no reference is left, it is deleted from memory.
 *
 * Copyright (C) 2017 - Shukant Pal
 *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=*/
#ifndef MEMORY_PMM_MANAGER_H
#define MEMORY_PMM_MANAGER_H

#include "MemoryManager.h"
#include "Pager.h"
#include <Util/AVLTree.h>
#include <Util/LinkedList.h>
#include <TYPE.h>

struct _PSMM_MANAGER;

/**
 * Enum: MMREGION_TYPE
 *
 * Summary: This enum declares the types of regions that could be used in the PSMM
 * memory manager.
 *
 * @Version 1
 * @Since Circuit 2.03
 */
typedef
enum _MMREGION_TYPE
{
	CODE = 0, /* Executable code */
	DATA = 1, /* Data in the program's executable file */
	BSS = 2, /* Blank-section in the program's executable file */
	HEAP = 3, /* User-space heap */
	STACK_MEMORY = 4, /* Processor stack memory */
	SHARED_MEMORY = 5, /* shm-IPC memory */
	CUSTOM_MEMORY = 6, /* unknown/manual memory */
	BOUNDARY_MEMORY = 7, /* Memory used for preventing buffer-overflow (never mapped) */
	UNKNOWN = 8 /* Unknown type (used for recording errors) */
} MMREGION_TYPE;

enum { BOUND_START = 0, BOUND_END = 1 };

/**
 * Type: PSMM_MANAGER
 *
 * Summary: This type defines the methodology, for management and maintenance of process
 * address space. It can be shared among multiple processes (from the same program
 * core, of course). It defines various builtin regions such as CODE, DATA, BSS and
 * STACK. It has the following functions -
 * 1. Program Data Management
 * 2. PFRA
 * 3. Allocation of memory regions
 * 4. Page Fault Handling
 * 
 * @Version 1
 * @Since Circuit 2.03
 */
typedef
struct _PSMM_MANAGER {
	MM_MANAGER MmMgr;/* MemoryManager Interface */
	MM_REGION *LastUsed;/* Cache for last used region */
	
	#define PM_MAP 0
	unsigned long PmFlags;/* Internal state flags */
	unsigned long UsedMemory;/* Amount of memory usable by the process (mapped in regions) */
	unsigned long CodeBounds[2];/* Executable Code */
	unsigned long DataBounds[2];/* Static Data */
	unsigned long BSSBounds[2];/* Uninitialized Data */
	unsigned long HeapBounds[2];/* Heap given by kernel */
	unsigned long StackBounds[2];/* Stack provided by kernel */
	unsigned long CodePages;/* Pages mapped to executable code */
	unsigned long DataPages;
	unsigned long BSSPages;
	unsigned long HeapPages;
	unsigned long SharedPages;/* Pages mapped for shm-IPC */
	unsigned long PinnedPages;
	unsigned long DLLCount;
} PSMM_MANAGER;

#define convert_psmm PSMM_MANAGER*

/**
 * PMgrCreate() -
 *
 * Summary:
 * This function creates a PSMM-manager and sets all fields to default
 * values, and creates memory bounds for various elf-sections for the
 * process program.
 *
 * Args:
 * unsigned long codeBounds[2] - Bounds for the code section
 * unsigned long dataBounds[2] - Bounds for the data section
 * unsigned long bssBounds[2] - Bounds for the BSS section
 *
 * @Version 1
 * @Since Circuit 2.03
 */
PSMM_MANAGER *PMgrCreate(
	unsigned long codeBounds[2],
	unsigned long dataBounds[2],
	unsigned long bssBounds[2]
);

/** @Interface MM_MANAGER.MmInsertRegion() */
unsigned long PInsertRegion(
	unsigned long rgAddress,
	unsigned long rgSize,
	unsigned long rgFlags,
	PAGE_ATTRIBUTES pgFlags,
	MM_MANAGER *mgr
);

/** @Interface MM_MANAGER.MmRemoveRegion() */
unsigned long PRemoveRegion(
	unsigned long rgAddress,
	unsigned long rgSize,
	unsigned short rgType,
	MM_MANAGER *mgr
);

/** @Interface MM_MANAGER.MmExtendRegion() */
unsigned long PExtendRegion(
	unsigned long rgAddress,
	unsigned long exAddress,
	MM_MANAGER *mgr
);

/** @Interface MM_MANAGER.MmValidateAddress() */
MM_REGION *PValidateAddress(
	unsigned long address,
	MM_MANAGER *mgr
);

void PMgrDipose(
	PSMM_MANAGER *mgr
);

#endif /* Memory/PMemoryManager.h */
