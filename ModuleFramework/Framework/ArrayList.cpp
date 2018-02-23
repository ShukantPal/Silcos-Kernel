///
/// @file ArrayList.cpp
/// -------------------------------------------------------------------
/// This program is free software: you can redistribute it and/or modify
/// it under the terms of the GNU General Public License as published by
/// the Free Software Foundation, either version 3 of the License, or
/// (at your option) any later version.
///
/// This program is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
/// GNU General Public License for more details.
///
/// You should have received a copy of the GNU General Public License
/// along with this program.  If not, see <http://www.gnu.org/licenses/>
///
/// Copyright (C) 2017 - Shukant Pal
///

#include <Atomic.hpp>
#include <Heap.hpp>
#include <Debugging.h>
#include <Utils/Arrays.hpp>
#include <Utils/LinkedList.h>
#include <Utils/ArrayList.hpp>

const char *f = "HELLOW ";

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
	Dbg(f);
	this->capacity = defaultInitialArrayListSize * sizeof(void*);
	this->changeCount = 0;
	this->size = 0;
	this->elemData = (void **) kmalloc(capacity);
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
	if(initialCapacity==24)
		Dbg(" ARRAY_LIST ");
	this->capacity = initialCapacity;
	this->changeCount = 0;
	this->size = 0;
	this->elemData = (void **) kmalloc(capacity);
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
	this->elemData = (void **) kmalloc(capacity);

	LinkedListNode *elnode = elems.head;
	while(elnode)
	{
		this->elemData[size] = (void*) elnode;
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
unsigned long ArrayList::add(void *elem)
{
	ensureBuffer(size + 1);
	elemData[size++] = elem;
	Atomic::inc(&changeCount);
	return (size - 1);
}

/*
 * Adds the element at the given specified index, and moves subsequent elements
 * to the right. This may incur large delays if the index is low and size is
 * high.
 *
 * @param elem - element to add at given index
 * @param index - index where elem is to be added
 * @return - if element was added then index, else random value other than
 * 		index
 * @author Shukant Pal
 */
unsigned long ArrayList::add(void *elem, unsigned long index)
{
	if(isValidIndex(index))
	{
		ensureBuffer(size + 1);
		Arrays::copyFastFromBack(elemData + size, elemData + size + 1,
						size - index);
		Atomic::inc(&changeCount);
		return (index);
	}
	else
	{
		return (--index);
	}
}

/*
 * Adds all the elements in the linked-list given at the very end of this
 * array-list, provided that it isn't being used externally.
 *
 * @param elems - linked-list of elements to add
 * @author Shukant Pal
 */
unsigned long ArrayList::addAll(LinkedList& elems)
{
	ensureBuffer(size + elems.count);

	LinkedListNode *lielem = elems.head;
	while(lielem)
	{
		elemData[size++] = (void *) lielem;
		lielem = lielem->next;
	}
	Atomic::inc(&changeCount);

	return (size - elems.count);
}

/*
 * Ensures that the current data-buffer has enough capacity hold newCapacity
 * elements without expansion. If required, elements are copied from the old
 * buffer to the new-buffer.
 *
 * @param newCapacity - required minimal capacity for the new array-list.
 * @author Shukant Pal
 */
inline void ArrayList::ensureBuffer(unsigned long newCapacity)
{
	newCapacity *= sizeof(void*);
	if(newCapacity > capacity)
	{
		void **newBuffer = (void**) kralloc(elemData, newCapacity);

		if(newBuffer != elemData)
		{
			Arrays::copy(elemData, newBuffer, capacity);
			Atomic::inc(&changeCount);
		}

		capacity = newCapacity;
	}
}

/*
 * Searches for the object in the array-list and returns its first index
 * from the start.
 *
 * @param o - element whose first-index is required
 * @return - first-index of o, if found in the list; if the element is not in
 * 		the list, then 0xFFFFFFFF
 * @author Shukant Pal
 */
unsigned long ArrayList::firstIndexOf(void *o)
{
	unsigned long accToken = this->size;
	void **elemPtr = elemData;
	while(accToken)
	{
		if(*elemPtr == (void *) o)
			return (size - accToken);

		++(elemPtr);
		--(accToken);
	}

	return (0xFFFFFFFF);
}

/*
 * Searches for the object from behind in the array-list and returns its last
 * index from the start.
 *
 * @param o - element whose index is required
 * @returns - index of last occurence of o, if found in the list; otherwise,
 * 		if the element isn't in the list, then 0xFFFFFFFF
 */
unsigned long ArrayList::lastIndexOf(void *o)
{
	unsigned long accToken = size;
	void **elem = elemData + size;
	while(accToken)
	{
		if(*elem == o)
			return (accToken);

		--(elem);
		--(accToken);
	}
	return (0xFFFFFFFF);
}

/*
 * Removes the element present at the given index, provided that it is in
 * bounds.
 *
 * @param idx - index of element to remove
 * @return - if index was in bounds & element was removed
 * @author Shukant Pal
 */
bool ArrayList::remove(unsigned long idx)
{
	if(isValidIndex(idx))
	{
		Arrays::copyFast(elemData + idx + 1, elemData + idx,
					(size - idx - 1) * sizeof(void *));
		--(size);
		elemData[size] = null;
		return (true);
	}
	else
	{
		return (false);
	}
}

/*
 * Sets the element present at the given index. If the index given is out of
 * bounds, then the array is expanded to accomodate it. Further, if an element
 * is already present at that index, then it is replaced, and no shifting of
 * adjacent elements occurs. That means an element may be lost from the array
 * due to careless set()ing.
 *
 * @param elem - element which should be placed at given index
 * @param idx - index at which element is to be placed
 * @author Shukant Pal
 */
void ArrayList::set(void *elem, unsigned long idx)
{
	ensureBuffer(idx + 1);
	elemData[idx] = elem;

	if(++idx > size)
		size = idx;
}
