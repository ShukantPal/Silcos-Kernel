/* Copyright (C) 2017 - Shukant Pal */

#include <Util/CircularList.h>

extern "C" void ClnInsert(CircularListNode *clnNode, unsigned long clnPosition,
						CircularList *clList)
{
	CLNODE *clnMain = clList->lMain;
	if(clnMain != NULL)
	{
		if(clnPosition == CLN_FIRST)
		{
			clnNode->next = clnMain->next;
			clnNode->next->last = clnNode;// Make sure back-linkage is correct!!! (ERR: Fixed)

			clnNode->last = clnMain;
			clnMain->next = clnNode;
		}
		else
		{
			clnNode->next = clnMain;
			clnMain->last = clnNode;
			
			clnNode->last = clnMain->last;
			clnNode->last->next = clnNode;// Make sure forward-linkage is correct!!! (ERR: Fixed)
		}
	}
	else
	{
		clnNode->next = clnNode;
		clnNode->last = clnNode;
		clList->lMain = clnNode;
	}

	++(clList->count);
}

extern "C" void ClnRemove(CircularListNode *clnNode, CircularList *clList)
{
	unsigned long clnCount = clList->count;
	
	if(clnCount > 0)
	{/* Eliminate blind-removals */
		if(clnCount > 1)
		{
			CLNODE *clnNext = clnNode->next;
			CLNODE *clnLast = clnNode->last;

			clnNext->last = clnLast;
			clnLast->next = clnNext;

			if(clnNode == clList->lMain)
			{
				clList->lMain = clnNext;
			}
		}
		else
		{
			clList->lMain = NULL;
		}

		clnNode->next = NULL;
		clnNode->last = NULL;
		--(clList->count);
	}
}
