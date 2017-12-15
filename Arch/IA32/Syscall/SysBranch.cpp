/**
 * Copyright (C) 2017 - Shukant Pal
 */

#include "BranchSymbols.h"
#include <HAL/Processor.h>

unsigned long BranchTable[9] = {
	(unsigned long) &SyTID,
	(unsigned long) &SyCreateThread,
	(unsigned long) &SyExitThread,
	(unsigned long) &SyWaitForThread,
	(unsigned long) &SySleepThread,
	(unsigned long) &SyWaitForThreadTill,
	(unsigned long) &SyDeleteThread,
	(unsigned long) &SyStopThread,
	(unsigned long) &SyWakeupThread
};

/* Sy Entrance Handler*/
extern void KernelIntrEnter();

/* HAL IDT manipulation link. */
extern void AddHandler(unsigned short, unsigned long);

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
void SySetup() {
	PROCESSOR *pCPU = GetProcessorById(PROCESSOR_ID);
	PROCESSOR_INFO *pInfo = &(pCPU->Hardware);

	MapHandler(150, (unsigned long) &KernelIntrEnter, &(pInfo->IDT[0]));
}
