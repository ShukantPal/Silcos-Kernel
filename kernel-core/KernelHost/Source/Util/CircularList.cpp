/**
 * @file CircularList.cpp
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
#include <Utils/CircularList.h>
#include <TYPE.h>

/**
 * Adds the given node into the given circular list, at the
 * specified position.
 *
 * @param clnNode - the node to add to the list
 * @param pos - <tt>CFIRST</tt> to add the element at the beginning
 * 		or <tt>CLAST</tt> to add it at the very end
 * @param clList - the circular list to hold the node
 */
decl_c void AddCElement(CircularListNode *clnNode,
		unsigned long clnPosition, CircularList *clList)
{
	CircularListNode *clnMain = clList->lMain;

	if(clnMain != NULL) {
		if(clnPosition == CFIRST) {
			clnNode->next = clnMain->next;
			clnNode->next->last = clnNode;
			// Make sure back-linkage is correct!!! (ERR: Fixed)

			clnNode->last = clnMain;
			clnMain->next = clnNode;
		} else {
			clnNode->next = clnMain;
			clnMain->last = clnNode;
			
			clnNode->last = clnMain->last;
			clnNode->last->next = clnNode;
			// Make sure forward-linkage is correct!!! (ERR: Fixed)
		}
	} else {
		clnNode->next = clnNode;
		clnNode->last = clnNode;
		clList->lMain = clnNode;
	}

	++(clList->count);
}

/**
 * Removes the node given, from the circular-list (also given). If
 * the node wasn't added before, then the list will become corrupted.
 *
 * @param clnNode - the node to remove from the circular list
 * @param clList - the list holding the node
 */
decl_c void RemoveCElement(CircularListNode *clnNode, CircularList *clList)
{
	unsigned long clnCount = clList->count;
	
	if(clnCount > 0) {/* Eliminate blind-removals */
		if(clnCount > 1) {
			CircularListNode *clnNext = clnNode->next;
			CircularListNode *clnLast = clnNode->last;

			clnNext->last = clnLast;
			clnLast->next = clnNext;

			if(clnNode == clList->lMain) {
				clList->lMain = clnNext;
			}
		} else {
			clList->lMain = NULL;
		}

		clnNode->next = NULL;
		clnNode->last = NULL;
		--(clList->count);
	}
}
