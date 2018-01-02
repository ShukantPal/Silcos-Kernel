/**
 * File: CircularList.h
 *
 * Summary:
 * C-based circular-list is declared here.
 *
 * Functions:
 * ClnInsert - insert a circular-list node into a list (pre-allocated)
 * ClnRemove - removes a circular-list node from its owner-list
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
#ifndef UTIL_CIRCULAR_LIST_H__
#define UTIL_CIRCULAR_LIST_H__

#include <TYPE.h>

typedef
struct CircularListNode
{
	CircularListNode *next;
	CircularListNode *last;
} CLNODE;

typedef
struct CircularList {
	unsigned long count;
	CircularListNode *lMain;
} CLIST;

#define CLN_LAST 0
#define CLN_FIRST 1

extern "C" void ClnInsert(CircularListNode *ClnNode, unsigned long ClnPosition, CircularList *clList);
extern "C" void ClnRemove(CircularListNode *ClnNode, CircularList *clList);

#endif/* Util/CircularList.h */
