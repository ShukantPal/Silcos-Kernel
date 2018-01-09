/**
 * File: CircularList.cpp
 *
 * Summary:
 * Implements the add/remove operations on the circular list.
 *
 * Functions:
 * AddCElement, RemoveCElement which operate on CircularList
 *
 *  -------------------------------------------------------------------
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

#include <Util/CircularList.h>
#include <TYPE.h>

/**
 * Function: AddCElement
 *
 * Summary:
 * Inserts the element into the circular list. There are two positions in which
 * the node could be inserted - after or before the clnMain node (CLAST or CFIRST)
 * and this function takes the pos argument from client.
 *
 * Args:
 * CircularListNode *clnNode - the node to add to the list
 * unsigned long pos - CFIRST or CLAST for adding the element before/after main
 * CircularList *clList - the list to add in
 *
 * Since: Circuit 2.03
 * Author: Shukant Pal
 */
extern "C" void AddCElement(CircularListNode *clnNode, unsigned long clnPosition,
						CircularList *clList)
{
	CircularListNode *clnMain = clList->lMain;
	if(clnMain != NULL)
	{
		if(clnPosition == CFIRST)
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

/**
 * Function: RemoveCElement
 *
 * Summary:
 * Remove the element from the circular list assuming it was added before.
 *
 * Args:
 * CircularListNode *clnNode - node to remove
 * CircularList *clList - list from which node is to be removed
 *
 * Since: Circuit 2.03
 * Author: Shukant Pal
 */
extern "C" void RemoveCElement(CircularListNode *clnNode, CircularList *clList)
{
	unsigned long clnCount = clList->count;
	
	if(clnCount > 0)
	{/* Eliminate blind-removals */
		if(clnCount > 1)
		{
			CircularListNode *clnNext = clnNode->next;
			CircularListNode *clnLast = clnNode->last;

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
