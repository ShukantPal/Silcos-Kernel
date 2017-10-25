/**
 * Copyright (C) 2017 - Shukant Pal
 */

#include "BranchSymbols.h"
#include <HAL/Processor.h>

ULONG BranchTable[9] = {
	(ULONG) &SyTID,
	(ULONG) &SyCreateThread,
	(ULONG) &SyExitThread,
	(ULONG) &SyWaitForThread,
	(ULONG) &SySleepThread,
	(ULONG) &SyWaitForThreadTill,
	(ULONG) &SyDeleteThread,
	(ULONG) &SyStopThread,
	(ULONG) &SyWakeupThread
};

/* Sy Entrance Handler*/
extern VOID KernelIntrEnter();

/* HAL IDT manipulation link. */
extern VOID AddHandler(USHORT, ULONG);

/**
 * SySetup() -
 *
 * Setups the syscall feature of the kernel, for
 * execution of user-processes later on.
 *
 * It is a architectural function & does not have
 * any header for it. Use a link in C file only
 * to use it.
 *
 * @Version 1
 * @Since Circuit 2.01
 */
VOID SySetup() {
	PROCESSOR *pCPU = GetProcessorById(PROCESSOR_ID);
	PROCESSOR_INFO *pInfo = &(pCPU->Hardware);

	MapHandler(150, (ULONG) &KernelIntrEnter, &(pInfo->IDT[0]));
}
