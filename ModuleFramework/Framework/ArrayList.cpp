/* @file ArrayList.cpp
 * -------------------------------------------------------------------
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 *
 * Copyright (C) 2017 - Shukant Pal
 */
#include <Heap.hxx>
#include <Util/ArrayList.hpp>
#include <Util/Arrays.hpp>
#include <Util/LinkedList.h>

#define defaultInitialArrayListSize 8

/* @constructor
 *
 * Initializes the array-list with default parameters. The initial capacity
 * of this list will be 8 (or 32-bytes, which is the min. heap allocation
 * unit).
 *
 * @author Shukant Pal
 */
ArrayList::ArrayList()
{
	this->capacity = defaultInitialArrayListSize;
	this->changeCount = 0;
	this->size = 0;
	this->elemData = (Object volatile **) kmalloc(capacity);
}

/* @constructor
 *
 * Initializes the array-list with the given initial buffer-capacity. By
 * specifying so, you can prepare this list to add more amounts of data
 * beforehand.
 *
 * @param initialCapacity - initial buffer-capacity for the list
 * @author Shukant Pal
 */
ArrayList::ArrayList(unsigned long initialCapacity)
{
	this->capacity = initialCapacity;
	this->changeCount = 0;
	this->size = 0;
	this->elemData = (Object volatile **) kmalloc(capacity);
}

/* @constructor
 *
 * Copies all the elements from the elems linked-list to this array-list. The
 * size & capacity will be identical to the count of the linked-list. If
 * concurrent usage of the linked-list is going on, be sure to synchronize it.
 *
 * Note that corrupt linked-lists will also form a corrupt array-list.
 *
 * @param elems - linked-list of whose all elements are to be included in this
 * @author Shukant Pal
 */
ArrayList::ArrayList(LinkedList &elems)
{
	this->capacity = elems.count;
	this->changeCount = 0;
	this->size = elems.count;
	this->elemData = (Object volatile **) kmalloc(capacity);

	LinkedListNode *elnode = elems.head;
	while(elnode)
	{
		this->elemData[size] = elnode;
		elnode = elnode->next;
	}
}

/*
 * Adds the element to the array-list at the very end. This elem can also
 * be a "valid" null-pointer. Try to stay away from dangling pointers, though.
 *
 * @param elem - element to add
 * @author Shukant Pal
 */
unsigned long ArrayList::add(Object *elem)
{
	ensureBuffer(size + 1);
	elemData[size++] = elem;
	return (size - 1);
}

unsigned long ArrayList::add(Object *elem, unsigned long index)
{

}

/*
 * Ensures that the current data-buffer has enough capacity hold newCapacity
 * elements without expansion. If required, elements are copied from the old
 * buffer to the new-buffer.
 *
 * @param newCapacity - required minimal capacity for the new array-list.
 * @author Shukant Pal
 */
void ArrayList::ensureBuffer(unsigned long newCapacity)
{
	if(newCapacity > capacity)
	{
		Object volatile **newBuffer = kralloc(elemData, newCapacity);

		if(newBuffer != elemData)
			Arrays::copy(elemData, newBuffer, capacity);

		capacity = newCapacity;
	}
}
