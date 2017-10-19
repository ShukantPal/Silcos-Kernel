/* Copyright (C) 2017 - Shukant Pal */

#include <HAL/Processor.h>
#include <Memory/CacheRegister.h>

LINODE *ChDataAllocate(CHSYS *chInfo, ULONG *statusFilter){
	ULONG curProcessor = (ULONG) GetProcessorById(PROCESSOR_ID);
	CHREG *chReg = (CHREG *) (curProcessor + chInfo->ChMemoryOffset);

	if(chReg->DCount != 0) {
		LINODE *dNode = chReg->DList.Head;
		RemoveElement(dNode, &chReg->DList);

		if(chReg->DCount == 0)
			*statusFilter = (ULONG) chReg | (1 << CH_POPULATE);
		else
			*statusFilter = 0;

		return (dNode);
	} else {
		*statusFilter = (ULONG) chReg | (1 << CH_POPULATE);
		return (NULL);
	}
}

LINODE *ChDataFree(LINODE *dNode, CHSYS *chInfo){
	ULONG curProcessor = (ULONG) GetProcessorById(PROCESSOR_ID);
	CHREG *chReg = (CHREG *) (curProcessor + chInfo->ChMemoryOffset);
	PushHead(dNode, &chReg->DList);

	if(chReg->DCount >= 64) 
		return (PullTail(&chReg->DList));
	else
		return (NULL);
}
