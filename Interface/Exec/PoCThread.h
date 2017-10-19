/**
 * Copyright (C) 2017 - Shukant Pal
 */

#ifndef EXEC_POC_THREAD_H
#define EXEC_POC_THREAD_H

#include <HAL/Processor.h>
#include <Exec/Thread.h>

static inline
UINT32_T GtPoC_LoadLowest()
{
	return (0);
}

static inline
UINT32_T AddThreadTo(struct PoC *P, struct Thread *T)
{
	if(ISRealtime(T)) {

	}
}

#endif /* Exec/PoCThread.h */
