/* Copyright (C) 2017 - Shukant Pal */

#include <Memory/KMemorySpace.h>
#include <Memory/Pager.h>
#include <Multiboot2.h>
#include <Debugging.h>

MultibootTag *MultibootChannel::tagTable;
unsigned long MultibootChannel::tagFence;

/**
 * Maps the multiboot-table in kernel-memory so that it can be
 * accessed. It doesn't assume that the table will fit in one page.
 */
void MultibootChannel::init(unsigned long multibootTable)
{
	Pager::map(MULTIBOOT_INTERFACE, multibootTable, null, KernelData);

	tagTable = (MultibootTag *)(MULTIBOOT_INTERFACE + multibootTable % PAGE);
	tagFence = (unsigned long) tagTable + *(unsigned long *) tagTable;

	if(getMultibootTableSize(tagTable) > PAGE)
		Pager::mapAll(MULTIBOOT_INTERFACE + PAGE, multibootTable + PAGE,
				*(unsigned long *) tagTable - PAGE, null, KernelData);

	++(tagTable);
}

/**
 * Finds the first entry in the multiboot information table present
 * with the given tag.
 *
 * @param typeToken - The type of tag required for the entry being
 * 					searched for.
 * @return - A generic pointer to the multiboot entry; null, if no
 * 			entry was found.
 */
MultibootSearch MultibootChannel::getTag(U32 typeToken)
{
	MultibootSearch tPtr(getFirstTag());

	while(tPtr.loc < tagFence) {
		if(tPtr.tag->type == typeToken) {
			return (tPtr);
		}

		getNextTag(tPtr);
	}

	tPtr.loc = null;
	return (tPtr);
}

/**
 * Forward searches for the next entry, of the same type as passed. If
 * another entry of the same type is found, it is returned.
 *
 * @param from - A generic pointer to an entry, from which the next one
 * 				is to be searched.
 * @return - A generic pointer to the next entry of the same type; null,
 * 				if it is not present.
 */
MultibootSearch MultibootChannel::getNextTagOfType(MultibootSearch from)
{
	unsigned long typeToken = from.tag->type;
	MultibootSearch tPtr(from);
	getNextTag(tPtr);

	while(tPtr.loc < tagFence) {
		if(tPtr.tag->type == typeToken) {
			return (tPtr);
		}

		getNextTag(tPtr);
	}

	tPtr.loc = null;
	return (tPtr);
}
