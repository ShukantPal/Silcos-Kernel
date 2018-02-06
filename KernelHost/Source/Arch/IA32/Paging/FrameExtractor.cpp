/* Copyright (C) 2017 - Shukant Pal */

#define NS_KFRAMEMANAGER

#include <Memory/Address.h>
#include <Memory/Pager.h>
#include <Memory/KFrameManager.h>
#include <Memory/MemoryTransfer.h>
#include <IA32/PageExplorer.h>
#include "../../../../../Interface/Utils/Stack.h"

/**
 * Function: GetFrames
 *
 * Summary:
 * This function creates a stack containing the pages from the given address,
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
 * Version: 1
 * Since: Circuit 2.03
 * Author: Shukant Pal
 */
MMFRAME *GetFrames(ADDRESS Address, ADDRESS Pages, CONTEXT *pageContext)
{
	unsigned long pageCounter;
	U64 *pageTable;
	unsigned long tableOffset;
	PADDRESS frameAddress;
	MMFRAME *frame;
	STACK frameStack = NEW_STACK;

	for(pageCounter = 0; pageCounter < Pages;)
	{
		tableOffset = ((Address + pageCounter * KB(4)) % MB(2)) / KB(4);
		pageTable = GetPageTable(Address / GB(1), (Address % GB(1)) / MB(2), 0, pageContext);
		
		while(pageCounter < Pages && tableOffset < 512)
		{
			frameAddress = pageTable[tableOffset] & ~((1 << 12) - 1);
			frame = FRAME_AT(frameAddress);
			PushElement((STACK_ELEMENT *) frame, &frameStack);
			++tableOffset;
			++pageCounter;
		}
	}

	return (MMFRAME *) (frameStack.Head);
}

/**
 * Function: SetFrames
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
 * Version: 1
 * Since: Circuit 2.03
 * Author: Shukant Pal
 */
void SetFrames(ADDRESS addressEnd, MMFRAME *frameHead, CONTEXT *pageContext)
{
	MMFRAME *frame = frameHead;
	PADDRESS frameAddress;
	unsigned long tableOffset;
	U64 *pageTable;

	addressEnd -= KB(4);
	while(frame != NULL)
	{
		tableOffset = (addressEnd % MB(2)) / KB(4);
		pageTable = GetPageTable(addressEnd / GB(1), (addressEnd % GB(1)) / MB(2), 0, pageContext);

		while(frame != NULL && tableOffset > 0)
		{
			frameAddress = FRADDRESS(frame);
			pageTable[tableOffset] |= frameAddress | KernelData;
			--tableOffset;
			FlushTLB(addressEnd);
			frame = (MMFRAME *) (frame->ListLinker.next);
			addressEnd -= KB(4);
		}
	}
} 
