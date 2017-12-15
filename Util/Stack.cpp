/* Copyright (C) 2017 - Shukant Pal */

#include <Util/Stack.h>

void PushElement(STACK_ELEMENT *Element, STACK *Stack)
{
	Element->Next = Stack->Head;
	Stack->Head = Element;
}

STACK_ELEMENT *PopElement(STACK *Stack)
{
	STACK_ELEMENT *Element = Stack->Head;
	if(Stack->Head == NULL)
		return (NULL);

	Stack->Head = Element->Next;
	Element->Next = NULL;
	return (Element);
}
