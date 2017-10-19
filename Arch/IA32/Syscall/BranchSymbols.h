/**
 * Copyright (C) 2017 - Shukant Pal
 */

#ifndef X86_SYSCALL_BRANCH_SYMBOLS_H
#define X86_SYSCALL_BRANCH_SYMBOLS_H

#include <HAL/Processor.h>
#include <Types.h>

VOID SyTID(VOID);
VOID SyCreateThread(VOID);
VOID SyExitThread(VOID);
VOID SyWaitForThread(VOID);
VOID SySleepThread(VOID);
VOID SyWaitForThreadTill(VOID);
VOID SyDeleteThread(VOID);
VOID SyStopThread(VOID);
VOID SyWakeupThread(VOID);

#endif /* BranchSymbols.h */
