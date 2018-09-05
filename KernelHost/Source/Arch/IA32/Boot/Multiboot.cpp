/**
 * @file Multiboot.cpp
 * ------------------------------------------------------------------
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 *
 * Copyright (C) 2018 - Shukant Pal
 */
#include <Memory/KMemorySpace.h>
#include <Memory/Pager.h>
#include <Multiboot2.h>
#include <Debugging.h>

/**
 * Holds a valid pointer to the multiboot table in kernel memory. This
 * skips the first element, the size of the whole table, which can be
 * accessed using <tt>getMultibootTableSize</tt>
 */
MultibootTag *MultibootChannel::tagTable;

/**
 * The upper-bound of the multiboot table <tt>tagTable</tt>. This can
 * be used directly as <tt>tag &lt; (MultibootTag*) tagFence</tt> to end
 * a loop.
 */
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
 * Returns the first tag, of the given type, in the multiboot
 * table. If no such tag exists, a <tt>weak_null</tt> object is
 * returned.
 *
 * @param typeToken - type of the required tag
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
 * Returns the tag of type <tt>from.tag->type</tt> which comes first
 * after the given <tt>from</tt> tag. If no such tag exists, then
 * a <tt>weak_null</tt> is passed.
 *
 * @param from - a multiboot-search object pointing the given tag,
 * 				from which the next tag is to be found.
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
