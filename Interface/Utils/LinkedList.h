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

/**
 * Struct: LinkedListNode
 *
 * Summary:
 * This is the node used by the linked-list for connecting each element in the
 * list. For other data structures, this should be put at the very beginning
 * so that a mere (LinkedListNode*) cast can be passed to functions operating
 * on the data in lists.
 *
 * Author: Shukant Pal
 */
struct LinkedListNode
{
	LinkedListNode *next;
	LinkedListNode *prev;
};

/**
 * Struct: LinkedList
 *
 * Summary:
 * This is the descriptor for linked-lists which terminate at both ends with
 * null pointers.
 *
 * Author: Shukant Pal
 */
struct LinkedList
{
	unsigned long count;
	LinkedListNode *head;
	LinkedListNode *tail;
};

extern "C"
{
	void AddElement(LinkedListNode *newNode, LinkedList *List);
	void RemoveElement(LinkedListNode *newNode, LinkedList *list);
	void InsertElementAfter(LinkedListNode *oldElement, LinkedListNode *newElement,
					LinkedList *list);
	void InsertElementBefore(LinkedListNode *oldElement, LinkedListNode *newElement,
					LinkedList *list);
	void PushHead(LinkedListNode *newHead, LinkedList *list);
	LinkedListNode *PullTail(LinkedList *fromList);
}

static inline void ShiftElement(LinkedListNode *elem, LinkedList *old, LinkedList *newList)
{
	RemoveElement(elem, old);
	AddElement(elem, newList);
}

#endif/* Util/LinkedList.h */
