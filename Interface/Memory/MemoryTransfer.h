/*=++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * File: MemoryTransfer.h
 *
 * Summary: This file contains the interface to map physical pageframes from one virtual address
 * to another. This is useful because copying is not needed, and thus data structures can easily be
 * expanded.
 *
 * Functions:
 *
 * GetFrames() - This function is used for retrieving physical pageframes from a virtual address.
 *
 * SetFrames() - This function is used for mapping the pageframes, given from GetFrames() to the
 * destination address.
 *
 * Copyright (C) 2017 - Shukant Pal
 *+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=*/
#ifndef MEMORY_TRANSFER_H
#define MEMORY_TRANSFER_H

#define NS_KFRAMEMANAGER/* MM_FRAME is internal to the KFrameManager */

#include "Pager.h"

/******************************************************************************
 * Function: GetFrames()
 *
 * Summary: This function creates a stack containing the pages from the given address,
 * where the head of the stack is the last page in the address region. It doesn't support
 * large pages, thus, usage of this function should be done on known regions of memory,
 * where only 4-k pages are mapped.
 *
 * Args:
 * ADDRESS address - Address of the source
 * ADDRESS pages - No. of pages to stack
 * CONTEXT *pageContext - Address space, to retrive pages from
 *
 * Changes:
 * None (read-only work done)
 *
 * Returns: The head of the stack created, after processing the page tables.
 *
 * @Version 1
 * @Since Circuit 2.03
 * @Author Shukant Pal
 ******************************************************************************/
MMFRAME *GetFrames(
	ADDRESS address,
	ADDRESS pages,
	CONTEXT *pageContext
);

/******************************************************************************
 * Function: SetFrames()
 *
 * Summary: This function maps the frame stack given by GetFrames() , to the destination,
 * in the same order as they were mapped at the source.
 *
 * Args:
 * address - Address of the destination
 * frameHead - Head of the stack (as returned by GetFrames())
 * pageContext - Current address space (must be correct, otherwise nothing occurs)
 *
 * Changes:
 * 1. The state of the context given
 * 2. Current address space (if context is current one)
 *
 * Returns: void
 *
 * @Version 1
 * @Since Circuit 2.03
 * @Author Shukant Pal
 ******************************************************************************/
void SetFrames(
	ADDRESS address,
	MMFRAME *frameHead,
	CONTEXT *pageContext
);

#endif
