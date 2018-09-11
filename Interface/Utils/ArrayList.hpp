/**
 * @file ArrayList.hpp
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
#ifndef MDFRWK_LIST_HPP__
#define MDFRWK_LIST_HPP__

#include <Synch/Spinlock.h>
#include <TYPE.h>

struct LinkedList;

/**
 * Holds object pointers in an array that can be randomly accessed
 * to add, remove, read, and modify objects. <tt>ArrayList</tt>
 * cannot be accessed concurrently, which may cause undefined effects
 * on its data.
 */
class ArrayList final
{
public:
	class Iterator;

	ArrayList();
	ArrayList(unsigned long initialCapacity);
	ArrayList(LinkedList& elems);
	~ArrayList();
	unsigned long add(void *elem);
	unsigned long add(void *elem, unsigned long index);
	unsigned long addAll(LinkedList& elems);
	unsigned long count(){ return (size); }
	inline void ensureBuffer(unsigned long newCapacity);

	void *fastGet(unsigned long idx){
		return (elemData[idx]);
	}

	void *&fastRef(unsigned long idx){
		return (elemData[idx]);
	}

	void *get(unsigned long ix) {
		return (ix < size ? elemData[ix] : 0);
	}

	bool isEmpty() {
		return (!size);
	}

	unsigned long firstIndexOf(void *);
	unsigned long lastIndexOf(void *);
	bool remove(unsigned long index);
	bool remove(void *elem);
	void set(void *elem, unsigned long idx);
	ArrayList *subList(unsigned long start, unsigned long end);
	void trimToSize();

	Spinlock modl;
private:
	unsigned long size;
	unsigned long capacity;
	unsigned long changeCount;
	void **elemData;
	bool isValidIndex(unsigned long idx){ return (idx < size); }
	void removeAt(unsigned long idx);
};

/**
 * Iterates sequentially over the elements stored in an <tt>ArrayList
 * </tt>. Generally <tt>ArrayList::Iterator</tt> usage decreases the
 * amount of code required to perform a simple loop operation.
 *
 * <tt>ArrayList::Iterator</tt> are best allocated on the stack.
 */
class ArrayList::Iterator final
{
public:
	Iterator(ArrayList& _list) : list(_list) {
		this->changeCount = list.changeCount;
		this->curIdx = 0;
		this->curPtr = *list.elemData;
		this->ref = list.elemData;
	}

	inline void *fastNext() {
		if(curIdx < list.size - 1) {
			++(curIdx);
			++(ref);
			curPtr = *ref;
			return (curPtr);
		} else {
			return (null);
		}
	}

	inline void *fastPrev() {
		if(curIdx) {
			--(curIdx);
			--(ref);
			curPtr = *ref;
			return (curPtr);
		}
		else {
			return (null);
		}
	}

	inline void *get() {
		return (curPtr);
	}

	inline unsigned long index() {
		return (curIdx);
	}

	inline void *next() {
		if(this->changeCount != list.changeCount)
			return (null);

		if(curIdx == list.size - 1)
			return (null);

		++curIdx;
		++(ref);
		curPtr = *ref;
		return (curPtr);
	}

	inline void *prev() {
		if(this->changeCount != list.changeCount)
			return (null);

		if(curIdx)
		{
			--(curIdx);
			++(ref);
			curPtr = *ref;
			return (curPtr);
		}
		else
		{
			return (null);
		}
	}

	inline void set(void *rep) {
		curPtr = rep;
		*ref = curPtr;
	}
private:
	ArrayList& list;
	unsigned long changeCount;
	unsigned long curIdx;
	void *curPtr;
	void **ref;
};

#endif/* Utils/ArrayList.hpp */
