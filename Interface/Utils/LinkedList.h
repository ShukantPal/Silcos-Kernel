/**
 * File: LinkedList.h
 *
 * Summary:
 * Declares the LinkedListNode, LinkedList and the functions which operate
 * on them.
 *
 * Functions:
 * ShiftElement - moves a element from one list to the other
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
#ifndef UTIL_LINKED_LIST_H__
#define UTIL_LINKED_LIST_H__

struct LinkedListNode
{
	struct LinkedListNode *next;
	struct LinkedListNode *prev;
};

struct LinkedList
{
	unsigned long count;
	struct LinkedListNode *head;
	struct LinkedListNode *tail;
};

#ifndef _CBUILD
extern "C"
{
#endif

	void AddElement(struct LinkedListNode *newNode, struct LinkedList *List);
	void RemoveElement(struct LinkedListNode *newNode,
			struct LinkedList *list);
	void InsertElementAfter(struct LinkedListNode *oldElement,
			struct LinkedListNode *newElement, struct LinkedList *list);
	void InsertElementBefore(struct LinkedListNode *oldElement,
			struct LinkedListNode *newElement, struct LinkedList *list);
	void PushHead(struct LinkedListNode *newHead, struct LinkedList *list);
	struct LinkedListNode *PullTail(struct LinkedList *fromList);

#ifndef _CBUILD
}
#endif

static inline void ShiftElement(struct LinkedListNode *elem,
		struct LinkedList *old, struct LinkedList *newList)
{
	RemoveElement(elem, old);
	AddElement(elem, newList);
}

#endif/* Util/LinkedList.h */
