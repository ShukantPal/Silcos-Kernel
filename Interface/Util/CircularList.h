/**
 * Copyright (C) 2017 - Shukant Pal
 */
#ifndef UTIL_CIRCULAR_LIST_H
#define UTIL_CIRCULAR_LIST_H

#include <TYPE.h>

typedef
struct CircularListNode {
	struct CircularListNode *ClnNext;
	struct CircularListNode *ClnLast;
} CLNODE;

typedef
struct _CLIST {
	ULONG ClnCount;
	CLNODE *ClnMain;
} CLIST;

#define CLN_LAST 0
#define CLN_FIRST 1

VOID ClnInsert(
	CLNODE *ClnNode,
	ULONG ClnPosition,
	CLIST *clList
);

VOID ClnRemove(
	CLNODE *ClnNode,
	CLIST *clList
);

#endif/* Util/CircularList.h */
