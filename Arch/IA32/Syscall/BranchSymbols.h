/**
 * Copyright (C) 2017 - Shukant Pal
 */

#ifndef X86_SYSCALL_BRANCH_SYMBOLS_H
#define X86_SYSCALL_BRANCH_SYMBOLS_H

#include <HAL/Processor.h>
#include <Types.h>

void SyTID(void);
void SyCreateThread(void);
void SyExitThread(void);
void SyWaitForThread(void);
void SySleepThread(void);
void SyWaitForThreadTill(void);
void SyDeleteThread(void);
void SyStopThread(void);
void SyWakeupThread(void);

#endif /* BranchSymbols.h */
