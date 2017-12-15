/* Copyright (C) 2017 - Shukant Pal */

#include <HAL/Processor.h>
#include <Memory/CacheRegister.h>

LinkedListNode *ChDataAllocate(CHSYS *chInfo, unsigned long *statusFilter)
{
	unsigned long curProcessor = (unsigned long) GetProcessorById(PROCESSOR_ID);
	CHREG *chReg = (CHREG *) (curProcessor + chInfo->ChMemoryOffset);

	if(chReg->DCount != 0)
	{
		LinkedListNode *dNode = chReg->DList.Head;
		RemoveElement(dNode, &chReg->DList);

		if(chReg->DCount == 0)
			*statusFilter = (unsigned long) chReg | (1 << CH_POPULATE);
		else
			*statusFilter = 0;

		return (dNode);
	}
	else
	{
		*statusFilter = (unsigned long) chReg | (1 << CH_POPULATE);
		return (NULL);
	}
}

LinkedListNode *ChDataFree(LinkedListNode *dNode, CHSYS *chInfo)
{
	unsigned long curProcessor = (unsigned long) GetProcessorById(PROCESSOR_ID);
	CHREG *chReg = (CHREG *) (curProcessor + chInfo->ChMemoryOffset);
	PushHead(dNode, &chReg->DList);

	if(chReg->DCount >= 64) 
		return (PullTail(&chReg->DList));
	else
		return (NULL);
}
