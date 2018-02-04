/* Copyright (C) 2017 - Shukant Pal */

#include <Debugging.h>
#include <Executable/Scheduler.h>
#include <Executable/ScheduleRoller.h>
#include <Memory/Pager.h>
#include <Util/AVLTree.hpp>

using namespace Executable;

Time XMilliTime; /* Maintain 64-bit kernel time. */

export_asm void Schedule(Processor *tproc)
{
	ScheduleInfo *tsched = &tproc->crolStatus;
	Executable::ScheduleRoller *lrol = tsched->presRoll;
	Executable::ScheduleRoller *nrol = tproc->lschedTable[0];

	tsched->presRoll = nrol;
	Executable::Task *ntask;

	if(nrol != lrol)
	{
		if(lrol != NULL)
		{
			lrol->free(XMilliTime, tproc);
		}

		ntask = nrol->allocate(XMilliTime, tproc);
	}
	else
	{
		ntask = nrol->update(XMilliTime, tproc);
	}

	tproc->ctask = ntask;
	if(ntask->mmu != NULL)
		SwitchContext(ntask->mmu);
}
