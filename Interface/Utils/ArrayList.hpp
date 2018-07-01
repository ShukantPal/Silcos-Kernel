///
/// @file ArrayList.hpp
/// @module ModuleFramework (kernel.silcos.mdfrwk)
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
#ifndef MDFRWK_LIST_HPP__
#define MDFRWK_LIST_HPP__

#include <Synch/Spinlock.h>
#include <TYPE.h>

struct LinkedList;

///
/// Holds elements in a resizable array-buffer in the form of pointers. This
/// type of list is useful for non-intrusive usage and low-memory
/// requirements. This array-list also allows random access and allowing sorted
/// inserts & removes through the iterator.
///
/// @version 1.0
/// @since Silcos 3.02
/// @author Shukant Pal
///
class ArrayList final
{
public:
	ArrayList();
	ArrayList(unsigned long initialCapacity);
	ArrayList(LinkedList& elems);
	~ArrayList();
	unsigned long add(void *elem);
	unsigned long add(void *elem, unsigned long index);
	unsigned long addAll(LinkedList& elems);
	unsigned long count(){ return (size); }
	inline void ensureBuffer(unsigned long newCapacity);
	void *fastGet(unsigned long idx){ return (elemData[idx]); }
	void *&fastRef(unsigned long idx){ return (elemData[idx]); }
	void *get(unsigned long ix){ return (ix < size ? elemData[ix] : 0); }
	bool isEmpty(){ return (!size); }
	unsigned long firstIndexOf(void *);
	unsigned long lastIndexOf(void *);
	bool remove(unsigned long index);
	bool remove(void *elem);
	void set(void *elem, unsigned long idx);
	ArrayList *subList(unsigned long start, unsigned long end);
	void trimToSize();

	///
	/// Abstraction for iterating over an ArrayList. It will hold a current
	/// index by which the user can serially go back and forth. A new
	/// Iterator always starts from zero, unless specified. By iterating,
	/// the user can also replace objects "on the fly".
	///
	/// To optimize local usage, declare the Iterator on the stack -
	///
	/// 			Iterator listItr(mylist);
	///
	/// @version 1.0
	/// @since Silcos 3.02
	/// @author Shukant Pal
	///
	class Iterator final
	{
	public:
		Iterator(ArrayList& _list) : list(_list)
		{
			this->changeCount = list.changeCount;
			this->curIdx = 0;
			this->curPtr = *list.elemData;
			this->ref = list.elemData;
		}

		inline void *fastNext()
		{
			if(curIdx < list.size)
			{
				++(curIdx);
				++(ref);
				curPtr = *ref;
				return (curPtr);
			}
			else
				return (null);
		}

		inline void *fastPrev()
		{
			if(curIdx)
			{
				--(curIdx);
				--(ref);
				curPtr = *ref;
				return (curPtr);
			}
			else
				return (null);
		}

		inline void *get(){ return (curPtr); }
		inline unsigned long index(){ return (curIdx); }

		inline void *next()
		{
			if(this->changeCount != list.changeCount)
				return (null);

			if(curIdx == list.size - 1)
				return (null);

			++curIdx;
			++(ref);
			curPtr = *ref;
			return (curPtr);
		}

		inline void *prev()
		{
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

		inline void set(void *rep)
		{
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

	Spinlock modl;
private:
	unsigned long size;
	unsigned long capacity;
	unsigned long changeCount;
	void **elemData;
	bool isValidIndex(unsigned long idx){ return (idx < size); }
	void removeAt(unsigned long idx);
};

#endif/* Util/List.hpp */
