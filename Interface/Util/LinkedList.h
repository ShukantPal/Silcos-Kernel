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
} LINODE;

typedef LINODE LIST_ELEMENT;

/* Helper macros */
#define Head(L) (L -> Head)
#define Tail(L) (L -> Tail)
#define Count(L) (L -> Count)

typedef
struct LinkedList {
	SIZE_T Count;
	struct LinkedListNode *Head;
	struct LinkedListNode*Tail;
} LINKED_LIST;

VOID AddElement(
		struct LinkedListNode *newNode,
		struct LinkedList *List
);

VOID RemoveElement(
		struct LinkedListNode *newNode,
		struct LinkedList *list
);

/**
 * InsertElementAfter() - 
 *
 * Summary:
 * This function inserts the new element after the given old element, and
 * assumes the old element to be a non-null LINODE, which is participating
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
 * Returns: VOID
 *
 * @Version 1
 * @Since Circuit 2.03
 */
VOID InsertElementAfter(
	LINODE *oldElement,
	LINODE *newElement,
	LINKED_LIST *list
);

/**
 * InsertElementBefore() - 
 *
 * Summary:
 * This function inserts the new element before the given old element, and
 * assumes the old element to be a non-null LINODE, which is participating
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
 * Returns: VOID
 *
 * @Version 1
 * @Since Circuit 2.03
 */
VOID InsertElementBefore(
	LINODE *oldElement,
	LINODE *newElement,
	LINKED_LIST *list
);

VOID PushHead(
	LINODE *New,
	LINKED_LIST *List
);

LINODE *PullTail(
	LINKED_LIST *List
);

static inline
VOID ShiftElement(LINODE *Elem, LINKED_LIST *OldList, LINKED_LIST *NewList){
	RemoveElement(Elem, OldList);
	AddElement(Elem, NewList);
}

#endif/* Util/LinkedList.h */
