/**
 * Copyright (C) 2017 - Shukant Pal
 */

#ifndef UTIL_STACK_H
#define UTIL_STACK_H

#include "LinkedList.h"

typedef
struct _STACK_ELEMENT
{
	struct _STACK_ELEMENT *Next;
} STACK_ELEMENT;

typedef
struct
{
	STACK_ELEMENT *Head;
} STACK;

#define NEW_STACK { .Head = NULL }
#define PACK_STACK(Head_) { .Head = Head_ }

VOID PushElement(
	STACK_ELEMENT *New,
	STACK *Stack
);

STACK_ELEMENT *PopElement(
	STACK *Stack
);

#endif /* Util/Queue.h */
