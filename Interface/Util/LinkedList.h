/**
 * Copyright (C) 2017 - Shukant Pal
 */

#ifndef __UTIL_LINKED_LIST_H__
#define __UTIL_LINKED_LIST_H__

#include <Types.h>

/* Helper macros */
#define NextElement(E) (E -> Next)
#define PreviousElement(E) (E -> Previous)

typedef
struct LinkedListNode {
	struct LinkedListNode *Next;
	struct LinkedListNode *Previous;
} LinkedListNode;

/**
 * Struct: GenericLinkedListNode
 *
 * Summary:
 * This generic linked-list node is used for lists in which the elements point
 * to certain object by a (void*) pointer. This means they aren't contained in
 * the object listed.
 *
 * NOTE:
 * This is of same size as the 'linked-list' thus you should allocated it from
 * the tLinkedList object-allocator.
 *
 * Author: Shukant Pal
 */
struct GenericLinkedListNode
{
	union
	{
		GenericLinkedListNode *nextGenericNode;/* for client */
		LinkedListNode *nextNode;/* for compat with linode */
	};
	union
	{
		GenericLinkedListNode *previousGenericNode;/* for client */
		LinkedListNode *previousNode;/* for compat with linode */
	};
	Void *objectListed;
};

typedef LinkedListNode LIST_ELEMENT;

/* Helper macros */
#define Head(L) (L -> Head)
#define Tail(L) (L -> Tail)
#define Count(L) (L -> Count)

typedef
struct LinkedList {
	SIZE Count;
	struct LinkedListNode *Head;
	struct LinkedListNode*Tail;
} LinkedList;

void AddElement(LinkedListNode *newNode, struct LinkedList *List);

void RemoveElement(LinkedListNode *newNode, LinkedList *list);

/**
 * InsertElementAfter() - 
 *
 * Summary:
 * This function inserts the new element after the given old element, and
 * assumes the old element to be a non-null LinkedListNode, which is participating
 * in the same list. If the given element is null or belongs to a different list
 * then the list will become corrupted.
 *
 * The new element should also be isolated and not belong to another list,
 * otherwise that list will become corrupted.
 *
 * Args:
 * oldElement - Existing element, of the same list, after which a element is to inserted
 * newElement - A isolated element, to be added to the list
 * list - List on which operation is being done
 *
 * Returns: void
 *
 * @Version 1
 * @Since Circuit 2.03
 */
void InsertElementAfter(
	LinkedListNode *oldElement,
	LinkedListNode *newElement,
	LinkedList *list
);

/**
 * InsertElementBefore() - 
 *
 * Summary:
 * This function inserts the new element before the given old element, and
 * assumes the old element to be a non-null LinkedListNode, which is participating
 * in the same list. If the given element is null or belongs to a different list
 * then the list will become corrupted.
 *
 * The new element should also be isolated and not belong to another list,
 * otherwise that list will become corrupted.
 *
 * Args:
 * oldElement - Existing element, of the same list, before which a element is to inserted
 * newElement - A isolated element, to add to the list
 * list - List on which operation is being done
 *
 * Returns: void
 *
 * @Version 1
 * @Since Circuit 2.03
 */
void InsertElementBefore(
	LinkedListNode *oldElement,
	LinkedListNode *newElement,
	LinkedList *list
);

void PushHead(
	LinkedListNode *New,
	LinkedList *List
);

LinkedListNode *PullTail(
	LinkedList *List
);

static inline
void ShiftElement(LinkedListNode *Elem, LinkedList *OldList, LinkedList *NewList){
	RemoveElement(Elem, OldList);
	AddElement(Elem, NewList);
}

#endif/* Util/LinkedList.h */
