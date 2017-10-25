/* Copyright (C) 2017 - Shukant Pal */

#include <Util/LinkedList.h>

/**
 * AddElement() -
 *
 * Summary:
 * This function adds the list node to the given list, assuming that it is
 * non-NULL and is isolated. If not, the owner list and this list, both may
 * get corruption.
 *
 * Args:
 * newNode - Node to be added
 * list - List on which the operation is being done
 *
 * Returns: VOID
 *
 * @Version 1
 * @Since Circuit 2.03
 */
VOID AddElement(LIST_ELEMENT *New, LINKED_LIST *List){
	LIST_ELEMENT *lHead = List->Head;

	if(lHead != NULL)
	{
		LIST_ELEMENT *lTail = List->Tail;

		if(lTail != NULL) {
			New->Previous = lTail;
			lTail->Next = New;
		} else {
			New->Previous = lHead;
			lHead->Next = New;
		}

		List->Tail = New;
	} else {
		New->Previous = NULL;
		List->Head = New;
	}

	New->Next = NULL;
	++(Count(List));
}

/**
 * RemoveElement() -
 *
 * Summary:
 * This function will remove oldElement for lList, assuming that it is already
 * in the list right now. NULL elements are not allowed.
 *
 * Although oldElement is removed from the chain of the list, it can still be
 * used for getting access in the list because its Next & Previous pointers
 * still have the same values.
 *
 * Args:
 * newNode - Node to be added
 * list - List is on which the operation is being done
 */
VOID RemoveElement(LIST_ELEMENT *oldElement, LINKED_LIST *lList){
	LIST_ELEMENT *oldNext = oldElement->Next;
	LIST_ELEMENT *oldPrevious = oldElement->Previous;

	if(oldPrevious != NULL)
		oldPrevious->Next = oldNext;
	else
		lList->Head = oldNext;

	if(oldNext != NULL)
		oldNext->Previous = oldPrevious;
	else
		lList->Tail = oldPrevious;

	--(lList->Count);

	if(lList->Count == 1) {
		lList->Head->Next = NULL;
		lList->Tail = NULL;
	} else if(lList->Count == 0) {
		lList->Head = NULL;
		lList->Tail = NULL;
	}
}

VOID InsertElementAfter(LIST_ELEMENT *Old, LIST_ELEMENT *New, LINKED_LIST *List){
	NextElement(New) = NextElement(Old);
	PreviousElement(New) = Old;

	NextElement(Old) = New;
	if(NextElement(New) != NULL)
		NextElement(New) -> Previous = New;
	else
		Tail(List) = New;

	++(Count(List));
}

VOID InsertElementBefore(LINODE *oldElement, LINODE *newElement, LINKED_LIST *list){
	LINODE *previousElement = oldElement->Previous;
	if(previousElement == NULL)
		list->Head = newElement;
	else
		previousElement->Next = newElement;
	newElement->Previous = previousElement;

	oldElement->Previous = newElement;
	newElement->Next = oldElement;

	++(list->Count);
}

VOID PushHead(LIST_ELEMENT *New, LINKED_LIST *List)
{
	LIST_ELEMENT *lHead = List->Head;

	if(lHead == NULL)
		New->Next = NULL;
	else {
		New->Next = lHead;
		lHead->Previous = New;

		LIST_ELEMENT *lTail = List->Tail;
		if(lTail == NULL) {
			List->Tail = New;
		}
	}

	List->Head = New;
	New->Previous = NULL;
	++(Count(List));
}

LIST_ELEMENT *PullTail(LINKED_LIST *List)
{
	LIST_ELEMENT *oldTail = List->Tail;
	LIST_ELEMENT *oldHead = List->Head;

	if(oldTail != NULL) {
		if(oldHead->Next == oldTail) {
			oldHead->Next = NULL;
			List->Tail = NULL;
		} else {
			List->Tail = oldTail->Previous;
			if(List->Tail)
				List->Tail->Next = NULL;
		}
	} else {
		oldTail = oldHead;
		List->Head = NULL;
	}

	--(List->Count);
	return (oldTail);
}
