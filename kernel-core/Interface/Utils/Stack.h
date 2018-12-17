/**
 * Copyright (C) 2017 - Shukant Pal
 */

#ifndef UTIL_STACK_H
#define UTIL_STACK_H

#include "../Utils/LinkedList.h"

typedef
struct StackElement
{
	struct StackElement *Next;
} STACK_ELEMENT;

typedef
struct Stack
{
	STACK_ELEMENT *Head;
} STACK;

#define NEW_STACK { .Head = NULL }
#define PACK_STACK(Head_) { .Head = Head_ }

void PushElement(
	STACK_ELEMENT *New,
	STACK *Stack
);

STACK_ELEMENT *PopElement(
	STACK *Stack
);

#endif /* Util/Queue.h */
