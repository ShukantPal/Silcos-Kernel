/* @file List.hpp
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

#include <Object.hpp>
#include <TYPE.h>

struct LinkedList;

/* @class ArrayList
 *
 * Holds elements in a resizable array-buffer in the form of pointers. This
 * type of list is useful for non-intrusive usage and low-memory
 * requirements. This array-list also allows random access and allowing sorted
 * inserts & removes through the iterator.
 *
 * @version 1.0
 * @since Silcos 3.02
 * @author Shukant Pal
 */
class ArrayList
{
public:
	ArrayList();
	ArrayList(unsigned long initialCapacity);
	ArrayList(LinkedList& elems);
	unsigned long add(Object *elem);
	unsigned long add(Object *elem, unsigned long index);
	unsigned long addAll(LinkedList elems);
	unsigned long count(){ return (size); }
	void ensureBuffer(unsigned long newCapacity);
	Object *get(unsigned long ix){ return (ix < size ? elemData[ix] : 0); }
	bool isEmpty(){ return (!size); }
	unsigned long firstIndexOf(Object *);
	unsigned long lastIndexOf(Object *);
	bool remove(unsigned long index);
	bool remove(Object *elem);
	ArrayList *subList(unsigned long start, unsigned long end);
	void trimToSize();

	/* @class ArrayList::Iterator
	 *
	 * Abstraction for iterating over an ArrayList. It will hold a current
	 * index by which the user can serially go back and forth. A new
	 * Iterator always starts from zero, unless specified. By iterating,
	 * the user can also replace objects "on the fly".
	 *
	 * To optimize local usage, declare the Iterator on the stack -
	 *
	 * 			Iterator listItr(mylist);
	 *
	 * @version 1.0
	 * @since Silcos 3.02
	 * @author Shukant Pal
	 */
	class Iterator
	{
	public:
		Iterator(ArrayList& list);

		inline Object *get()
		{
			return (curPtr);
		}

		inline Object *next()
		{
			if(this->changeCount != list.changeCount)
				return (null);

			++curIdx;
			curPtr = (curIdx < list.size) ?
					(Object*) list.elemData[curIdx] :
					null;
			return (curPtr);
		}

		inline Object *prev()
		{
			if(this->changeCount != list.changeCount)
				return (null);

			if(curIdx)
			{
				--(curIdx);
				curPtr = (Object*) list.elemData[curIdx];
				return (curPtr);
			}
			else
			{
				return (null);
			}
		}

		inline void set(Object *rep)
		{
			curPtr = rep;
			list.elemData[curIdx] = curPtr;
		}
	private:
		ArrayList& list;
		unsigned long changeCount;
		unsigned long curIdx;
		Object *curPtr;
	};
private:
	unsigned long size;
	unsigned long capacity;
	unsigned long changeCount;
	Object volatile **elemData;
	bool isValidIndex(unsigned long idx){ return (idx < size); }
	void removeAt(unsigned long idx);
};

#endif/* Util/List.hpp */
