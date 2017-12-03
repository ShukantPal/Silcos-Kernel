/* Copyright (C) 2017 - Shukant Pal */

#include <Util/CircularList.h>

decl_c void ClnInsert(
		CircularListNode *clnNode,
		unsigned long clnPosition,
		CircularList *clList
){
	CLNODE *clnMain = clList->ClnMain;
	if(clnMain != NULL){
		if(clnPosition == CLN_FIRST){
			clnNode->ClnNext = clnMain->ClnNext;
			clnNode->ClnNext->ClnLast = clnNode;// Make sure back-linkage is correct!!! (ERR: Fixed)

			clnNode->ClnLast = clnMain;
			clnMain->ClnNext = clnNode;
		} else {
			clnNode->ClnNext = clnMain;
			clnMain->ClnLast = clnNode;
			
			clnNode->ClnLast = clnMain->ClnLast;
			clnNode->ClnLast->ClnNext = clnNode;// Make sure forward-linkage is correct!!! (ERR: Fixed)
		}
	} else {
		clnNode->ClnNext = clnNode;
		clnNode->ClnLast = clnNode;
		clList->ClnMain = clnNode;
	}

	++(clList->ClnCount);
}

decl_c void ClnRemove(
		CircularListNode *clnNode,
		CircularList *clList
){
	ULONG clnCount = clList->ClnCount;
	if(clnCount > 0){/* Eliminate blind-removals */
		if(clnCount > 1) {
			CLNODE *clnNext = clnNode->ClnNext;
			CLNODE *clnLast = clnNode->ClnLast;

			clnNext->ClnLast = clnLast;
			clnLast->ClnNext = clnNext;

			if(clnNode == clList->ClnMain)
			{ clList->ClnMain = clnNext; }
		} else {
			clList->ClnMain = NULL;
		}

		clnNode->ClnNext = NULL;
		clnNode->ClnLast = NULL;
		--(clList->ClnCount);
	}
}
