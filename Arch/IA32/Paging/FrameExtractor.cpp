/* Copyright (C) 2017 - Shukant Pal */

#define NS_KFRAMEMANAGER

#include <Memory/Address.h>
#include <Memory/Pager.h>
#include <Memory/KFrameManager.h>
#include <Memory/MemoryTransfer.h>
#include <IA32/PageExplorer.h>
#include <Util/Stack.h>

MMFRAME *GetFrames(ADDRESS Address, ADDRESS Pages, CONTEXT *pageContext)
{
	unsigned long pageCounter;
	U64 *pageTable;
	unsigned long tableOffset;
	PADDRESS frameAddress;
	MMFRAME *frame;
	STACK frameStack = NEW_STACK;

	for(pageCounter = 0; pageCounter < Pages;) {
		tableOffset = ((Address + pageCounter * KB(4)) % MB(2)) / KB(4);
		pageTable = GetPageTable(Address / GB(1), (Address % GB(1)) / MB(2), 0, pageContext);
		while(pageCounter < Pages && tableOffset < 512) {
			frameAddress = pageTable[tableOffset] & ~((1 << 12) - 1);
			frame = FRAME_AT(frameAddress);
			PushElement((STACK_ELEMENT *) frame, &frameStack);
			++tableOffset;
			++pageCounter;
		}
	}

	return (MMFRAME *) (frameStack.Head);
}

void SetFrames(ADDRESS addressEnd, MMFRAME *frameHead, CONTEXT *pageContext)
{
	MMFRAME *frame = frameHead;
	PADDRESS frameAddress;
	unsigned long tableOffset;
	U64 *pageTable;

	addressEnd -= KB(4);
	while(frame != NULL) {
		tableOffset = (addressEnd % MB(2)) / KB(4);
		pageTable = GetPageTable(addressEnd / GB(1), (addressEnd % GB(1)) / MB(2), 0, pageContext);
		while(frame != NULL && tableOffset > 0) {
			frameAddress = FRADDRESS(frame);
			pageTable[tableOffset] |= frameAddress | KernelData;
			--tableOffset;
			FlushTLB(addressEnd);
			frame = (MMFRAME *) (frame->ListLinker.Next);
			addressEnd -= KB(4);
		}
	}
} 
